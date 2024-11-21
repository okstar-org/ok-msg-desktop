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

Conductor::Conductor(WebRTC* webrtc, const std::string& peerId_, const std::string& sId_)
        : peerId(peerId_)
        , sId(sId_)
        , webRtc{webrtc}
        , _remote_audio_track(nullptr)
        , _remote_video_track(nullptr) {
    RTC_LOG(LS_INFO) << "sId:" << sId << "peerId:" << peerId;

    assert(!peerId.empty());
    assert(!sId.empty());
    assert(webRtc);

    webRtc->ensureStart();
    assert(webRtc->isStarted());
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

void Conductor::OnRenegotiationNeeded() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
}

/**
 * ICE 收集状态
 * @param state
 */
void Conductor::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState state) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "=>"
                     << webrtc::PeerConnectionInterface::AsString(state).data();

    if (webRtc->getHandler()) {
        webRtc->getHandler()->onIceGatheringChange(
                sId, peerId, static_cast<ortc::IceGatheringState>(state));
    }
}

/**
 * ICE 连接状态
 * @param state
 */
void Conductor::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState state) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "=>"
                     << webrtc::PeerConnectionInterface::AsString(state).data();
    if (webRtc->getHandler()) {
        webRtc->getHandler()->onIceConnectionChange(
                sId, peerId, static_cast<ortc::IceConnectionState>(state));
    }
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* ice) {
    std::string str;
    ice->ToString(&str);
    RTC_LOG(LS_INFO) << __FUNCTION__ << " Candidate:" << str;
    _candidates.push_back(str);
}

void Conductor::OnIceConnectionReceivingChange(bool receiving) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << receiving;
}

void Conductor::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "=>"
                     << webrtc::PeerConnectionInterface::AsString(state).data();
    if (webRtc->getHandler()) {
        webRtc->getHandler()->onSignalingChange(
                sId, peerId, static_cast<ortc::SignalingState>(state));
    }
}

void Conductor::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState state) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "=>"
                     << webrtc::PeerConnectionInterface::AsString(state).data();

    if (webRtc->getHandler()) {
        webRtc->getHandler()->onPeerConnectionChange(
                sId, peerId, static_cast<ortc::PeerConnectionState>(state));
    }
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

    //    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = receiver->track();
    //    if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
    //        auto remote_video_track = static_cast<webrtc::VideoTrackInterface*>(track.get());
    //    }
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
    RTC_LOG(LS_INFO) << __FUNCTION__ << " ...";
    peer_connection_->CreateAnswer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    RTC_LOG(LS_INFO) << __FUNCTION__ << " done.";
}

void Conductor::SetRemoteDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " desc type:" << desc->type();

    std::string sdp;
    desc->ToString(&sdp);
    RTC_LOG(LS_INFO) << "set remote sdp:\n" << sdp;

    peer_connection_->SetRemoteDescription(this, desc.release());
}

bool Conductor::setTransportInfo(std::unique_ptr<webrtc::IceCandidateInterface> candidate) {
    std::string str;
    candidate->ToString(&str);
    RTC_LOG(LS_INFO) << __FUNCTION__ << " set remote candidate:"
                     << " mid:" << candidate->sdp_mid()
                     << " mline: " << candidate->sdp_mline_index() << " | " << str;

    auto added = peer_connection_->AddIceCandidate(candidate.release());
    RTC_LOG(LS_INFO) << __FUNCTION__ << " => " << added;
    return added;
}

void Conductor::sessionTerminate() {
    peer_connection_->Close();
}

void Conductor::OnSessionTerminate(const std::string& sid, OkRTCHandler* handler) {}

void Conductor::OnSuccess() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    std::string sdp;
    desc->ToString(&sdp);

    RTC_LOG(LS_INFO) << __FUNCTION__ << "sdp:" << sdp;
    //    peer_connection_->SetLocalDescription(this, desc);
}

void Conductor::OnFailure(webrtc::RTCError error) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << error.message();

    if (webRtc->getHandler()) {
        webRtc->getHandler()->onFailure(sId, peerId, error.message());
    }
}

void Conductor::OnSetRemoteDescriptionComplete(webrtc::RTCError error) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " : " << error.message();
}

const webrtc::SessionDescriptionInterface* Conductor::getLocalSdp() const {
    return peer_connection_->local_description();
}

}  // namespace ortc
}  // namespace lib
