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

//
// Created by gaojie on 24-11-23.
//

#pragma once

#include <meethandler.h>
#include <meetmanager.h>

#include "IM.h"
#include "IMFromHostHandler.h"
#include "IMJingle.h"


namespace lib::messenger {

class IM;

class IMMeet : public IMJingle,
               public IMHandler,
               public SelfHandler,
               public IMSessionHandler,
               public IMFromHostHandler,
               public ortc::OkRTCHandler,
               public gloox::MeetHandler {
public:
    explicit IMMeet(IM* im);
    ~IMMeet() override;

    /**
     * 创建会议
     * @param name
     * @return
     */
    const std::string& create(const std::string& name,
                             const lib::ortc::DeviceConfig& conf);

    /**
     * 解散会议
     */
    void disband();

    /**
     * 退出会议
     */
    void leave();

    /**
     * 加入会议
     */
    void join();

    /**
     * 发送消息
     * @param msg
     */
    void sendMessage(const std::string& msg);

    /**
     * 添加处理器
     * @param hdr
     */
    void addMeetHandler(MessengerMeetHandler* hdr);

    /**
     * 移除处理器
     * @param hdr
     */
    void removeMeetHandler(MessengerMeetHandler* hdr);

    /**
     * 选择视频设备
     * @param deviceId 设备id
     */
    void switchVideoDevice(const std::string& deviceId);

    /**
     * 选择视频设备
     * @param selected 设备索引 0~N
     */
    void switchVideoDevice(int selected);

    /**
     * 获取视频设备列表
     * @return
     */
    std::vector<std::string> getVideoDeviceList();

    void setEnable(ortc::CtrlState state);

    void doStartRTC(const IMPeerId& peerId, const ortc::OJingleContentMap& cav) const;

protected:
    void handleHostPresence(const gloox::JID& from, const gloox::Presence& presence) override;

    void handleHostMessage(const gloox::JID& from, const gloox::Message& msg) override;
    void handleHostMessageSession(const gloox::JID& from, const std::string& sid) override;

    void handleCreation(const gloox::JID& jid, bool ready,
                        const std::map<std::string, std::string>& props) override;

    void handleParticipant(const gloox::JID& jid,
                           const gloox::Meet::Participant& participant) override;

    void handleStatsId(const gloox::JID& jid, const std::string& statsId) override;

    void handleJsonMessage(const gloox::JID& jid, const gloox::JsonMessage* json) override;

    /**
     * IMHandler
     */
    void onConnecting()override;
    void onConnected()override;
    void onDisconnected(int)override;
    void onStarted()override;
    void onStopped()override;

    /**
     * SelfHandler
     */
    void onSelfIdChanged(const std::string& id) override;
    void onSelfNameChanged(const std::string& name) override;
    void onSelfAvatarChanged(const std::string& avatar) override;
    void onSelfStatusChanged(IMStatus status, const std::string& msg) override ;
    void onSelfVCardChanged(IMVCard& imvCard) override;

    /**
     * IMSessionHandler
     */
    bool doSessionInitiate(gloox::Jingle::Session* session,        //
                           const gloox::Jingle::Session::Jingle*,  //
                           const IMPeerId&) override;

    bool doSessionTerminate(gloox::Jingle::Session* session,        //
                            const gloox::Jingle::Session::Jingle*,  //
                            const IMPeerId&) override;

    bool doSessionAccept(gloox::Jingle::Session* session,               //
                         const gloox::Jingle::Session::Jingle* jingle,  //
                         const IMPeerId& peerId) override;

    bool doSessionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doContentAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doContentRemove(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doContentModify(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doContentAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doContentReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doTransportInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doTransportAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doTransportReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doTransportReplace(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doSecurityInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doDescriptionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doSourceAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    bool doInvalidAction(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    // IMJingle
    void handleJingleMessage(const IMPeerId& peerId,
                             const gloox::Jingle::JingleMessage* jm) override;

    void clearSessionInfo(const std::string& sId) override;

    // OkRTCHandler
    void onCreatePeerConnection(const std::string& sId,
                                const std::string& peerId,
                                bool ok) override;

    void onLocalDescriptionSet(const std::string& sId,
                               const std::string& peerId,
                               const ortc::OJingleContentMap* osd) override;

    void onFailure(const std::string& sId,
                   const std::string& peerId,
                   const std::string& error) override;

    void onIceGatheringChange(const std::string& sId,
                              const std::string& peerId,
                              ortc::IceGatheringState state) override;

    void onIceConnectionChange(const std::string& sId,
                               const std::string& peerId,
                               ortc::IceConnectionState state) override;

    void onPeerConnectionChange(const std::string& sId,
                                const std::string& peerId,
                                ortc::PeerConnectionState state) override;

    void onSignalingChange(const std::string& sId,
                           const std::string& peerId,
                           ortc::SignalingState state) override;

    void onIce(const std::string& sId,
               const std::string& peerId,
               const ortc::OIceUdp& iceUdp) override;

    void onRender(const ortc::RendererImage& image,
                  const std::string& peerId,
                  const std::string& resource) override;

    void ToMeetSdp(const ortc::OJingleContentMap* av, gloox::Jingle::PluginList& plugins);

private:
    void doForIceCompleted(const std::string& sId, const std::string& peerId);

    gloox::Meet* meet;
    gloox::MeetManager* meetManager;

    std::vector<MessengerMeetHandler*> handlers;
    IMVCard vCard;
    std::string resource;
    lib::ortc::DeviceConfig conf;

    // signals:
    // ice

    // public slots:
    void onSelfVCard(const IMVCard& vCard);
    // Participant toParticipant(const gloox::Meet::Participant& participant) ;
};

}  // namespace lib::messenger
