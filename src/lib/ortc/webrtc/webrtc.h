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
#include <optional>
#include <string>

#include <api/peer_connection_interface.h>
#include <modules/video_capture/video_capture.h>
#include <pc/session_description.h>
#include <rtc_base/thread.h>

#include "../ok_rtc.h"
#include "VideoCaptureInterface.h"

namespace webrtc {
class PeerConnectionFactoryInterface;
class SessionDescriptionInterface;
class AudioSourceInterface;
class VideoEncoderFactory;
class VideoDecoderFactory;
// class VideoCaptureModule;

}  // namespace webrtc

namespace lib::ortc {

class Conductor;

std::unique_ptr<cricket::AudioContentDescription> createAudioDescription(
        const ORTP& rtp,  //
        const std::map<std::string, OMeetSSRCBundle>&);

std::unique_ptr<cricket::VideoContentDescription> createVideoDescription(
        const ORTP& rtp,  //
        const std::map<std::string, OMeetSSRCBundle>& ssrcBundleMap);
std::unique_ptr<cricket::SctpDataContentDescription> createDataDescription(const OSdp& sdp);

class WebRTCObserver {
public:
    virtual void onRemoteDescriptionSet(const webrtc::SessionDescriptionInterface* sdp,
                                        const std::string& sId, const std::string& peerId) = 0;
    virtual void onLocalDescriptionSet(const webrtc::SessionDescriptionInterface* sdp,
                                       const std::string& sId, const std::string& peerId) = 0;
};

class WebRTC : public OkRTC, public WebRTCObserver {
public:
    explicit WebRTC(std::string res);

    ~WebRTC() override;

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

    std::shared_ptr<VideoCaptureInterface> getVideoCapture(const std::string& deviceId);
    void destroyVideoCapture();

    bool quit(const std::string& peerId) override;

    void setIceOptions(std::vector<IceServer>& ices) override;

    webrtc::SdpType convertToSdpType(JingleSdpType sdpType);

    std::map<std::string, OIceUdp> getCandidates(const std::string& peerId) override;

    std::unique_ptr<OJingleContentAv> getLocalSdp(const std::string& peerId) override;

    const std::vector<OkRTCHandler*>& getHandlers() override;

    void addRTCHandler(OkRTCHandler* hand) override;
    void removeRTCHandler(OkRTCHandler* hand) override;

    void switchVideoDevice(const std::string& deviceId) override;

    void switchVideoDevice(int selected) override;

    const webrtc::PeerConnectionInterface::RTCConfiguration& getConfig() const {
        return _rtcConfig;
    }

    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> getFactory() {
        return peer_connection_factory;
    }

    std::string getVideoDeviceId(int selected);

    std::vector<std::string> getVideoDeviceList() override;

protected:
    void onRemoteDescriptionSet(const webrtc::SessionDescriptionInterface* sdp,
                                const std::string& sId, const std::string& peerId) override;

    void onLocalDescriptionSet(const webrtc::SessionDescriptionInterface* sdp,
                               const std::string& sId, const std::string& peerId) override;

private:
    void addIceServer(const IceServer& ice);

    Conductor* createConductor(const std::string& peerId, const std::string& sId, bool video);

    Conductor* getConductor(const std::string& peerId);

    // ============转换=============
    /**
     * 转换Sdp
     * @param av
     * @return webrtc::SessionDescriptionInterface
     */
    std::unique_ptr<webrtc::SessionDescriptionInterface> convertSdpToDown(
            const OJingleContentAv& av);

    std::unique_ptr<OJingleContentAv> convertSdpToUp(
            const webrtc::SessionDescriptionInterface* sdp);

    /**
     * 转换成 webrtc::SdpType
     * @param type
     * @return
     */
    webrtc::SdpType convertSdpTypeDown(JingleSdpType type);

    /**
     * 转换成 JingleSdpType
     * @param type
     * @return
     */
    JingleSdpType convertSdpTypeUp(webrtc::SdpType type);

    /**
     * 转成cricket::Candidate
     * @param item
     * @param iceUdp
     * @return
     */
    cricket::Candidate convertCandidateToDown(const Candidate& item, const OIceUdp& iceUdp);

    /**
     * 转成Candidate
     * @param cand
     * @return
     */
    Candidate convertCandidateToUp(const cricket::Candidate& cand);

    /**
     * 转成 cricket::TransportInfo
     * @param name
     * @param iceUdp
     * @return
     */
    cricket::TransportInfo convertTransportToDown(const std::string& name, const OIceUdp& iceUdp);

    /**
     * 从Sdp获取Ice
     * @param sdp
     * @return
     */
    std::map<std::string, ortc::OIceUdp> getIceFromDown(
            const webrtc::SessionDescriptionInterface* sdp);

    Dtls getDtls(const cricket::TransportInfo& info);

    Dtls getDtls(const webrtc::SessionDescriptionInterface* sdp, const std::string& mid);

    // ============转换=============

    void initAudioDevice();

    void linkAudioDevice(Conductor* c);

    void linkVideoDevice(Conductor* c, int selected);

    std::recursive_mutex mutex;

    // 资源ID
    std::string resource;

    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> deviceInfo = nullptr;

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
    std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> videoSink;

    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory;
};
}  // namespace lib::ortc
