﻿/*
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

#ifndef IMJINGLE_H
#define IMJINGLE_H

#include <QMap>
#include <list>
#include <map>

#include <client.h>
#include <jinglesession.h>
#include <jinglesessionhandler.h>
#include <jinglesessionmanager.h>

#include <error.h>
#include <inbandbytestream.h>
#include <messagesessionhandler.h>
#include <presencehandler.h>

#include "IMConference.h"
#include "IMJingleSession.h"
#include "lib/messenger/messenger.h"
#include "lib/session/AuthSession.h"

#include "lib/ortc/ok_rtc.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "lib/ortc/ok_rtc_manager.h"

namespace gloox {
namespace Jingle {
class JingleMessage;
}
}  // namespace gloox

namespace lib::messenger {

enum class CallDirection;

class IMJingle : public QObject,
                 public gloox::MessageHandler,
                 public gloox::IqHandler,
                 public gloox::MessageSessionHandler,
                 public gloox::Jingle::SessionHandler {
    Q_OBJECT

public:
    static IMJingle* getInstance();

    IMJingle(IM* im, QObject* parent = nullptr);
    ~IMJingle() override;

    virtual void handleMessageSession(gloox::MessageSession* session) override;
    virtual void handleMessage(const gloox::Message& msg, gloox::MessageSession* session = 0) override;

    /**
     * jingle-message
     */
    // 处理JingleMessage消息
    void doJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm);

    // 发起呼叫邀请
    void proposeJingleMessage(const QString& friendId, const QString& callId, bool video);

    void rejectJingleMessage(const QString& friendId, const QString& callId);

    void acceptJingleMessage(const IMPeerId& peerId, const QString& callId, bool video);

    void retractJingleMessage(const QString& friendId, const QString& callId);

protected:
    bool handleIq(const IQ& iq) override;

    void handleIqID(const IQ& iq, int context) override;

    void handleSessionAction(Jingle::Action action, Jingle::Session* session,
                             const Jingle::Session::Jingle* jingle) override;

    void handleSessionActionError(Jingle::Action action, Jingle::Session* session,
                                  const gloox::Error* error) override;

    void handleIncomingSession(Jingle::Session* session) override;

    IMJingleSession* findSession(const QString& sId);

    IMJingleSession* createSession(const IMPeerId& to, const QString& sId,
                                   lib::ortc::JingleCallType ct);

    // receiver -> sid
    QMap<IMPeerId, QString> m_friendSessionMap;

    // sid -> session
    QMap<QString, IMJingleSession*> m_sessionMap;

    void clearSessionInfo(Jingle::Session* session);

    std::unique_ptr<Jingle::SessionManager> _sessionManager;

    IMJingleSession* cacheSessionInfo(Jingle::Session* session, lib::ortc::JingleCallType callType);

private:
    QString getSessionByFriendId(const QString& friendId);

    void doSessionInitiate(Jingle::Session* session, const Jingle::Session::Jingle*,
                           const IMPeerId&);

    void doSessionTerminate(Jingle::Session* session, const Jingle::Session::Jingle*,
                            const IMPeerId&);

    void doSessionAccept(Jingle::Session* session,        //
                         const Jingle::Session::Jingle*,  //
                         const IMPeerId&);
    void doSessionInfo(const Jingle::Session::Jingle*, const IMPeerId&);
    void doContentAdd(const Jingle::Session::Jingle*, const IMPeerId&);
    void doContentRemove(const Jingle::Session::Jingle*, const IMPeerId&);
    void doContentModify(const Jingle::Session::Jingle*, const IMPeerId&);
    void doContentAccept(const Jingle::Session::Jingle*, const IMPeerId&);
    void doContentReject(const Jingle::Session::Jingle*, const IMPeerId&);
    void doTransportAccept(const Jingle::Session::Jingle*, const IMPeerId&);
    void doTransportInfo(const Jingle::Session::Jingle*, const IMPeerId&);
    void doTransportReject(const Jingle::Session::Jingle*, const IMPeerId&);
    void doTransportReplace(const Jingle::Session::Jingle*, const IMPeerId&);
    void doSecurityInfo(const Jingle::Session::Jingle*, const IMPeerId&);
    void doDescriptionInfo(const Jingle::Session::Jingle*, const IMPeerId&);
    void doInvalidAction(const Jingle::Session::Jingle*, const IMPeerId&);

    IM* _im;

    //  std::vector<FileHandler *> *fileHandlers;

    // sid -> isVideo,在jingle-message阶段暂时保留呼叫的类型是视频（音频无需保存）。
    QMap<QString, bool> m_sidVideo;

    QList<Jingle::Content*> m_ices;

signals:
    void callStarted();

    // 呼叫请求
    void receiveCallRequest(IMPeerId peerId, QString callId, bool audio, bool video);

    void receiveFriendCall(QString friendId, QString callId, bool audio, bool video);

    // 呼叫撤回
    void receiveCallRetract(QString friendId, int state);
    void receiveCallAcceptByOther(QString callId, IMPeerId peerId);
    void receiveFriendHangup(QString friendId, int state);

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
};

}  // namespace lib::messenger
#endif  // IMJINGLE_H
