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

#include <map>
#include <string>
#include <mutex>

#include "api/scoped_refptr.h"
#include "api/media_stream_interface.h"
#include "modules/video_capture/video_capture.h"

#include "../ok_rtc_defs.h"
#include "../ok_rtc_proxy.h"
#include "vcm_capturer.h"

namespace rtc {
class Thread;
}
namespace webrtc {
class PeerConnectionFactoryInterface;
class SessionDescriptionInterface;
class AudioEncoderFactory;
class AudioDecoderFactory;
class VideoEncoderFactory;
class VideoDecoderFactory;
} // namespace webrtc

namespace lib {
namespace ortc {

class Conductor;



class ORTC : public OkRTCProxy {
public:
  ORTC(std::list<IceServer> iceServers, OkRTCHandler *handler, OkRTCRenderer *renderer);

  virtual ~ORTC();



  void
  SetRemoteDescription(const std::string &peerId,
                       const lib::ortc::JingleContext &jingleContext) override;

  void CreateOffer(const std::string &peerId) ;

  void CreateAnswer(const std::string &peerId,
                    const lib::ortc::JingleContext &pContent) override;

  bool SetTransportInfo(const std::string &peerId, //
                        const ortc::OIceUdp &iceUdp);

  void ContentAdd(std::map<std::string, gloox::Jingle::Session> sdMap,
                  ortc::OkRTCHandler *handler) override;

  void ContentRemove(std::map<std::string, gloox::Jingle::Session> sdMap,
                     OkRTCHandler *handler) override;

  void SessionTerminate(const std::string &peerId) override;

  void setMute(bool mute) override;

  void setRemoteMute(bool mute) ;

  void start();

  void shutdown();

  void createPeerConnection() override;

  size_t getVideoSize() override;

  bool join(const std::string &peerId,
            const std::string &sId,
            const JingleContext& context) override;

  bool call(const std::string &peerId,
            const std::string &sId,
            JingleCallType callType) override;

  bool quit(const std::string &peerId) override;

  std::list<IceServer>& iceOptions (){
      return _iceOptions;
  }

private:

  Conductor *createConductor(const std::string &peerId,
                             const std::string &sId,
                             JingleCallType callType);

  Conductor *getConductor(const std::string &peerId);

  void setTransportInfo(Conductor *conductor, const ortc::OIceUdp &iceUdp);


  bool _started = false;
  bool _shutdown = false;
  std::mutex _start_shutdown_mtx;


  std::list<IceServer> _iceOptions;

  std::unique_ptr<rtc::Thread> network_thread;
  std::unique_ptr<rtc::Thread> worker_thread;
  std::unique_ptr<rtc::Thread> signaling_thread;
  // TODO:移除 自己
  std::unique_ptr<Conductor> _c;
  std::map<std::string, Conductor *> _pcMap;

  OkRTCRenderer *_rtcRenderer;
  OkRTCHandler *_rtcHandler;

  /**
   * 音频源
   */
  rtc::scoped_refptr<webrtc::AudioSourceInterface> _audioSource;

  /**
   * 视频源
   */
//  std::unique_ptr<VcmCapturer> capturer;
  std::shared_ptr<webrtc::VideoCaptureModule::DeviceInfo> _videoDeviceInfo;
  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> _videoTrackSource;
  rtc::scoped_refptr<webrtc::VideoTrackInterface> _video_track;


  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;
};
} // namespace ortc
} // namespace lib
