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
// Created by gaojie on 24-5-29.
//

#ifndef OKMSG_PROJECT_IMCALL_H
#define OKMSG_PROJECT_IMCALL_H

#include "IMFriend.h"
#include "IMJingle.h"
#include "tox/toxav.h"

namespace ok::session {
class AuthSession;
}

namespace lib::ortc {
class OkRTCManager;
}

namespace lib::messenger {

class IM;
class IMJingle;
enum class CallDirection;

struct IMCall0 {
    QString id;
    QString sId;
    CallDirection direction;
};

class IMCall : public IMJingle, public lib::ortc::OkRTCHandler {
    Q_OBJECT
public:
    IMCall(IM* im, QObject* parent = nullptr);

    void toPlugins(const ortc::OJingleContentAv& oContext, PluginList& plugins);
    void parse(const PluginList& plugins, ortc::OJingleContentAv& oContext);

    void onCreatePeerConnection(const std::string& sId, const std::string& peerId,
                                bool ok) override;

    // onRTP
    void onRTP(const std::string& sId,       //
               const std::string& friendId,  //
               const lib::ortc::OJingleContentAv& oContext) override;

    // onIce
    void onIce(const std::string& sId,       //
               const std::string& friendId,  //
               const lib::ortc::OIceUdp&) override;

    // Renderer
    void onRender(const std::string& peerId, lib::ortc::RendererImage image) override;

    void addCallHandler(CallHandler*);
    bool callToGroup(const QString& g);

    // 发起呼叫邀请
    bool callToFriend(const QString& f, const QString& sId, bool video);
    // 创建呼叫
    bool callToPeerId(const IMPeerId& to, const QString& sId, bool video);
    // 应答呼叫
    bool callAnswerToFriend(const IMPeerId& peer, const QString& callId, bool video);
    // 取消呼叫
    void callRetract(const IMContactId& f, const QString& sId);
    // 拒绝呼叫
    void callReject(const IMPeerId& f, const QString& sId);

    // 静音功能
    void setMute(bool mute);
    void setRemoteMute(bool mute);

    /**
     * jingle-message
     */
    // 处理JingleMessage消息
    void doJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm);

    // 发起呼叫邀请
    void proposeJingleMessage(const QString& friendId, const QString& callId, bool video);
    // 接受
    void acceptJingleMessage(const IMPeerId& peerId, const QString& callId, bool video);
    // 拒绝
    void rejectJingleMessage(const QString& friendId, const QString& callId);
    // 撤回
    void retractJingleMessage(const QString& friendId, const QString& callId);

protected:
    void handleJingleMessage(const IMPeerId& peerId, const Jingle::JingleMessage* jm) override;

    virtual void doSessionInfo(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentAdd(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentRemove(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentModify(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentAccept(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentReject(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doTransportInfo(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doTransportAccept(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doTransportReject(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doTransportReplace(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doSecurityInfo(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doDescriptionInfo(const Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doInvalidAction(const Jingle::Session::Jingle*, const IMPeerId&) override;

    IMJingleSession* cacheSessionInfo(const IMContactId& from,
                                      const IMPeerId& to,
                                      const QString& sId,
                                      lib::ortc::JingleCallType callType);

    void clearSessionInfo(Jingle::Session* session);

    IMJingleSession* createSession(const IMContactId& from,
                                   const IMPeerId& to,
                                   const QString& sId,
                                   lib::ortc::JingleCallType ct);

    IMJingleSession* findSession(const QString& sId) { return m_sessionMap.value(sId); }

    void sessionOnAccept(const QString& sId,
                         Jingle::Session* session,
                         const IMPeerId& peerId,
                         const Jingle::Session::Jingle* jingle) override;
    void sessionOnTerminate(const QString& sId, const IMPeerId& peerId) override;
    void sessionOnInitiate(const QString& sId,
                           Jingle::Session* session,
                           const Jingle::Session::Jingle* jingle,
                           const IMPeerId& peerId) override;

signals:
    void sig_createPeerConnection(const QString sId, const QString peerId, bool ok);

    void receiveSelfVideoFrame(uint16_t w, uint16_t h,  //
                               const uint8_t* y,        //
                               const uint8_t* u,        //
                               const uint8_t* v,        //
                               int32_t ystride,         //
                               int32_t ustride,         //
                               int32_t vstride);

    void receiveFriendVideoFrame(const QString& friendId,  //
                                 uint16_t w, uint16_t h,   //
                                 const uint8_t* y,         //
                                 const uint8_t* u,         //
                                 const uint8_t* v,         //
                                 int32_t ystride,          //
                                 int32_t ustride,          //
                                 int32_t vstride);

private:
    void connectJingle(IMJingle* jingle);

    /**
     * 发起呼叫
     * @param friendId
     * @param video
     * @return
     */
    bool startCall(const QString& friendId, const QString& sId, bool video);

    bool sendCallToResource(const QString& friendId, const QString& sId, bool video);

    bool createCall(const IMPeerId& to, const QString& sId, bool video);

    bool answer(const IMPeerId& to, const QString& callId, bool video);

    void cancel(const QString& friendId);
    // 取消呼叫
    void cancelCall(const IMContactId& friendId, const QString& sId);
    void rejectCall(const IMPeerId& friendId, const QString& sId);

    void join(const JID& room);

    IM* im;
    IMJingle* jingle;
    //    ok::session::AuthSession* session;
    std::vector<CallHandler*> callHandlers;

    // sid -> session
    QMap<QString, IMJingleSession*> m_sessionMap;

    // sid -> isVideo,在jingle-message阶段暂时保留呼叫的类型是视频（音频无需保存）。
    QMap<QString, bool> m_sidVideo;

public slots:
    void onCallAccepted(IMPeerId peerId, QString callId, bool video);
};

}  // namespace lib::messenger

#endif
