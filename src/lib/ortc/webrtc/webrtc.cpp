/*
 * Copyright (c) 2022 ???? chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */
#include "webrtc.h"
#include "StaticThreads.h"
#include "ok_conductor.h"

#include <memory>
#include <string>
#include <utility>

#include <rtc_base/thread.h>
#include <rtc_base/logging.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/string_encode.h>
#include <api/peer_connection_interface.h>
#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/create_peerconnection_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <pc/video_track_source.h>
#include <modules/video_capture/video_capture_factory.h>


namespace lib {
namespace ortc {

static std::unique_ptr<webrtc::SessionDescriptionInterface>
CreateSessionDescription(webrtc::SdpType sdpType, const OJingleContentAv &context) {

  auto sessionDescription = std::make_unique<::cricket::SessionDescription>();
  auto &contents = context.contents;

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

WebRTC::WebRTC()
    : peer_connection_factory{nullptr}, _rtcHandler{nullptr}
{
    _rtcConfig.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    _rtcConfig.enable_implicit_rollback = false;
    _rtcConfig.enable_ice_renomination = true;
}

WebRTC::~WebRTC() {
    if(isStarted()){
        stop();
    }
}

bool WebRTC::start()
{
    RTC_DLOG_F(LS_INFO) << "Starting the WebRTC...";
    // lock
    start_mtx.lock();

    //  _logSink(std::make_unique<LogSinkImpl>())
    //    rtc::LogMessage::AddLogToStream(_logSink.get(), rtc::LS_INFO);
     rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
    //    rtc::LogMessage::SetLogToStderr(false);


    RTC_DLOG_F(LS_INFO) << "InitializeSSL=>"<<rtc::InitializeSSL();;


    RTC_DLOG_F(LS_INFO) << "Creating network thread";
    network_thread = rtc::Thread::CreateWithSocketServer();
    RTC_DLOG_F(LS_INFO) << "Network thread=>" << network_thread;
    network_thread->SetName("network_thread", this);
    RTC_DLOG_F(LS_INFO) << "Network thread is started=>" << network_thread->Start();

    RTC_DLOG_F(LS_INFO) << "Creating worker thread";
    worker_thread = rtc::Thread::Create();
    RTC_DLOG_F(LS_INFO) << "Worker thread=>" << worker_thread;
    worker_thread->SetName("worker_thread", this);
    RTC_DLOG_F(LS_INFO) << "Worker thread is started=>" << worker_thread->Start();

    RTC_DLOG_F(LS_INFO) << "Creating signaling thread";
    signaling_thread = rtc::Thread::Create();
    RTC_DLOG_F(LS_INFO) << "Signaling thread=>" << signaling_thread;;
    signaling_thread->SetName("signaling_thread", this);
    RTC_DLOG_F(LS_INFO) << "Signaling thread is started=>" << signaling_thread->Start();


    peer_connection_factory = webrtc::CreatePeerConnectionFactory(
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

     RTC_DLOG_F(LS_INFO) << "peer_connection_factory:" << peer_connection_factory;

      webrtc::PeerConnectionFactoryInterface::Options options;
      options.disable_encryption = false;
      peer_connection_factory->SetOptions(options);


    RTC_DLOG_F(LS_INFO) << "Create audio source...";
    audioSource = peer_connection_factory->CreateAudioSource(::cricket::AudioOptions());
    RTC_DLOG_F(LS_INFO) << "Audio source is:"<< audioSource;


    RTC_DLOG_F(LS_INFO) << "Create video device...";
    auto vdi = webrtc::VideoCaptureFactory::CreateDeviceInfo();
    RTC_DLOG_F(LS_INFO) << "Video capture numbers:"<< vdi->NumberOfDevices();



    start_mtx.unlock();

     RTC_DLOG_F(LS_INFO) << "WebRTC has be started.";
     return true;
}

bool WebRTC::stop()
{
    RTC_DLOG_F(LS_INFO) << "WebRTC will be destroy...";
    std::lock_guard<std::recursive_mutex> lock(start_mtx);

    //销毁connection
    for(auto pc : _pcMap){
        delete pc.second;
    }

    //销毁factory
    peer_connection_factory = nullptr;

    //清除ssl
    rtc::CleanupSSL();
    RTC_DLOG_F(LS_INFO) << "WebRTC has be destroyed.";
    return true;
}

bool WebRTC::isStarted()
{
    return peer_connection_factory;
}

bool WebRTC::ensureStart()
{
   std::lock_guard<std::recursive_mutex> lock(start_mtx);
   return isStarted()? true: start();
}

void WebRTC::addIceServer(const IceServer &ice)
{
    // Add the ice server.
    webrtc::PeerConnectionInterface::IceServer stunServer;

      stunServer.urls.push_back(ice.uri);
      stunServer.tls_cert_policy = webrtc::PeerConnectionInterface::kTlsCertPolicyInsecureNoCheck;
      if (!ice.username.empty()) {
        stunServer.username = ice.username;
        stunServer.password = ice.password;
      }

    _rtcConfig.servers.push_back(stunServer);

}

void WebRTC::addRTCHandler(  OkRTCHandler *hand)
{
    _rtcHandler = hand;
}

bool WebRTC::call(const std::string &peerId, const std::string &sId,
                    bool video) {
  RTC_DLOG_F(LS_INFO) <<"peerId:" << peerId;
  return createConductor(peerId, sId, video);
}

bool WebRTC::join(const std::string &peerId,
                  const std::string &sId,
                  const OJingleContentAv &context) {
  assert(!peerId.empty());

//  createConductor(peerId, sId, context.callType());
  // RTC_DLOG_F(LS_INFO) << "end";

  return true;
}

bool WebRTC::quit(const std::string &peerId) {
  //  qDebug(("Quit for WebRTC peerId:%1").arg(qstring(peerId)));
//  auto it = _pcMap.find(peerId);
//  if (it == _pcMap.end()) {
//    return false;
//  }

//  delete it->second;
//  it->second = nullptr;
//  _pcMap.erase(it);
  return true;
}

Conductor *WebRTC::getConductor(const std::string &peerId) {
    return _pcMap[peerId];
}

Conductor *WebRTC::createConductor(const std::string &peerId,
                                 const std::string &sId,
                                 bool video) {

   RTC_LOG_F(LS_INFO) << "peer:" << peerId //
                     << " sid:" << sId                //
                     << " video:" << video;     //
  auto conductor = _pcMap[peerId];
  if(conductor){
      return conductor;
  }

  conductor = new Conductor(this, peerId, sId);

  if (!video) {
    RTC_DLOG_V(rtc::LS_INFO) << "AddTrack audio..." << audioSource;
    conductor->AddTrack(audioSource);
  } else {
    RTC_DLOG_V(rtc::LS_INFO) << "AddTrack audio..." << audioSource;
    conductor->AddTrack(audioSource);

    auto vdi = webrtc::VideoCaptureFactory::CreateDeviceInfo();
    int num_devices = vdi->NumberOfDevices();
    RTC_DLOG_F(LS_INFO) << "Get number of video devices:" << num_devices;

    const size_t kWidth = 840;
    const size_t kHeight = 480;
    const size_t kFps = 30;
    std::string label = "-";

    int videoIdx = -1;
    for (int i = 0; i < vdi->NumberOfDevices(); ++i) {


//      auto capturer = new VcmCapturer(vdi);
//      if (!capturer) {
//         RTC_LOG(LS_WARNING) << "Can not create camera capture for device" << i;
//        continue;
//      }
//
//      bool created = capturer->Create(kWidth, kHeight, kFps, i);
//      if (!created) {
//        RTC_DLOG_F(LS_INFO) << "Can not start camera capture for device" << i;
//        continue;
//      }
//
      char name[50] = {0};
      char uid[50] = {0};
      char puid[50] = {0};
      vdi->GetDeviceName(i, name, 50, uid, 50, puid, 50);
     RTC_DLOG_F(LS_INFO) << "Video device info:" << name << uid << puid;


//    auto  _videoTrackSource =
//          rtc::make_ref_counted<webrtc::VideoTrackSource>(std::unique_ptr<VcmCapturer>(capturer));
//    new webrtc::VideoTrackSource(false);
//    auto x=  peer_connection_factory->CreateVideoTrack(name, capturer);

      auto vc= getVideoCapture(uid);
      conductor->AddTrack(vc->source());


      // ??????
      //_video_track = peer_connection_factory_->CreateVideoTrack(label,
      //_videoTrackSource.get());
      //_video_track->AddOrUpdateSink(new OVideoSink(_rtcRenderer),
      // rtc::VideoSinkWants()); qDebug(("Added video track, The device num
       // is:%1").arg(i));

      break;
    }
  }
  _pcMap[peerId]= conductor;
  return conductor;
}

void WebRTC::SetRemoteDescription(const std::string &peerId,
                                    const OJingleContentAv &jingleContext) {

  auto conductor = getConductor(peerId);
  auto sdi = CreateSessionDescription(
      jingleContext.sdpType == JingleSdpType::Answer ? webrtc::SdpType::kAnswer
                                                     : webrtc::SdpType::kOffer,
      jingleContext);
  conductor->SetRemoteDescription(std::move(sdi));
}

void WebRTC::setTransportInfo(const std::string &peerId,
                              const std::string &sId,
                              const ortc::OIceUdp &iceUdp) {

    RTC_LOG_F(LS_INFO) << "peerId:"<< peerId;

    Conductor *conductor = createConductor(peerId, sId, false);
    if (!conductor) {
      RTC_LOG_F(LS_WARNING) << "conductor is null!";
      return;
    }

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
    candidate.set_component(std::stoi(_candidate.component));
    candidate.set_tcptype(_candidate.tcptype);   // passive
    candidate.set_protocol(_candidate.protocol); // udp,ssltcp
    candidate.set_priority(_candidate.priority);
    candidate.set_generation(std::stoi(_candidate.generation));

    if (!_candidate.network.empty()) {
      candidate.set_network_id(std::stoi(_candidate.network));
    } else {
      candidate.set_network_id(i);
    }

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

    auto jsep_candidate = webrtc::CreateIceCandidate(iceUdp.mid, iceUdp.mline, candidate);
    conductor->setTransportInfo(std::move(jsep_candidate));
    i++;
  }
}

void WebRTC::ContentAdd(std::map<std::string, gloox::Jingle::Session> sdMap,
                      OkRTCHandler *handler) {
//  _c->OnContentAdd(sdMap, handler);
}

void WebRTC::ContentRemove(std::map<std::string, gloox::Jingle::Session> sdMap,
                         OkRTCHandler *handler) {

//  _c->OnContentRemove(sdMap, handler);
}

void WebRTC::setMute(bool mute) {
//  for (auto it : _pcMap) {
//    it.second->setMute(mute);
//  }
}

void WebRTC::setRemoteMute(bool mute) {
//  for (auto it : _pcMap) {
//    it.second->setRemoteMute(mute);
//  }
}
void WebRTC::createPeerConnection() {}

size_t WebRTC::getVideoSize() {
    RTC_DLOG_F(LS_INFO) << "Create video device...";
    auto vdi = webrtc::VideoCaptureFactory::CreateDeviceInfo();
    RTC_DLOG_F(LS_INFO) << "Video capture numbers:"<< vdi->NumberOfDevices();
    return vdi->NumberOfDevices();
}

std::shared_ptr<VideoCaptureInterface> WebRTC::getVideoCapture(
    std::optional<std::string> deviceId,
    bool isScreenCapture) {

  if(deviceId->empty()){
    return {};
  }

  if (auto result = _videoCapture.lock()) {
    if (deviceId) {
      result->switchToDevice( *deviceId,isScreenCapture);
    }
    return result;
  }

  const auto startDeviceId =   *deviceId;

  auto result = std::shared_ptr<VideoCaptureInterface>(
      VideoCaptureInterface::Create(StaticThreads::getThreads(),startDeviceId));
  _videoCapture = result;
  return result;
}

void WebRTC::CreateOffer(const std::string &peerId) {
  Conductor *conductor = getConductor(peerId);
  conductor->CreateOffer();
}

void WebRTC::SessionTerminate(const std::string &peerId) {
//    quit(peerId);
}

void WebRTC::CreateAnswer(const std::string &peerId,
                          const std::string &sId,
                          const OJingleContentAv &pContent) {

  RTC_LOG_F(LS_INFO) << "peerId:"<< peerId ;

  Conductor *conductor = createConductor(peerId, sId, false);
  if (!conductor) {
    RTC_LOG_F(LS_WARNING) << "conductor is null!";
    return;
  }

  auto sdp = CreateSessionDescription(webrtc::SdpType::kOffer, pContent);
  conductor->SetRemoteDescription(std::move(sdp));
  conductor->CreateAnswer();

}


} // namespace ortc
} // namespace lib
