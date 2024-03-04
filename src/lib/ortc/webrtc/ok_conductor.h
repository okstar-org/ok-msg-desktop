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

#include "../ok_rtc_proxy.h"

#include "ok_videosink.h"
#include "vcm_capturer.h"

namespace lib {
namespace ortc {

class Conductor : public webrtc::PeerConnectionObserver,
                  public webrtc::CreateSessionDescriptionObserver,
                  public webrtc::SetSessionDescriptionObserver,
                  public webrtc::SetRemoteDescriptionObserverInterface {
public:
  Conductor(const webrtc::PeerConnectionInterface::RTCConfiguration &config, //
            rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf,
            const std::string &peerId_,                                       //
            const std::string &sId,
            OkRTCHandler *rtcHandler,
            OkRTCRenderer *rtcRenderer);

  virtual ~Conductor() override;

  void CreateAnswer(std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

  void CreateOffer();

  void sessionTerminate();

  void
  setTransportInfo(std::unique_ptr<webrtc::IceCandidateInterface> candidate);

  virtual void OnContentAdd(std::map<std::string, gloox::Jingle::Session> sdMap,
                            ortc::OkRTCHandler *handler);

  virtual void
  OnContentRemove(std::map<std::string, gloox::Jingle::Session> sdMap,
                  ortc::OkRTCHandler *handler);

  virtual void OnSessionTerminate(const std::string &sid,
                                  ortc::OkRTCHandler *handler);


  void SetRemoteDescription(
      std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

  virtual void setMute(bool mute);
  virtual void setRemoteMute(bool mute);

  inline ortc::JoinOptions joinOptions() { return _joinOptions; }

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> getConnection() {
    return peer_connection_;
  }

  size_t getVideoCaptureSize();

  JingleContents toJingleSdp(const webrtc::SessionDescriptionInterface *desc);

  virtual void AddRef() const {};
  virtual rtc::RefCountReleaseStatus Release() const {
    return rtc::RefCountReleaseStatus::kDroppedLastRef;
  };

  void
  OnSessionAccept(std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

  void AddTrack(webrtc::AudioSourceInterface* _audioSource);

  void AddTrack(webrtc::VideoTrackSourceInterface* _videoTrackSource);

protected:
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> CreatePeerConnection();
  void DestroyPeerConnection();

  //
  // PeerConnectionObserver implementation.
  //
  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState state) override;

  void OnConnectionChange(
      webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;

  void
  OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
             const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>
                 &streams) override;
  void
  OnTrack(rtc::scoped_refptr<RtpTransceiverInterface> transceiver) override;
  void OnRemoveTrack(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
  void OnRenegotiationNeeded() override;
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState state) override;
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState state) override;
  void OnIceCandidate(const webrtc::IceCandidateInterface *ice) override;
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
  virtual void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;

  /**
   * CreateSessionDescriptionObserver and SetSessionDescriptionObserver
   * @param error
   */
  virtual void OnFailure(webrtc::RTCError error) override;

  virtual bool started() const { return _started; }

private:
  void updateCandidates();

  bool _started = false;
  std::mutex _session_mutex;

  std::unique_ptr<webrtc::PeerConnectionInterface::RTCConfiguration> config;
  std::string peerId;
  std::string sId;

  OkRTCHandler *rtcHandler;
  OkRTCRenderer *rtcRenderer;

  ortc::JoinOptions _joinOptions;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;

  std::list<webrtc::IceCandidateInterface *> _candidates;
  std::unique_ptr<OVideoSink> _videoSink;

//  std::map<std::string, std::shared_ptr<OVideoSink>> _VideoSinkMap;

  rtc::scoped_refptr<webrtc::AudioTrackInterface> _audioTrack;
  rtc::scoped_refptr<webrtc::VideoTrackInterface> _videoTrack;

  webrtc::AudioTrackInterface * _remote_audio_track;
  webrtc::VideoTrackInterface * _remote_video_track;

  rtc::scoped_refptr<RtpSenderInterface> _audioRtpSender;
  rtc::scoped_refptr<RtpSenderInterface> _videoRtpSender;
//  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> _videoTrackSource;
};

} // namespace ortc
} // namespace lib
