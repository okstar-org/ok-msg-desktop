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
#include <string>
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
#include "StaticThreads.h"

namespace lib {
namespace ortc {

Conductor::Conductor(WebRTC* webrtc,
                     const std::string& peerId_,  //
                     const std::string& sId_      //
                     )
        : peerId(peerId_)
        ,  //
        sId(sId_)
        ,  //

        webRtc{webrtc}
        , _remote_audio_track(nullptr)
        , _remote_video_track(nullptr) {
    RTC_LOG(LS_INFO) << "...";

    assert(webRtc);
    assert(webRtc->ensureStart());

    assert(!peerId_.empty());

    CreatePeerConnection();

    RTC_LOG(LS_INFO) << __FUNCTION__;
}

Conductor::~Conductor() {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "...";
    DestroyPeerConnection();
    RTC_LOG(LS_INFO) << __FUNCTION__ << "Destroyed";
}

void Conductor::CreatePeerConnection() {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "...";

    webrtc::PeerConnectionDependencies pc_dependencies(this);

    auto maybe = webRtc->getFactory()->CreatePeerConnectionOrError(webRtc->getConfig(),
                                                                   std::move(pc_dependencies));
    if (webRtc->getHandler()) {
        webRtc->getHandler()->onCreatePeerConnection(sId, peerId, maybe.ok());
    }

    if (!maybe.ok()) {
        return;
    }

    peer_connection_ = maybe.value();
    RTC_LOG(LS_INFO) << __FUNCTION__ << " done.";
}

void Conductor::DestroyPeerConnection() {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "...";
    RemoveAudioTrack();
    RemoveVideoTrack();
    peer_connection_->Close();
    peer_connection_.release();
    RTC_LOG(LS_INFO) << __FUNCTION__ << " done.";
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
    RTC_LOG(LS_INFO) << __FUNCTION__;
    if (_audioTrack) {
        _audioTrack->set_enabled(!mute);
    }
}

void Conductor::setRemoteMute(bool mute) {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    if (_remote_audio_track) {
        _remote_audio_track->set_enabled(!mute);
    }
}

bool Conductor::AddAudioTrack(webrtc::AudioSourceInterface* _audioSource) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << ":" << _audioSource;

    std::string label = "ok-audio-label";
    _audioTrack = webRtc->getFactory()->CreateAudioTrack(label, _audioSource);
    RTC_LOG(LS_INFO) << "Created audio track:" << _videoTrack.get();

    std::string streamId = "ok-audio-stream";
    auto added = peer_connection_->AddTrack(_audioTrack, {streamId});
    if (!added.ok()) {
        RTC_LOG(LS_INFO) << "Failed to add track:%1" << added.error().message();
        return false;
    }

    _audioRtpSender = added.value();
    RTC_LOG(LS_INFO) << "Audio rtp sender:" << _audioRtpSender.get();
    return true;
}

bool Conductor::RemoveAudioTrack() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    auto result = peer_connection_->RemoveTrackOrError(_audioRtpSender);
    return result.ok();
}

bool Conductor::AddVideoTrack(webrtc::VideoTrackSourceInterface* _videoTrackSource) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << ":" << _videoTrackSource;

    std::string label = "ok-video-track-label";

    _videoTrack = webRtc->getFactory()->CreateVideoTrack(label, _videoTrackSource);
    RTC_LOG(LS_INFO) << "Created video track:" << _videoTrack.get();

    //  _videoTrack->AddOrUpdateSink(new VideoSink(_rtcRenderer),
    //        // rtc::VideoSinkWants()); qDebug(("Added video track, The device num
    //         // is:%1").arg(i));

    std::string streamId = "ok-video-stream";
    auto added = peer_connection_->AddTrack(_videoTrack, {streamId});

    if (!added.ok()) {
        RTC_LOG(LS_INFO) << "Failed to add track:" << added.error().message();
        return false;
    }

    _videoRtpSender = added.value();
    RTC_LOG(LS_INFO) << "Video rtp sender:" << _videoRtpSender.get();

    return true;
}

bool Conductor::RemoveVideoTrack() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    auto result = peer_connection_->RemoveTrackOrError(_videoRtpSender);
    return result.ok();
}

void Conductor::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "OnDataChannel channel id:" << channel->id();
}

void Conductor::OnRenegotiationNeeded() { RTC_LOG(LS_INFO) << __FUNCTION__; }

/**
 * ICE 连接状态
 * @param state
 */
void Conductor::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState state) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "=>"
                     << webrtc::PeerConnectionInterface::AsString(state).data();
}

void Conductor::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState state) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "=>"
                     << webrtc::PeerConnectionInterface::AsString(state).data();
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* ice) {
    std::string str;
    ice->ToString(&str);

    RTC_LOG(LS_INFO) << __FUNCTION__ << "=> mid:" << ice->sdp_mid() << " " << str;

    auto& cand = ice->candidate();

    /**
     * 发送 IceCandidate
     */
    OIceUdp iceUdp;
    iceUdp.mid = ice->sdp_mid();            //
    iceUdp.mline = ice->sdp_mline_index();  //
    iceUdp.ufrag = cand.username();
    iceUdp.pwd = cand.password();

    auto sdp = peer_connection_->local_description();
    auto transportInfos = sdp->description()->transport_infos();
    for (auto info : transportInfos) {
        if (info.content_name == ice->sdp_mid()) {
            if (info.description.identity_fingerprint) {
                iceUdp.dtls.hash = info.description.identity_fingerprint->algorithm;
                iceUdp.dtls.fingerprint =
                        info.description.identity_fingerprint->GetRfc4572Fingerprint();
            }

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
    Candidate oc;
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
        oc.type = Type::Host;
    } else if (cand.type() == ::cricket::STUN_PORT_TYPE) {
        oc.type = Type::ServerReflexive;
    } else if (cand.type() == ::cricket::PRFLX_PORT_TYPE) {
        oc.type = Type::PeerReflexive;
    } else if (cand.type() == ::cricket::RELAY_PORT_TYPE) {
        oc.type = Type::Relayed;
    }

    if (oc.type != Type::Host && 0 < cand.related_address().port()) {
        oc.rel_addr = cand.related_address().ipaddr().ToString();
        oc.rel_port = cand.related_address().port();
    }

    iceUdp.candidates.push_back(oc);

    webRtc->getHandler()->onIce(sId, peerId, iceUdp);
}

void Conductor::OnIceConnectionReceivingChange(bool receiving) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << receiving;
}

void Conductor::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "=>"
                     << webrtc::PeerConnectionInterface::AsString(state).data();
}

void Conductor::OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
    std::string receiverId = receiver->id();
    RTC_LOG(LS_INFO) << __FUNCTION__ << "receiver id:" << receiverId;

    // track
    auto track = receiver->track();
    RTC_LOG(LS_INFO) << __FUNCTION__ << "track id:" << track->id() << " kind:" << track->kind();

    if (track->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
        _remote_audio_track = static_cast<webrtc::AudioTrackInterface*>(track.get());
    } else if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
        _videoSink = std::make_unique<VideoSink>(webRtc->getHandler(), peerId);
        _remote_video_track = static_cast<webrtc::VideoTrackInterface*>(track.get());
        _remote_video_track->AddOrUpdateSink(_videoSink.get(), rtc::VideoSinkWants());
    }
}

void Conductor::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " mid:" << transceiver->mid()->data()
                     << " type:" << transceiver->media_type();
}

/**
 * track删除事件
 * @brief Conductor::OnRemoveTrack
 * @param receiver
 */
void Conductor::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
    RTC_LOG(LS_INFO) << "TrackId:" << receiver->id();

    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = receiver->track();

    if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
        auto remote_video_track = static_cast<webrtc::VideoTrackInterface*>(track.get());
    }
}

/**
 * CreateOffer
 */
void Conductor::CreateOffer() {
    RTC_LOG(LS_INFO) << "CreateOffer...";
    peer_connection_->SetLocalDescription(this);
    peer_connection_->CreateOffer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    RTC_LOG(LS_INFO) << "CreateOffer has done.";
}

/**
 * @brief CreateAnswer
 *
 * @param desc
 */
void Conductor::CreateAnswer() {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "...";
    peer_connection_->CreateAnswer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    RTC_LOG(LS_INFO) << __FUNCTION__ << "done.";
}

void Conductor::SetRemoteDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "desc type:" << desc->type();

    std::string sdp;
    desc->ToString(&sdp);
    RTC_LOG(LS_INFO) << "sdp:\n" << sdp;

    peer_connection_->SetRemoteDescription(this, desc.release());
}

void Conductor::setTransportInfo(std::unique_ptr<webrtc::IceCandidateInterface> candidate) {
    // candidate:2956132637 2 udp 41623294 122.9.45.183 58616 typ relay raddr 116.162.2.202 rport
    // 33010 generation 0
    std::string str;
    candidate->ToString(&str);
    RTC_LOG(LS_INFO) << __FUNCTION__ << " mid:" << candidate->sdp_mid() << " " << str;

    auto c = candidate.release();
    auto added = peer_connection_->AddIceCandidate(c);

    RTC_LOG(LS_INFO) << __FUNCTION__ << " => " << added;
}

void Conductor::sessionTerminate() { peer_connection_->Close(); }

void Conductor::OnSessionTerminate(const std::string& sid, OkRTCHandler* handler) {}

OJingleContentAv Conductor::toJingleSdp(const webrtc::SessionDescriptionInterface* desc) {
    OJingleContentAv osdp;
    osdp.sessionId = desc->session_id();
    osdp.sessionVersion = desc->session_version();

    // ContentGroup
    ::cricket::ContentGroup group(::cricket::GROUP_TYPE_BUNDLE);

    auto sd = desc->description();
    for (auto rtcContent : sd->contents()) {
        OSdp oContent;

        const std::string& name = rtcContent.mid();
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
            oContent.iceUdp.dtls.hash = ti->description.identity_fingerprint->algorithm;

            // connection_role
            std::string setup;
            ::cricket::ConnectionRoleToString(ti->description.connection_role, &setup);
            oContent.iceUdp.dtls.setup = setup;
        }

        // hdrext
        const ::cricket::RtpHeaderExtensions hdrs = mediaDescription->rtp_header_extensions();
        for (auto& hdr : hdrs) {
            HdrExt hdrExt = {hdr.id, hdr.uri};
            oContent.rtp.hdrExts.push_back(hdrExt);
        }

        // ssrc
        for (auto& stream : mediaDescription->streams()) {
            //      "{id:5e9a64d8-b9d3-4fc7-a8eb-0ee6dec72138;  //track id
            //      ssrcs:[1679428189,751024037];
            //      ssrc_groups:{semantics:FID; ssrcs:[1679428189,751024037]};
            //      cname:dBhnE4FRSAUq1FZp;
            //      stream_ids:okedu-video-id;
            // }"
            RTC_LOG(LS_INFO) << "stream: " << (stream.ToString());

            // label
            const std::string& first_stream_id = stream.first_stream_id();

            for (auto& ssrc1 : stream.ssrcs) {
                RTC_LOG(LS_INFO) << " label:" << first_stream_id << " id:" << stream.id << " ssrc"
                                 << ssrc1;

                Parameter cname = {"cname", stream.cname};
                Parameter label = {"label", stream.id};
                Parameter mslabel = {"mslabel", first_stream_id};
                Parameter msid = {"msid", first_stream_id + " " + stream.id};

                // msid = mslabel+ label(stream.id)
                Parameters parameters;
                parameters.emplace_back(cname);
                parameters.emplace_back(msid);
                parameters.emplace_back(mslabel);
                parameters.emplace_back(label);

                Source source = {std::to_string(ssrc1), parameters};
                oContent.rtp.sources.emplace_back(source);
            }
        }

        // ssrc-group
        if (oContent.rtp.sources.size() >= 2) {
            oContent.rtp.ssrcGroup.semantics = "FID";
            for (auto& ssrc : oContent.rtp.sources) {
                oContent.rtp.ssrcGroup.ssrcs.emplace_back(ssrc.ssrc);
            }
        }

        // codecs
        switch (mt) {
            case ::cricket::MediaType::MEDIA_TYPE_AUDIO: {
                oContent.rtp.media = Media::audio;
                auto audio_desc = mediaDescription->as_audio();
                auto codecs = audio_desc->codecs();

                for (auto& codec : codecs) {
                    PayloadType type;
                    type.id = codec.id;
                    type.name = codec.name;
                    type.channels = codec.channels;
                    type.clockrate = codec.clockrate;
                    type.bitrate = codec.bitrate;

                    auto cps = codec.ToCodecParameters();
                    for (auto& it : cps.parameters) {
                        Parameter parameter;
                        if (parameter.name.empty()) continue;
                        parameter.name = it.first;
                        parameter.value = it.second;
                        type.parameters.emplace_back(parameter);
                    }

                    // rtcp-fb
                    for (auto& it : codec.feedback_params.params()) {
                        Feedback fb = {it.id(), it.param()};
                        type.feedbacks.push_back(fb);
                    }

                    oContent.rtp.payloadTypes.emplace_back(type);
                }

                break;
            }
            case ::cricket::MediaType::MEDIA_TYPE_VIDEO: {
                oContent.rtp.media = Media::video;
                auto video_desc = mediaDescription->as_video();
                for (auto& codec : video_desc->codecs()) {
                    // PayloadType
                    PayloadType type;
                    type.id = codec.id;
                    type.name = codec.name;
                    type.clockrate = codec.clockrate;

                    // PayloadType parameter
                    auto cps = codec.ToCodecParameters();
                    for (auto& it : cps.parameters) {
                        Parameter parameter;
                        parameter.name = it.first;
                        parameter.value = it.second;
                        type.parameters.emplace_back(parameter);
                    }

                    // rtcp-fb
                    for (auto& it : codec.feedback_params.params()) {
                        Feedback fb = {it.id(), it.param()};
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

        osdp.contents.push_back(oContent);
    }

    return osdp;
}

void Conductor::OnSuccess() { RTC_LOG(LS_INFO) << __FUNCTION__; }

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    RTC_LOG(LS_INFO) << __FUNCTION__;

    std::string sdp;
    desc->ToString(&sdp);

    RTC_LOG(LS_INFO) << "sdp:" << sdp;
    peer_connection_->SetLocalDescription(this, desc);

    if (webRtc->getHandler()) {
        auto osdp = webRtc->convertFromSdp(desc);
        webRtc->getHandler()->onRTP(sId, peerId, osdp);
    }
}

void Conductor::OnFailure(webrtc::RTCError error) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << error.message();
}

void Conductor::OnSetRemoteDescriptionComplete(webrtc::RTCError error) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " : " << error.message();
}

void Conductor::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " : "
                     << webrtc::PeerConnectionInterface::AsString(new_state).data();
}

void Conductor::OnSessionAccept(std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
    // qDebug(("type:%1").arg(qstring(desc->type())));
    SetRemoteDescription(std::move(desc));
}

}  // namespace ortc
}  // namespace lib
