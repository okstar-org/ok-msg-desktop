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

#pragma once

#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <string>

#include <api/media_stream_interface.h>
#include <api/peer_connection_interface.h>
#include <api/video/video_frame.h>
#include <api/video/video_frame_buffer.h>
#include <api/video/video_source_interface.h>
#include <media/base/video_adapter.h>
#include <modules/video_capture/video_capture.h>
#include <modules/video_capture/video_capture_factory.h>
#include <pc/session_description.h>

#include <rtc_base/ref_count.h>

#include "../ok_rtc.h"

#include "ok_video_sink.h"
#include "webrtc.h"

namespace lib {
namespace ortc {

class Conductor : public webrtc::PeerConnectionObserver,
                  public webrtc::CreateSessionDescriptionObserver,
                  public webrtc::SetSessionDescriptionObserver,
                  public webrtc::SetRemoteDescriptionObserverInterface {
public:
    Conductor(WebRTC* webrtc, const std::string& peerId_, const std::string& sId);

    ~Conductor();

    void CreateAnswer();

    void CreateOffer();

    void sessionTerminate();

    void setTransportInfo(std::unique_ptr<webrtc::IceCandidateInterface> candidate);

    virtual void OnSessionTerminate(const std::string& sid, ortc::OkRTCHandler* handler);

    void SetRemoteDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

    void setMute(bool mute);
    void setRemoteMute(bool mute);

    inline ortc::JoinOptions joinOptions() { return _joinOptions; }

    size_t getVideoCaptureSize();

    OJingleContentAv toJingleSdp(const webrtc::SessionDescriptionInterface* desc);

    virtual void AddRef() const override {};
    virtual rtc::RefCountReleaseStatus Release() const override {
        return rtc::RefCountReleaseStatus::kDroppedLastRef;
    };

    void OnSessionAccept(std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

    bool AddAudioTrack(webrtc::AudioSourceInterface* _audioSource);
    bool RemoveAudioTrack();

    bool AddVideoTrack(webrtc::VideoTrackSourceInterface* _videoTrackSource);
    bool RemoveVideoTrack();

protected:
    void CreatePeerConnection();
    void DestroyPeerConnection();

    //
    // PeerConnectionObserver implementation.
    //
    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state) override;

    void OnConnectionChange(
            webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;

    void OnAddTrack(
            rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
            const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override;
    void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;
    void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
    void OnRenegotiationNeeded() override;
    void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState state) override;
    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState state) override;
    void OnIceCandidate(const webrtc::IceCandidateInterface* ice) override;
    void OnIceConnectionReceivingChange(bool receiving) override;
    void OnSetRemoteDescriptionComplete(webrtc::RTCError error) override;

    /**
     * SetSessionDescriptionObserver
     * implementation.
     *
     */
    void OnSuccess() override;

    /**
     * CreateSessionDescriptionObserver
     * @param desc
     */
    virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;

    /**
     * CreateSessionDescriptionObserver and SetSessionDescriptionObserver
     * @param error
     */
    virtual void OnFailure(webrtc::RTCError error) override;

    virtual bool started() const { return _started; }

private:
    bool _started = false;
    std::mutex _session_mutex;

    std::string peerId;
    std::string sId;

    WebRTC* webRtc;

    ortc::JoinOptions _joinOptions;

    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

    std::unique_ptr<VideoSink> _videoSink;

    rtc::scoped_refptr<webrtc::AudioTrackInterface> _audioTrack;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> _videoTrack;

    webrtc::AudioTrackInterface* _remote_audio_track;
    webrtc::VideoTrackInterface* _remote_video_track;

    rtc::scoped_refptr<webrtc::RtpSenderInterface> _audioRtpSender;
    rtc::scoped_refptr<webrtc::RtpSenderInterface> _videoRtpSender;
    //  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> _videoTrackSource;
};

}  // namespace ortc
}  // namespace lib
