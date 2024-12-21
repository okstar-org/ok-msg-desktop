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

#include <sstream>
#include <string>

#include "ok_rtc_defs.h"
#include "ok_rtc_renderer.h"

namespace lib::ortc {

struct IceServer {
    std::string uri;
    std::string username;
    std::string password;

    [[nodiscard]] std::string toString() const {
        std::stringstream ss;
        ss << "{uri:" << uri << ",  username:" << username << ", password:" << password << "}";
        return ss.str();
    }
};

typedef struct join_options {
    std::string conference;
    std::string conference_id;
    std::string peer_name;
    long peer_id;
} JoinOptions;

class OkRTCHandler {
public:
    virtual void onCreatePeerConnection(const std::string& sId,
                                        const std::string& peerId,
                                        bool ok) = 0;

    virtual void onRTP(const std::string& sId,
                       const std::string& peerId,
                       const OJingleContentAv& osd) = 0;

    virtual void onFailure(const std::string& sId,
                           const std::string& peerId,
                           const std::string& error) = 0;

    virtual void onIceGatheringChange(const std::string& sId,
                                      const std::string& peerId,
                                      IceGatheringState state) = 0;

    virtual void onIceConnectionChange(const std::string& sId,
                                       const std::string& peerId,
                                       IceConnectionState state) = 0;

    virtual void onPeerConnectionChange(const std::string& sId,
                                        const std::string& peerId,
                                        PeerConnectionState state) = 0;

    virtual void onSignalingChange(const std::string& sId,
                                   const std::string& peerId,
                                   SignalingState state) = 0;

    virtual void onIce(const std::string& sId,
                       const std::string& peerId,
                       const OIceUdp& iceUdp) = 0;

    virtual void onRender(const RendererImage& image,
                          const std::string& peerId,
                          const std::string& resource) = 0;
};

/**
 * RTC 接口
 */
class OkRTC {
public:
    virtual ~OkRTC() = default;

    // 启动rtc实例
    virtual bool start() = 0;
    // 停止rtc实例
    virtual bool stop() = 0;

    virtual bool isStarted() = 0;

    virtual bool ensureStart() = 0;

    virtual void setIceOptions(std::list<IceServer>& ices) = 0;

    virtual void addRTCHandler(OkRTCHandler* hand) = 0;
    virtual void removeRTCHandler(OkRTCHandler* hand) = 0;
    virtual const std::vector<OkRTCHandler*>& getHandlers() = 0;

    virtual bool CreateOffer(const std::string& peerId, const std::string& sId, bool video) = 0;

    virtual void CreateAnswer(const std::string& peerId, const OJingleContentAv& av) = 0;

    virtual void setRemoteDescription(const std::string& peerId, const OJingleContentAv& av) = 0;

    virtual void SessionTerminate(const std::string& peerId) = 0;

    virtual void setTransportInfo(const std::string& peerId,
                                  const std::string& sId,
                                  const OIceUdp& oIceUdp) = 0;

    virtual void setMute(bool mute) = 0;

    virtual void setRemoteMute(bool mute) = 0;

    virtual bool quit(const std::string& peerId) = 0;

    // 获取视频设备数量
    virtual size_t getVideoSize() = 0;

    virtual std::map<std::string, OIceUdp> getCandidates(const std::string& peerId) = 0;

    virtual void getLocalSdp(const std::string& peerId, ortc::OJingleContentAv& oContext) = 0;

    virtual void addSource(const std::string& peerId,
                           const std::map<std::string, ortc::OMeetSSRCBundle>& map) = 0;

    virtual void switchVideoDevice(const std::string& deviceId) = 0;

    virtual void switchVideoDevice(int selected) = 0;
};

}  // namespace lib::ortc
