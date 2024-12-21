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
#include <QObject>
#include <memory>

#include "IM.h"
#include "IMFromHostHandler.h"
#include "IMJingle.h"
#include "base/jid.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "messenger.h"

namespace lib::messenger {

class IM;

class IMMeet : public IMJingle,
               public IMSessionHandler,
               public IMFromHostHandler,
               public ortc::OkRTCHandler,
               public gloox::MeetHandler {
    Q_OBJECT
public:
    explicit IMMeet(IM* im, QObject* parent = nullptr);
    ~IMMeet() override;

    /**
     * 创建会议
     * @param name
     * @return
     */
    const std::string& create(const QString& name);

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
    void switchVideoDevice(const QString& deviceId);

    /**
     * 选择视频设备
     * @param selected 设备索引
     */
    void switchVideoDevice(int selected);

protected:
    void handleHostPresence(const gloox::JID& from, const gloox::Presence& presence) override;

    void handleCreation(const gloox::JID& jid, bool ready,
                        const std::map<std::string, std::string>& props) override;

    void handleParticipant(const gloox::JID& jid,
                           const gloox::Meet::Participant& participant) override;

    void handleStatsId(const gloox::JID& jid, const std::string& statsId) override;

    void handleJsonMessage(const gloox::JID& jid, const gloox::JsonMessage* json) override;

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

    void clearSessionInfo(const QString& sId) override;

    // OkRTCHandler
    void onCreatePeerConnection(const std::string& sId,
                                const std::string& peerId,
                                bool ok) override;

    void onRTP(const std::string& sId,
               const std::string& peerId,
               const ortc::OJingleContentAv& osd) override;

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

private:
    void doForIceCompleted(const QString& sId, const QString& peerId);

    gloox::Meet* meet;
    gloox::MeetManager* manager;

    std::vector<MessengerMeetHandler*> handlers;
    IMVCard vCard;

signals:
    // ice
    void iceGatheringStateChanged(IMPeerId to, const QString sId, ortc::IceGatheringState);
    void iceConnectionStateChanged(IMPeerId to, const QString sId, ortc::IceConnectionState);

public slots:
    void onSelfVCard(const IMVCard& vCard);
    Participant toParticipant(const gloox::Meet::Participant& participant) const;
    void doStartRTC(const IMPeerId& peerId, const ortc::OJingleContentAv& cav) const;
};

}  // namespace lib::messenger
