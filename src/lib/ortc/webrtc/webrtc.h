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
#include <mutex>
#include <string>

#include <api/peer_connection_interface.h>
#include <pc/session_description.h>
#include <rtc_base/thread.h>
#include <optional>

#include "../ok_rtc.h"
#include "../ok_rtc_defs.h"
#include "VideoCaptureInterface.h"

namespace webrtc {
class PeerConnectionFactoryInterface;
class SessionDescriptionInterface;
class AudioSourceInterface;
class VideoEncoderFactory;
class VideoDecoderFactory;
}  // namespace webrtc

namespace lib::ortc {

class Conductor;

// std::unique_ptr<cricket::AudioContentDescription> createAudioDescription(
//         const Sources& sources, const SsrcGroup& g);

std::unique_ptr<cricket::AudioContentDescription> createAudioDescription(
        const ORTP& rtp,  //
        const std::map<std::string, OMeetSSRCBundle>&);

std::unique_ptr<cricket::VideoContentDescription> createVideoDescription(
        const ORTP& rtp,  //
        const std::map<std::string, OMeetSSRCBundle>& ssrcBundleMap);
std::unique_ptr<cricket::SctpDataContentDescription> createDataDescription(const OSdp& sdp);

class WebRTC : public OkRTC {
public:
    WebRTC();

    ~WebRTC();

    bool start() override;

    bool stop() override;

    bool isStarted() override;

    bool ensureStart() override;

    void setRemoteDescription(const std::string& peerId, const OJingleContentAv& av) override;

    bool CreateOffer(const std::string& peerId, const std::string& sId, bool video) override;

    void CreateAnswer(const std::string& peerId, const OJingleContentAv& av) override;

    void setTransportInfo(const std::string& peerId,
                          const std::string& sId,
                          const ortc::OIceUdp& iceUdp) override;

    void SessionTerminate(const std::string& peerId) override;

    void setMute(bool mute) override;

    void setRemoteMute(bool mute) override;

    void addSource(const std::string& peerId,
                   const std::map<std::string, ortc::OMeetSSRCBundle>& map) override;

    size_t getVideoSize() override;

    std::shared_ptr<VideoCaptureInterface> createVideoCapture(const std::string& deviceId);

    bool quit(const std::string& peerId) override;

    void setIceOptions(std::list<IceServer>& ices) override;

    webrtc::SdpType convertToSdpType(JingleSdpType sdpType);

    JingleSdpType convertFromSdpType(webrtc::SdpType sdpType);

    std::unique_ptr<webrtc::SessionDescriptionInterface> convertToSdp(const OJingleContentAv& av);

    std::map<std::string, OIceUdp> getCandidates(const std::string& peerId) override;

    void getLocalSdp(const std::string& peerId, ortc::OJingleContentAv& oContext) override;

    std::vector<OkRTCHandler*> getHandlers();

    void addRTCHandler(OkRTCHandler* hand) override;
    void removeRTCHandler(OkRTCHandler* hand) override;

    const webrtc::PeerConnectionInterface::RTCConfiguration& getConfig() const {
        return _rtcConfig;
    }

    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> getFactory() {
        return peer_connection_factory;
    }

private:
    //    std::unique_ptr<LogSinkImpl> _logSink;
    void addIceServer(const IceServer& ice);

    Conductor* createConductor(const std::string& peerId, const std::string& sId, bool video);

    Conductor* getConductor(const std::string& peerId);

    std::recursive_mutex mutex;

    webrtc::PeerConnectionInterface::RTCConfiguration _rtcConfig;

    std::unique_ptr<rtc::Thread> network_thread;
    std::unique_ptr<rtc::Thread> worker_thread;
    std::unique_ptr<rtc::Thread> signaling_thread;

    std::map<std::string, Conductor*> _pcMap;

    std::vector<OkRTCHandler*> _handlers;

    // 音频源
    rtc::scoped_refptr<webrtc::AudioSourceInterface> audioSource;

    // 视频源
    std::shared_ptr<VideoCaptureInterface> videoCapture;

    // sink
    std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink;

    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory;
};
}  // namespace lib::ortc
