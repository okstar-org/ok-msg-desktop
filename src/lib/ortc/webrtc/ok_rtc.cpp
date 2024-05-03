/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */
#include "ok_rtc.h"

#include <memory>
#include <string>
#include <utility>

#include "api/peer_connection_interface.h"

#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/create_peerconnection_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <pc/video_track_source.h>


#include <rtc_base/thread.h>
#include <rtc_base/logging.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/string_encode.h>

#include "ok_conductor.h"
#include "vcm_capturer.h"

namespace lib {
namespace ortc {

class ORTC;

class CapturerTrackSource : public webrtc::VideoTrackSource {
public:
  explicit CapturerTrackSource(std::unique_ptr<VcmCapturer> capturer)
      : VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

private:
  std::unique_ptr<VcmCapturer> capturer_;

  rtc::VideoSourceInterface<webrtc::VideoFrame> *source() override {
    return capturer_.get();
  }
};

static std::unique_ptr<webrtc::SessionDescriptionInterface>
CreateSessionDescription(webrtc::SdpType sdpType,
                         const JingleContext &context) {

  auto sessionDescription = std::make_unique<::cricket::SessionDescription>();
  auto &contents = context.getContents();

  cricket::ContentGroup group(cricket::GROUP_TYPE_BUNDLE);
  for (const auto &content : contents) {

    auto name = content.name;
    group.AddContentName(name);

    auto description = content.rtp;
    auto iceUdp = content.iceUdp;

    // iceUdp
    ::cricket::TransportInfo ti;
    ti.content_name = name;
    ti.description.ice_ufrag = iceUdp.ufrag;
    ti.description.ice_pwd = iceUdp.pwd;

    auto dtls = iceUdp.dtls;
    auto fp = ::cricket::TransportDescription::CopyFingerprint( //
        rtc::SSLFingerprint::CreateFromRfc4572                  //
        (dtls.hash, dtls.fingerprint));                         //
    ti.description.identity_fingerprint.reset(fp);

    if (dtls.setup == "actpass") {
      ti.description.connection_role = ::cricket::CONNECTIONROLE_ACTPASS;
    } else if (dtls.setup == "active") {
      ti.description.connection_role = ::cricket::CONNECTIONROLE_ACTIVE;
    } else if (dtls.setup == "passive") {
      ti.description.connection_role = ::cricket::CONNECTIONROLE_PASSIVE;
    } else if (dtls.setup == "holdconn") {
      ti.description.connection_role = ::cricket::CONNECTIONROLE_HOLDCONN;
    } else {
      ti.description.connection_role = ::cricket::CONNECTIONROLE_NONE;
    }

    sessionDescription->AddTransportInfo(ti);

    switch (description.media) {
    case gloox::Jingle::audio: {
      auto acd = std::make_unique<::cricket::AudioContentDescription>();

      for (auto &pt : description.payloadTypes) {
//        auto ac = ::cricket::CreateAudioCodec(
//                                 pt.id, pt.name, pt.clockrate,
//                                 pt.channels);
::cricket::AudioCodec ac(pt.id, pt.name, pt.clockrate, pt.bitrate,pt.channels);
        for (auto &e : pt.parameters) {
          ac.SetParam(e.name, e.value);
        }
        for (auto &e : pt.feedbacks) {
          ::cricket::FeedbackParam fb(e.type, e.subtype);
          ac.AddFeedbackParam(fb);
        }
        acd->AddCodec(ac);
      }

      for (auto &hdrext : description.hdrExts) {
        webrtc::RtpExtension ext(hdrext.uri, hdrext.id);
        acd->AddRtpHeaderExtension(ext);
      }

      ::cricket::StreamParams streamParams;
      for (auto &src : description.sources) {
        streamParams.ssrcs.push_back(std::stoul(src.ssrc));
        for (auto &p : src.parameters) {
          if (p.name == "cname") {
            streamParams.cname = p.value;
          } else if (p.name == "label") {
            streamParams.id = p.value;
          } else if (p.name == "mslabel") {
            streamParams.set_stream_ids({p.value});
          }
        };
      }

      auto g = description.ssrcGroup;
      std::vector<uint32_t> ssrcs;
      std::transform(g.ssrcs.begin(), g.ssrcs.end(), std::back_inserter(ssrcs),
                     [](auto s) -> uint32_t { return std::stoul(s); });
      ::cricket::SsrcGroup ssrcGroup(g.semantics, ssrcs);

      // ssrc-groups
      streamParams.ssrc_groups.emplace_back(ssrcGroup);

      // ssrc
      acd->AddStream(streamParams);

      // rtcp-mux
      acd->set_rtcp_mux(description.rtcpMux);

      sessionDescription->AddContent(name, ::cricket::MediaProtocolType::kRtp,
                                     std::move(acd));
      break;
    }
    case gloox::Jingle::video: {
      auto vcd = std::make_unique<::cricket::VideoContentDescription>();
      for (auto &pt : description.payloadTypes) {
        ::cricket::VideoCodec vc(pt.id, pt.name);
//        auto vc = ::cricket::CreateVideoCodec(pt.id, pt.name);
        for (auto &e : pt.parameters) {
          vc.SetParam(e.name, e.value);
        }
        for (auto &e : pt.feedbacks) {
          ::cricket::FeedbackParam fb(e.type, e.subtype);
          vc.AddFeedbackParam(fb);
        }
        vcd->AddCodec(vc);
      }
      for (auto &hdrExt : description.hdrExts) {
        webrtc::RtpExtension ext(hdrExt.uri, hdrExt.id);
        vcd->AddRtpHeaderExtension(ext);
      }
      vcd->set_rtcp_mux(description.rtcpMux);

      ::cricket::StreamParams streamParams;
      for (auto &src : description.sources) {
        streamParams.ssrcs.push_back(std::stoul(src.ssrc));
        for (auto &p : src.parameters) {
          if (p.name == "cname") {
            streamParams.cname = p.value;
          } else if (p.name == "label") {
            streamParams.id = p.value;
          } else if (p.name == "mslabel") {
            streamParams.set_stream_ids({p.value});
          }
        };
      }

      // ssrc-group
      auto g = description.ssrcGroup;
      std::vector<uint32_t> ssrcs;
      std::transform(g.ssrcs.begin(), g.ssrcs.end(), std::back_inserter(ssrcs),
                     [](auto s) -> uint32_t { return std::stoul(s); });
      ::cricket::SsrcGroup ssrcGroup(g.semantics, ssrcs);
      streamParams.ssrc_groups.emplace_back(ssrcGroup);
      vcd->AddStream(streamParams);

      sessionDescription->AddContent(name, ::cricket::MediaProtocolType::kRtp,
                                     std::move(vcd));
      break;
    }
    default:
      break;
    }
  }
  sessionDescription->AddGroup(group);

  auto sessionDescriptionInterface = webrtc::CreateSessionDescription(
      sdpType, context.sessionId, context.sessionVersion,
      std::move(sessionDescription));

  return sessionDescriptionInterface;
}

ORTC::ORTC(std::list<IceServer> iceOptions, OkRTCHandler *handler,
           OkRTCRenderer *renderer)
    : _iceOptions((iceOptions)) {

  _rtcHandler = handler;
  _rtcRenderer = renderer;
  start();

}

ORTC::~ORTC() {
  RTC_LOG(LS_INFO) << "WebRTC will be destroy...";
  shutdown();
  RTC_LOG(LS_INFO) << "WebRTC has be destroyed.";
}

void ORTC::start() {

  assert(!_started);

  // lock
  _start_shutdown_mtx.lock();

  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::InitializeSSL();
  RTC_LOG(LS_INFO) << "WebRTC is starting...";

  network_thread = rtc::Thread::CreateWithSocketServer();
  network_thread->SetName("network_thread", nullptr);
  bool result = network_thread->Start();
  RTC_LOG(LS_INFO) << "Start network thread=>" << result;

  worker_thread = rtc::Thread::Create();
  worker_thread->SetName("worker_thread", nullptr);
  result = worker_thread->Start();
  RTC_LOG(LS_INFO) << "Start worker thread=>" << result;

  signaling_thread = rtc::Thread::Create();
  signaling_thread->SetName("signaling_thread", nullptr);
  result = signaling_thread->Start();
  RTC_LOG(LS_INFO) << "Start signaling thread=>" << result;

  peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
      network_thread.get(),   /* network_thread */
      worker_thread.get(),    /* worker_thread */
      signaling_thread.get(), /* signaling_thread */
      nullptr,                /* default_adm */
      webrtc::CreateBuiltinAudioEncoderFactory(), //
      webrtc::CreateBuiltinAudioDecoderFactory(), //
      webrtc::CreateBuiltinVideoEncoderFactory(), //
      webrtc::CreateBuiltinVideoDecoderFactory(), //
      nullptr /* audio_mixer */,                  //
      nullptr /* audio_processing */);

  webrtc::PeerConnectionFactoryInterface::Options options;
  options.disable_encryption = false;
  peer_connection_factory_->SetOptions(options);

  RTC_LOG(LS_INFO) << "peer_connection_factory_:"
                     << peer_connection_factory_.get();

  // qDebug(("Create audio source..."));
  _audioSource =
      peer_connection_factory_->CreateAudioSource(::cricket::AudioOptions());

  // qDebug(("Create video device..."));
  _videoDeviceInfo = std::shared_ptr<webrtc::VideoCaptureModule::DeviceInfo>(
      webrtc::VideoCaptureFactory::CreateDeviceInfo());

  _started = true;
  _shutdown = false;

  _start_shutdown_mtx.unlock();

  RTC_LOG(LS_INFO) << "WebRTC has be started.";
}

void ORTC::shutdown() {
  RTC_LOG(LS_INFO) << "WebRTC is shutdown...";
  assert(!_shutdown);

  // lock
  _start_shutdown_mtx.lock();

  peer_connection_factory_ = nullptr;
  rtc::CleanupSSL();

  _shutdown = true;
  // unlock
  _start_shutdown_mtx.unlock();
}

bool ORTC::call(const std::string &peerId, const std::string &sId,
                lib::ortc::JingleCallType callType) {

  createConductor(peerId, sId, callType);

  return true;
}

bool ORTC::join(const std::string &peerId, const std::string &sId,
                const JingleContext &context) {
  assert(!peerId.empty());

  createConductor(peerId, sId, context.callType());

  RTC_LOG(LS_INFO) << "end";
  return true;
}

bool ORTC::quit(const std::string &peerId) {
  //  qDebug(("Quit for WebRTC peerId:%1").arg(qstring(peerId)));
  auto it = _pcMap.find(peerId);
  if (it == _pcMap.end()) {
    return false;
  }

  delete it->second;
  it->second = nullptr;
  _pcMap.erase(it);
  return true;
}

Conductor *ORTC::getConductor(const std::string &peerId) {

  RTC_LOG(LS_INFO) << "Get conductor for peerId:" << peerId;

  auto it = _pcMap.find(peerId);
  if (it == _pcMap.end()) {
    return nullptr;
  }
  return it->second;
}

Conductor *ORTC::createConductor(const std::string &peerId,
                                 const std::string &sId,
                                 JingleCallType callType) {

  RTC_LOG(LS_INFO) << "createConductor:" << peerId //
                     << " sid:" << sId                //
                     << " callType:" << callType;     //

  webrtc::PeerConnectionInterface::RTCConfiguration _rtcConfig;
  _rtcConfig.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
  _rtcConfig.enable_implicit_rollback = false;
  _rtcConfig.enable_ice_renomination = true;

  // Add the ice server.
  webrtc::PeerConnectionInterface::IceServer stunServer;
  for (const auto &ice : iceOptions()) {
    stunServer.urls.push_back(ice.uri);
    stunServer.tls_cert_policy =
        webrtc::PeerConnectionInterface::kTlsCertPolicyInsecureNoCheck;
    if (!ice.username.empty()) {
      stunServer.username = ice.username;
      stunServer.password = ice.password;
    }
  }
  _rtcConfig.servers.push_back(stunServer);

  auto conductor = //
      new Conductor(_rtcConfig, peer_connection_factory_, peerId, sId,
                    _rtcHandler, _rtcRenderer);

  if (callType == JingleCallType::audio) {
    // qDebug(("AddTrack audio..."));
    conductor->AddTrack(_audioSource.get());
  }

  if (callType == JingleCallType::video) {
    // qDebug(("AddTrack audio..."));
    conductor->AddTrack(_audioSource.get());

    int num_devices = _videoDeviceInfo->NumberOfDevices();
    // qDebug(("Get number of video devices:%1").arg(num_devices));
    /**
     * TODO 多个视频设备需要提醒
     */
    const size_t kWidth = 840;
    const size_t kHeight = 480;
    const size_t kFps = 30;
    std::string label = "-";

    for (int i = 0; i < num_devices; ++i) {
      auto capturer = std::make_unique<VcmCapturer>(_videoDeviceInfo.get());
      if (!capturer) {
        RTC_LOG(LS_INFO) << "Can not create camera capture for device" << i;
        continue;
      }
      bool created = capturer->Create(kWidth, kHeight, kFps, i);
      if (!created) {
        RTC_LOG(LS_INFO) << "Can not start camera capture for device" << i;
        continue;
      }

      _videoTrackSource =
          rtc::make_ref_counted<CapturerTrackSource>(std::move(capturer));
      conductor->AddTrack(_videoTrackSource.get());

      // 本地视频显示
      //_video_track = peer_connection_factory_->CreateVideoTrack(label,
      //_videoTrackSource.get());
      //_video_track->AddOrUpdateSink(new OVideoSink(_rtcRenderer),
      // rtc::VideoSinkWants()); qDebug(("Added video track, The device num
      // is:%1").arg(i));

      break;
    }
  }

  _pcMap.emplace(peerId, conductor);
  return conductor;
}

void ORTC::SetRemoteDescription(const std::string &peerId,
                                const lib::ortc::JingleContext &jingleContext) {

  auto conductor = getConductor(peerId);
  auto sdi = CreateSessionDescription(
      jingleContext.sdpType == JingleSdpType::Answer ? webrtc::SdpType::kAnswer
                                                     : webrtc::SdpType::kOffer,
      jingleContext);
  conductor->SetRemoteDescription(std::move(sdi));
}

bool ORTC::SetTransportInfo(const std::string &peerId,   //
                            const ortc::OIceUdp &iceUdp) //
{
  Conductor *conductor = getConductor(peerId);
  if (!conductor)
    return false;
  setTransportInfo(conductor, iceUdp);
  return true;
}

void ORTC::ContentAdd(std::map<std::string, gloox::Jingle::Session> sdMap,
                      OkRTCHandler *handler) {
  _c->OnContentAdd(sdMap, handler);
}

void ORTC::ContentRemove(std::map<std::string, gloox::Jingle::Session> sdMap,
                         OkRTCHandler *handler) {

  _c->OnContentRemove(sdMap, handler);
}

void ORTC::setMute(bool mute) {
  for (auto it : _pcMap) {
    it.second->setMute(mute);
  }
}

void ORTC::setRemoteMute(bool mute) {
  for (auto it : _pcMap) {
    it.second->setRemoteMute(mute);
  }
}
void ORTC::createPeerConnection() {}

size_t ORTC::getVideoSize() { return _c->getVideoCaptureSize(); }

void ORTC::CreateOffer(const std::string &peerId) {
  Conductor *conductor = getConductor(peerId);
  conductor->CreateOffer();
}

void ORTC::SessionTerminate(const std::string &peerId) { quit(peerId); }

void ORTC::CreateAnswer(const std::string &peerId, //
                        const lib::ortc::JingleContext &pContent) {
  Conductor *conductor = getConductor(peerId);
  if (!conductor) {
    return;
  }
  auto sdi = CreateSessionDescription(webrtc::SdpType::kOffer, pContent);
  conductor->CreateAnswer(std::move(sdi));
}

void ORTC::setTransportInfo(Conductor *conductor, const ortc::OIceUdp &iceUdp) {

  //  //qDebug(("mid:%1
  //  mline:%2").arg(qstring(iceUdp.mid)).arg(iceUdp.mline))
  int i = 0;
  for (auto &_candidate : iceUdp.candidates) {
    ::cricket::Candidate candidate;
    if (!_candidate.id.empty()) {
      candidate.set_id(_candidate.id);
    } else {
      candidate.set_id(std::to_string(i));
    }
    if (_candidate.component.empty()) {
      return;
    }
    if (_candidate.generation.empty()) {
      return;
    }

    candidate.set_foundation(_candidate.foundation);
    candidate.set_tcptype(_candidate.tcptype);   // passive
    candidate.set_protocol(_candidate.protocol); // udp,ssltcp
    candidate.set_priority(_candidate.priority);
    candidate.set_component(std::stoi(_candidate.component));

    if (!_candidate.network.empty()) {
      candidate.set_network_id(std::stoi(_candidate.network));
    } else {
      candidate.set_network_id(i);
    }

    candidate.set_generation(std::stoi(_candidate.generation));
    candidate.set_address(::rtc::SocketAddress(_candidate.ip, _candidate.port));

    /**
     *  const auto& host = LOCAL_PORT_TYPE;
        const auto& srflx = STUN_PORT_TYPE;
        const auto& relay = RELAY_PORT_TYPE;
        const auto& prflx = PRFLX_PORT_TYPE;
     */

    switch (_candidate.type) {
    case gloox::Jingle::ICEUDP::Type::Host:
      candidate.set_type(::cricket::LOCAL_PORT_TYPE);
      break;
    case gloox::Jingle::ICEUDP::Type::PeerReflexive:
      candidate.set_type(::cricket::PRFLX_PORT_TYPE);
      break;
    case gloox::Jingle::ICEUDP::Type::Relayed:
      candidate.set_type(::cricket::RELAY_PORT_TYPE);
      break;
    case gloox::Jingle::ICEUDP::Type::ServerReflexive:
      candidate.set_type(::cricket::STUN_PORT_TYPE);
      break;
    }
    if (!_candidate.rel_addr.empty()) {
      rtc::SocketAddress raddr;
      raddr.SetIP(_candidate.rel_addr);
      raddr.SetPort(_candidate.rel_port);
      candidate.set_related_address(raddr);
    }

    auto jsep_candidate =
        webrtc::CreateIceCandidate(iceUdp.mid, iceUdp.mline, candidate);
    conductor->setTransportInfo(std::move(jsep_candidate));
    i++;
  }
}

} // namespace ortc
} // namespace lib
