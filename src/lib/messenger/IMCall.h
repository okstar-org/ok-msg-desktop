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

enum class CallDirection { CallIn, CallOut };

enum CallStage {
    StageNone,
    StageMessage,  // XEP-0353: Jingle Message Initiation
    StageSession   // XEP-0166: Jingle https://xmpp.org/extensions/xep-0166.html
};

class IMCallSession : public QObject, public IMJingleSession {
    Q_OBJECT
public:
    explicit IMCallSession(const QString& sId,
                           gloox::Jingle::Session* session,
                           const IMContactId& selfId,
                           const IMPeerId& peerId,
                           lib::ortc::JingleCallType callType);

    ~IMCallSession() override;

    void start() override;
    void stop() override;

    [[nodiscard]] gloox::Jingle::Session* getSession() const;
    [[nodiscard]] inline const ortc::OJingleContent& getContext() const { return context; }

    void onAccept();
    // 被动结束
    void onTerminate();
    // 主动结束
    void doTerminate();

    void createOffer(const std::string& peerId);

    void setContext(const ortc::OJingleContent&);

    const gloox::Jingle::Session::Jingle* getJingle() const;
    void setJingle(const gloox::Jingle::Session::Jingle* jingle);

    [[nodiscard]] CallDirection direction() const;

    void setCallStage(CallStage state);

    void setAccepted(bool y) { accepted = y; }

    [[nodiscard]] bool isAccepted() const { return accepted; }

    const QString& getId() const { return sId; }

    void appendIce(const ortc::OIceUdp& ice) { pendingIceCandidates.emplace_back(ice); }

    void pollIce(ok::base::Fn<void(const ortc::OIceUdp&)> fn) {
        while (!pendingIceCandidates.empty()) {
            fn(pendingIceCandidates.back());
            pendingIceCandidates.pop_back();
        }
    }

private:
    QString sId;
    gloox::Jingle::Session* session;
    IMContactId selfId;
    const gloox::Jingle::Session::Jingle* jingle;
    ortc::OJingleContent context;

    lib::ortc::JingleCallType m_callType;
    CallStage m_callStage;
    bool accepted;

    std::list<ortc::OIceUdp> pendingIceCandidates;
};

class IMCall : public IMJingle, public lib::ortc::OkRTCHandler {
    Q_OBJECT
public:
    explicit IMCall(IM* im, QObject* parent = nullptr);
    ~IMCall() override;
    void toPlugins(const ortc::OJingleContentAv& oContext, gloox::Jingle::PluginList& plugins);
    void parse(const gloox::Jingle::Session::Jingle* jingle, ortc::OJingleContentAv& oContext);

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
    void handleJingleMessage(const IMPeerId& peerId,
                             const gloox::Jingle::JingleMessage* jm) override;

    virtual void doSessionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentRemove(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentModify(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doContentReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doTransportInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doTransportAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doTransportReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doTransportReplace(const gloox::Jingle::Session::Jingle*,
                                    const IMPeerId&) override;
    virtual void doSecurityInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doDescriptionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;
    virtual void doInvalidAction(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    IMCallSession* cacheSessionInfo(const IMContactId& from,
                                    const IMPeerId& to,
                                    const QString& sId,
                                    lib::ortc::JingleCallType callType);

    void clearSessionInfo(const QString& sId) override;

    IMCallSession* createSession(const IMContactId& from,
                                 const IMPeerId& to,
                                 const QString& sId,
                                 lib::ortc::JingleCallType ct);

    IMCallSession* findSession(const QString& sId) { return m_sessionMap.value(sId); }

    void sessionOnAccept(const QString& sId,
                         gloox::Jingle::Session* session,
                         const IMPeerId& peerId,
                         const gloox::Jingle::Session::Jingle* jingle) override;
    void sessionOnTerminate(const QString& sId, const IMPeerId& peerId) override;
    void sessionOnInitiate(const QString& sId,
                           gloox::Jingle::Session* session,
                           const gloox::Jingle::Session::Jingle* jingle,
                           const IMPeerId& peerId) override;

signals:
    void sig_createPeerConnection(const QString sId, const QString peerId, bool ok);

    // 呼叫请求
    void receiveCallRequest(IMPeerId peerId, QString callId, bool audio, bool video);

    void receiveFriendCall(QString friendId, QString callId, bool audio, bool video);

    // 呼叫撤回
    void receiveCallRetract(QString friendId, CallState state);
    void receiveCallAcceptByOther(QString callId, IMPeerId peerId);
    void receiveFriendHangup(QString friendId, CallState state);

    // 对方状态变化
    void receiveCallStateAccepted(IMPeerId peerId, QString callId, bool video);
    void receiveCallStateRejected(IMPeerId peerId, QString callId, bool video);

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
    void connectCall(IMCall* imCall);

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

    void join(const gloox::JID& room);

    std::vector<CallHandler*> callHandlers;

    // sid -> session
    QMap<QString, IMCallSession*> m_sessionMap;

    // sid -> isVideo,在jingle-message阶段暂时保留呼叫的类型是视频（音频无需保存）。
    QMap<QString, bool> m_sidVideo;

public slots:
    void onCallAccepted(IMPeerId peerId, QString callId, bool video);
    void onImStartedCall();
};

}  // namespace lib::messenger

#endif
