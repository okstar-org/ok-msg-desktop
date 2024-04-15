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

#include "ok_conductor.h"

#include <memory>

#include <thread>
#include <utility>
#include <vector>

#include <api/audio_options.h>
#include <api/create_peerconnection_factory.h>
#include <api/data_channel_interface.h>
#include <api/jsep.h>
#include <modules/video_capture/video_capture.h>
#include <modules/video_capture/video_capture_factory.h>
#include <pc/video_track_source.h>

#include "vcm_capturer.h"

namespace lib {
namespace ortc {

Conductor::Conductor(
    const webrtc::PeerConnectionInterface::RTCConfiguration &config, //
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf,  //
    const std::string &peerId_,                                      //
    const std::string &sId_,                                         //
    OkRTCHandler *rtcHandler_,                                       //
    OkRTCRenderer *rtcRenderer_                                      //
    )
    : config(
          std::make_unique<webrtc::PeerConnectionInterface::RTCConfiguration>(
              config)),          //
      peerId(peerId_),           //
      sId(sId_),                 //
      rtcHandler(rtcHandler_),   //
      rtcRenderer(rtcRenderer_), //
      peer_connection_factory_(pcf), _remote_audio_track(nullptr),
      _remote_video_track(nullptr) {

  // qDebug(("begin"));

  assert(!peerId_.empty());

  assert(pcf.get());
  assert(rtcHandler);
  assert(rtcRenderer);

  peer_connection_ = CreatePeerConnection();
  // qDebug(("end"));
}

Conductor::~Conductor() {
  // qDebug(("begin"));

  if (_audioRtpSender.get()) {
    peer_connection_->RemoveTrackOrError(_audioRtpSender);
  }

  if (_videoRtpSender.get()) {
    peer_connection_->RemoveTrackOrError(_videoRtpSender);
  }

  DestroyPeerConnection();
  // qDebug(("end"));
}

rtc::scoped_refptr<webrtc::PeerConnectionInterface> Conductor::CreatePeerConnection() {
  RTC_LOG(LS_INFO) << "Create peer connection ...";
  webrtc::PeerConnectionDependencies pc_dependencies(this);

  auto maybe = peer_connection_factory_->CreatePeerConnectionOrError(*config, std::move(pc_dependencies));
  assert(maybe.ok());

  peer_connection_ = std::move(maybe.value());
  rtcHandler->onCreatePeerConnection(peerId, sId, maybe.ok());
  return peer_connection_;
}

void Conductor::DestroyPeerConnection() {
   RTC_LOG(LS_INFO) << "Destroy peer connection ...";
}

size_t Conductor::getVideoCaptureSize() {
  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
      webrtc::VideoCaptureFactory::CreateDeviceInfo());
  if (!info) {
    return 0;
  }
  int num_devices = info->NumberOfDevices();
  return num_devices;
}

void Conductor::setMute(bool mute) {
  // if (_audio_track)
  _audioTrack->set_enabled(!mute);
}

void Conductor::setRemoteMute(bool mute) {
  if (_remote_audio_track) {
    _remote_audio_track->set_enabled(!mute);
  }
}

void Conductor::AddTrack(webrtc::AudioSourceInterface *_audioSource) {

  std::string label = "-";
  std::string streamId = "okedu-audio-id";

  //  //qDebug(("kAudioLabel:%1  kAudioStreamId:%2")
  //                .arg(qstring(label))
  //                .arg(qstring(streamId)));

  assert(peer_connection_.get());

  _audioTrack = peer_connection_factory_->CreateAudioTrack(label, _audioSource);
  auto add_track_result = peer_connection_->AddTrack(_audioTrack, {streamId});

  if (!add_track_result.ok()) {
    // qDebug(("Failed to add audio track to PeerConnection:%1")
    //                  .arg(qstring(add_track_result.error().message())));
    return;
  }

  _audioRtpSender = add_track_result.value();
  // qDebug(("ssrc:%1").arg(_audioRtpSender->ssrc())) ;
}

void Conductor::AddTrack(webrtc::VideoTrackSourceInterface *_videoTrackSource) {
  std::string label = "-";
  std::string streamId = "okedu-video-id";

  // qDebug(("kVideoLabel:%1 kVideoStreamId:%2")
  //              .arg(qstring(label))
  //              .arg(qstring(streamId)));

  _videoTrack =
      peer_connection_factory_->CreateVideoTrack(label, _videoTrackSource);
  // RTC_LOG(LS_INFO) << "Added videoTrack id:" << (_videoTrack->id());

  auto add_track_result = peer_connection_->AddTrack(_videoTrack, {streamId});
  if (!add_track_result.ok()) {
    //    qDebug(("Failed to add
    //    track:%1").arg(qstring(add_track_result.error().message())));
    return;
  }
  _videoRtpSender = add_track_result.value();
  // RTC_LOG(LS_INFO) << "videoRtpSender:" << (_videoRtpSender.get());
}

void Conductor::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
  // qDebug(("OnDataChannel channel id: %1").arg(channel->id()));
}

void Conductor::OnRenegotiationNeeded() {
  // qDebug(("OnRenegotiationNeeded"));
}

/**
 * ICE 连接状态
 * @param state
 */
void Conductor::OnIceConnectionChange(
    webrtc::PeerConnectionInterface::IceConnectionState state) {
    RTC_LOG(LS_INFO) << "OnIceConnectionChange:"
      << webrtc::PeerConnectionInterface::AsString(state).data();
}

/**
 * ICE 采集状态
 * @param state
 */
void Conductor::OnIceGatheringChange(
    webrtc::PeerConnectionInterface::IceGatheringState state) {
   RTC_LOG(LS_INFO) << "OnIceGatheringChange:"
                      << webrtc::PeerConnectionInterface::AsString(state).data();
}


void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface *ice) {

  std::string str;
  ice->ToString(&str);

  // qDebug(("mid:%1=>%2").arg(qstring(ice->sdp_mid())).arg(qstring(str)));

  auto &cand = ice->candidate();

  /**
   * 发送 IceCandidate
   */
  OIceUdp iceUdp;
  iceUdp.mid = ice->sdp_mid();           //
  iceUdp.mline = ice->sdp_mline_index(); //
  iceUdp.ufrag = cand.username();
  iceUdp.pwd = cand.password();

  auto sdp = peer_connection_->local_description();
  auto transportInfos = sdp->description()->transport_infos();
  for (auto info : transportInfos) {

    if (info.content_name == ice->sdp_mid()) {
      iceUdp.dtls.hash = info.description.identity_fingerprint->algorithm;
      iceUdp.dtls.fingerprint =
          info.description.identity_fingerprint->GetRfc4572Fingerprint();
      switch (info.description.connection_role) {
      case ::cricket::CONNECTIONROLE_ACTIVE:
        iceUdp.dtls.setup = ::cricket::CONNECTIONROLE_ACTIVE_STR;
        break;
      case ::cricket::CONNECTIONROLE_ACTPASS:
        iceUdp.dtls.setup = ::cricket::CONNECTIONROLE_ACTPASS_STR;
        break;
      case ::cricket::CONNECTIONROLE_HOLDCONN:
        iceUdp.dtls.setup = ::cricket::CONNECTIONROLE_HOLDCONN_STR;
        break;
      case ::cricket::CONNECTIONROLE_PASSIVE:
        iceUdp.dtls.setup = ::cricket::CONNECTIONROLE_PASSIVE_STR;
        break;
      case ::cricket::CONNECTIONROLE_NONE:
        break;
      }
    }
  }
  // candidate
  gloox::Jingle::ICEUDP::Candidate oc;
  oc.id = cand.id();
  oc.foundation = cand.foundation();
  oc.priority = cand.priority();
  oc.protocol = cand.protocol();
  oc.tcptype = cand.tcptype();
  oc.generation = std::to_string(cand.generation());
  oc.component = std::to_string(cand.component());
  oc.network = std::to_string(cand.network_id());

  // addr
  oc.ip = cand.address().ipaddr().ToString();
  oc.port = cand.address().port();

  // “host” / “srflx” / “prflx” / “relay” / token
  if (cand.type() == ::cricket::LOCAL_PORT_TYPE) {
    oc.type = gloox::Jingle::ICEUDP::Host;
  } else if (cand.type() == ::cricket::STUN_PORT_TYPE) {
    oc.type = gloox::Jingle::ICEUDP::ServerReflexive;
  } else if (cand.type() == ::cricket::PRFLX_PORT_TYPE) {
    oc.type = gloox::Jingle::ICEUDP::PeerReflexive;
  } else if (cand.type() == ::cricket::RELAY_PORT_TYPE) {
    oc.type = gloox::Jingle::ICEUDP::Relayed;
  }

  if (oc.type != gloox::Jingle::ICEUDP::Host &&
      0 < cand.related_address().port()) {
    oc.rel_addr = cand.related_address().ipaddr().ToString();
    oc.rel_port = cand.related_address().port();
  }

  iceUdp.candidates.push_back(oc);

  rtcHandler->onIce(sId, peerId, iceUdp);
}

void Conductor::OnIceConnectionReceivingChange(bool receiving) {
  RTC_LOG(LS_INFO) << "OnIceConnectionReceivingChange:" << receiving;
}

void Conductor::OnSignalingChange(
    webrtc::PeerConnectionInterface::SignalingState state) {
   RTC_LOG(LS_INFO) << "OnSignalingChange:"
                   << webrtc::PeerConnectionInterface::AsString(state).data();
}


//
// PeerConnectionObserver implementation.
//
void Conductor::OnAddTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>
        &streams) {

  std::string receiverId = receiver->id();
  RTC_LOG(LS_INFO) << "OnAddTrack RtpReceiverId:" << receiverId;


  // track
  auto track = receiver->track();
  RTC_LOG(LS_INFO) << "Track Id:" << track->id() << " kind:"<<track->kind();

  if ( track->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
    _remote_audio_track =
        static_cast<webrtc::AudioTrackInterface *>(track.get());
  } else if ( track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
    _videoSink = std::make_unique<OVideoSink>(rtcRenderer, peerId);

    _remote_video_track = static_cast<webrtc::VideoTrackInterface *>(track.get());
    _remote_video_track->AddOrUpdateSink(_videoSink.get(),
                                         rtc::VideoSinkWants());
  }
}

void Conductor::OnTrack(
    rtc::scoped_refptr<RtpTransceiverInterface> transceiver) {
  RTC_LOG(LS_INFO) << "OnTrack mid:" << transceiver->mid()->data()
                   << " type:"<<transceiver->media_type();
}

/**
 * track删除事件
 * @brief Conductor::OnRemoveTrack
 * @param receiver
 */
void Conductor::OnRemoveTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
   RTC_LOG(LS_INFO) << "TrackId:" << receiver->id();

  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track =
      receiver->track();

  if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {

    auto remote_video_track =
        static_cast<webrtc::VideoTrackInterface *>(track.get());

    //    std::shared_ptr<OVideoSink> ovs;

    //    for (const auto& it : _VideoSinkMap) {
    //      auto vsk = it.first;
    //
    //      if (!it.second) {
    //        qDebug(LS_ERROR)).arg("VideoSink: ").arg(vsk).arg(" is
    //        null"));
    //        continue;
    //      }

    // 查询
    //      if (it.second->trackId() == remote_video_track->id()) {
    //        ovs = it.second;
    //      }
    //    }

    //    qDebug(("RemoveSink TrackId:").arg(remote_video_track->id()
    //                      ).arg(" VideoSink: ").arg(ovs;
    // 移除视频接收器
    //    remote_video_track->RemoveSink(ovs.get());

    // 释放资源
    //    ovs.reset();

    // 清空map
    //    _VideoSinkMap.erase(remote_video_track->id());

    //    qDebug(("RemoveSink TrackId:").arg(remote_video_track->id()
    //                      ).arg(" VideoSink: ").arg(ovs).arg(" is success!"));
  }
}

/**
 * CreateOffer
 */
void Conductor::CreateOffer() {
  RTC_LOG(LS_INFO) << "CreateOffer...";
  peer_connection_->SetLocalDescription(this);
  peer_connection_->CreateOffer(
      this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
  RTC_LOG(LS_INFO) << "CreateOffer has done.";
}

/**
 * @brief CreateAnswer
 *
 * @param desc
 */
void Conductor::CreateAnswer(
    std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {

  RTC_LOG(LS_INFO) << (("CreateAnswer..."));
  auto sdp = desc.release();
  peer_connection_->SetRemoteDescription(this, sdp);
  peer_connection_->CreateAnswer(
      this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
  updateCandidates();
  RTC_LOG(LS_INFO) << (("CreateAnswer has done."));
}

void Conductor::SetRemoteDescription(
    std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
  RTC_LOG(LS_INFO) << "desc type:" << desc->type();

  std::string sdp;
  desc->ToString(&sdp);
  RTC_LOG(LS_INFO) << "sdp:\n" << sdp;

  RTC_LOG(LS_INFO) <<  "SetRemoteDescription...";
  peer_connection_->SetRemoteDescription(this, desc.release());
}

void Conductor::setTransportInfo(
    std::unique_ptr<webrtc::IceCandidateInterface> candidate) {

  std::string str;
  candidate->ToString(&str);
  RTC_LOG(LS_INFO) << "setTransportInfo mid:" << candidate->sdp_mid() << " :"
                   << str;

  auto c = candidate.release();
  bool yes = peer_connection_->AddIceCandidate(c);
  RTC_LOG(LS_INFO) << "AddIceCandidate=>" << (yes);
  if (!yes) {
    _candidates.emplace_back(c);
  }
}

void Conductor::sessionTerminate() { peer_connection_->Close(); }

void Conductor::OnContentAdd(
    std::map<std::string, gloox::Jingle::Session> sdMap,
    OkRTCHandler *handler) {

  std::lock_guard<std::mutex> lock(_session_mutex);
  const webrtc::SessionDescriptionInterface *remoteSDI =
      peer_connection_->remote_description();

  std::string sdp;
  remoteSDI->ToString(&sdp);
  RTC_LOG(LS_INFO) << "OnContentAdd remote sdp:" << sdp;

  std::unique_ptr<webrtc::SessionDescriptionInterface> jsd =
      webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, sdp);

  ::cricket::ContentGroup group(::cricket::GROUP_TYPE_BUNDLE);
  for (auto it = sdMap.begin(); it != sdMap.end(); ++it) {
    std::string name = it->first;
    gloox::Jingle::Session sdp = it->second;
    ::cricket::ContentInfo *info = jsd->description()->GetContentByName(name);
    if (!info) {
      continue;
    }
  }

  std::string new_sdp;
  jsd->ToString(&new_sdp);
  // qDebug(("remote new sdp:\n%1\n").arg(qstring(new_sdp)));
  peer_connection_->SetRemoteDescription(this, jsd.release());

  // qDebug(("end"));
}

void Conductor::OnContentRemove(
    std::map<std::string, gloox::Jingle::Session> sdMap,
    OkRTCHandler *handler) {
#if 0
//  qDebug(LS_INFO);
  std::lock_guard<std::mutex> lock(_session_mutex);

  //    auto find = _VideoSinkMap.find(peer);
  //    if(find != _VideoSinkMap.end()){
  //        auto vs = _VideoSinkMap.at(peer);
  //        if(vs->disabled()){
  //            vs->setDisabled();
  //        }
  //    }

  webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_ =
      InitGetConnection();
  const webrtc::SessionDescriptionInterface *remoteSDI =
      peer_connection_->remote_description();

  std::string sdp;
  remoteSDI->ToString(&sdp);
  //qDebug(("remote orignal sdp:\n").arg(sdp;

  for (auto it = sdMap.begin(); it != sdMap.end(); ++it) {
    std::string name = it->first;
    gloox::Jingle::Session sdp = it->second;

    //qDebug(("sdp mid: ").arg(name;

    const cricket::SessionDescription *mcd = remoteSDI->description();
    const cricket::ContentInfo *info = mcd->GetContentByName(name);
    if (!info) {
      continue;
    }
    gloox::Jingle::Session::SSMAList list = sdp.rtp().ssmas;
    for (auto ssma : list) {
      const uint32_t ssrc = ssma.ssrc();
      const cricket::MediaContentDescription *mcd = info->media_description();
      cricket::StreamParamsVec &spv =
          const_cast<cricket::MediaContentDescription *>(mcd)
              ->mutable_streams();

      spv.erase(std::remove_if(spv.begin(), spv.end(),
                               [&](auto i) {
                                 bool y = i.has_ssrc(ssrc);
                                 if (y) {
                                   std::string cname = i.cname;
                                   std::string track_id = i.id;
                                   std::string stream_id = i.first_stream_id();

                                   //qDebug(LS_INFO)
                                      ).arg("Remove StreamParamsVec: ssrc: "
                                      ).arg(ssrc).arg(" cname: ").arg(cname
                                      ).arg(" track_id: ").arg(track_id
                                      ).arg(" stream_id: ").arg(stream_id;
                                 }
                                 return y;
                               }),
                spv.end());
    }



    std::string new_sdp;
    remoteSDI->ToString(&new_sdp);
    //qDebug(("remote new sdp: \n").arg(new_sdp;

    std::unique_ptr<webrtc::SessionDescriptionInterface> jsd =
        webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, new_sdp);

    peer_connection_->SetRemoteDescription(this, jsd.release());

    //qDebug(("end peer:"));
#endif
}

void Conductor::OnSessionTerminate(const std::string &sid,
                                   OkRTCHandler *handler) {
  // qDebug(("begin sid:").arg(qstring(sid)));
  std::lock_guard<std::mutex> lock(_session_mutex);
  //  Shutdown();
  // qDebug(("end"));
}

JingleContents
Conductor::toJingleSdp(const webrtc::SessionDescriptionInterface *desc) {

  assert(desc);
  //  assert(jingleContext);
  //
  //  jingleContext->sessionId = desc->session_id();
  //  jingleContext->sessionVersion = desc->session_version();

  JingleContents contents;
  // ContentGroup
  ::cricket::ContentGroup group(::cricket::GROUP_TYPE_BUNDLE);

  auto sd = desc->description();
  for (auto rtcContent : sd->contents()) {
    OContent oContent;

    const std::string &name = rtcContent.mid();
    // qDebug(("Content name: %1").arg(qstring(name)));

    oContent.name = name;

    auto mediaDescription = rtcContent.media_description();
    // media type
    auto mt = mediaDescription->type();

    // rtcp_mux
    oContent.rtp.rtcpMux = mediaDescription->rtcp_mux();

    // Transport
    auto ti = sd->GetTransportInfoByName(name);

    // pwd ufrag
    oContent.iceUdp.pwd = ti->description.ice_pwd;
    oContent.iceUdp.ufrag = ti->description.ice_ufrag;

    // fingerprint
    if (ti->description.identity_fingerprint) {
      oContent.iceUdp.dtls.fingerprint =
          ti->description.identity_fingerprint->GetRfc4572Fingerprint();
      oContent.iceUdp.dtls.hash =
          ti->description.identity_fingerprint->algorithm;

      // connection_role
      std::string setup;
      ::cricket::ConnectionRoleToString(ti->description.connection_role,
                                        &setup);
      oContent.iceUdp.dtls.setup = setup;
    }

    // hdrext
    const ::cricket::RtpHeaderExtensions hdrs =
        mediaDescription->rtp_header_extensions();
    for (auto &hdr : hdrs) {
      gloox::Jingle::RTP::HdrExt hdrExt = {hdr.id, hdr.uri};
      oContent.rtp.hdrExts.push_back(hdrExt);
    }

    // ssrc
    for (auto &stream : mediaDescription->streams()) {
      //      "{id:5e9a64d8-b9d3-4fc7-a8eb-0ee6dec72138;  //track id
      //      ssrcs:[1679428189,751024037];
      //      ssrc_groups:{semantics:FID;
      //      ssrcs:[1679428189,751024037]};
      //      cname:dBhnE4FRSAUq1FZp;
      //      stream_ids:okedu-video-id;}"
      // RTC_LOG(LS_INFO) << "stream: " << (stream.ToString());

      // label
      const std::string &first_stream_id = stream.first_stream_id();

      for (auto &ssrc1 : stream.ssrcs) {
        // RTC_LOG(LS_INFO) << "label" << (first_stream_id) << "id"
                          //  << (stream.id) << "ssrc" << ssrc1;
        gloox::Jingle::RTP::Parameter cname = {"cname", stream.cname};
        gloox::Jingle::RTP::Parameter label = {"label", stream.id};
        gloox::Jingle::RTP::Parameter mslabel = {"mslabel", first_stream_id};
        gloox::Jingle::RTP::Parameter msid = {"msid", first_stream_id + " " +
                                                          stream.id};

        // msid = mslabel+ label(stream.id)
        RTP::Parameters parameters;
        parameters.emplace_back(cname);
        parameters.emplace_back(msid);
        parameters.emplace_back(mslabel);
        parameters.emplace_back(label);

        gloox::Jingle::RTP::Source source = {std::to_string(ssrc1), parameters};
        oContent.rtp.sources.emplace_back(source);
      }
    }

    // ssrc-group
    if (!oContent.rtp.sources.empty()) {
      oContent.rtp.ssrcGroup.semantics = "FID";
      for (auto &ssrc : oContent.rtp.sources) {
        oContent.rtp.ssrcGroup.ssrcs.emplace_back(ssrc.ssrc);
      }
    }

    // codecs
    switch (mt) {
    case ::cricket::MediaType::MEDIA_TYPE_AUDIO: {
      oContent.rtp.media = gloox::Jingle::audio;
      auto audio_desc = mediaDescription->as_audio();
      auto codecs = audio_desc->codecs();

      for (auto &codec : codecs) {
        gloox::Jingle::RTP::PayloadType type;
        type.id = codec.id;
        type.name = codec.name;
        type.channels = codec.channels;
        type.clockrate = codec.clockrate;
        type.bitrate = codec.bitrate;

        auto cps = codec.ToCodecParameters();
        for (auto &it : cps.parameters) {
          gloox::Jingle::RTP::Parameter parameter;
          if (parameter.name.empty())
            continue;
          parameter.name = it.first;
          parameter.value = it.second;
          type.parameters.emplace_back(parameter);
        }

        // rtcp-fb
        for (auto &it : codec.feedback_params.params()) {
          gloox::Jingle::RTP::Feedback fb = {it.id(), it.param()};
          type.feedbacks.push_back(fb);
        }

        oContent.rtp.payloadTypes.emplace_back(type);
      }

      break;
    }
    case ::cricket::MediaType::MEDIA_TYPE_VIDEO: {
      oContent.rtp.media = (gloox::Jingle::video);
      auto video_desc = mediaDescription->as_video();
      for (auto &codec : video_desc->codecs()) {
        // PayloadType
        gloox::Jingle::RTP::PayloadType type;
        type.id = codec.id;
        type.name = codec.name;
        type.clockrate = codec.clockrate;

        // PayloadType parameter
        auto cps = codec.ToCodecParameters();
        for (auto &it : cps.parameters) {
          gloox::Jingle::RTP::Parameter parameter;
          parameter.name = it.first;
          parameter.value = it.second;
          type.parameters.emplace_back(parameter);
        }

        // rtcp-fb
        for (auto &it : codec.feedback_params.params()) {
          gloox::Jingle::RTP::Feedback fb = {it.id(), it.param()};
          type.feedbacks.push_back(fb);
        }

        oContent.rtp.payloadTypes.emplace_back(type);
      }
      break;
    }
    case ::cricket::MediaType::MEDIA_TYPE_DATA: {
      break;
    }
    case cricket::MEDIA_TYPE_UNSUPPORTED:
      break;
    }

    contents.emplace_back(oContent);
  }

  return contents;
}

/**
 * SetLocalDescription/SetRemoteDescription
 */
void Conductor::OnSuccess() {
  // qDebug(("SetLocalDescription or SetRemoteDescription=>OnSuccess"));
}

/**
 * CreateAnswer callback interface.
 * @param desc
 */
void Conductor::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
  // qDebug(("desc type:%1").arg(qstring(desc->type())));
  std::string sdp;
  desc->ToString(&sdp);

  // qDebug(("sdp:\n%1\n").arg(qstring(sdp)));
  peer_connection_->SetLocalDescription(this, desc);

  JingleSdpType jingleSdpType = JingleSdpType::Answer;

  switch (desc->GetType()) {
  case webrtc::SdpType::kAnswer:
    jingleSdpType = JingleSdpType::Answer;
    break;
  case webrtc::SdpType::kOffer:
    jingleSdpType = JingleSdpType::Offer;
    break;
  case webrtc::SdpType::kPrAnswer:
    jingleSdpType = JingleSdpType::Answer;
    break;
  case webrtc::SdpType::kRollback:
    break;
  }

  auto contents = toJingleSdp(desc);
  JingleContext jingleContext(jingleSdpType, peerId, sId,
                              desc->session_version(), contents);
  rtcHandler->onRTP(sId, peerId, jingleContext);
}

void Conductor::OnFailure(webrtc::RTCError error) {
  // qDebug(("error:%1").arg(qstring(error.message())));
}

void Conductor::OnSetRemoteDescriptionComplete(webrtc::RTCError error) {
  // qDebug(("error:%1").arg(qstring(error.message())));
}

void Conductor::OnConnectionChange(
    webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
  // qDebug(("new_state=>%1").arg((webrtc::PeerConnectionInterface::AsString(new_state).data())));
}

void Conductor::OnSessionAccept(
    std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
  // qDebug(("type:%1").arg(qstring(desc->type())));
  SetRemoteDescription(std::move(desc));
  updateCandidates();
}

void Conductor::updateCandidates() {
  // qDebug(("updateCandidates"));
  auto it = _candidates.begin();
  for (; it != _candidates.end();) {
    auto candidate = *it;

    std::string str;
    candidate->ToString(&str);
    // qDebug(("updateCandidates
    // sdp_mid:%1=>%2").arg(qstring(candidate->sdp_mid())).arg(qstring(str)));

    // 规范：https://datatracker.ietf.org/doc/html/rfc5245#section-15.1
    // 格式：candidate:1022514418 1 udp 2122197247 192.168.8.2 44868 typ host
    // generation 0
    bool add = peer_connection_->AddIceCandidate(candidate);
    // qDebug(("AddIceCandidate=>%1").arg(add));
    it = _candidates.erase(it);
  }
}

} // namespace ortc
} // namespace lib
