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
#include <jinglemessage.h>
#include <messagesessionhandler.h>
#include <presencehandler.h>

#include "lib/messenger/messenger.h"
#include "lib/session/AuthSession.h"

#include "lib/ortc/ok_rtc.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "lib/ortc/ok_rtc_manager.h"

namespace lib::messenger {

enum class CallDirection;

class IMJingleSession {
public:
    virtual void start() = 0;
    virtual void stop() = 0;
};

class IMJingle : public QObject,
                 public gloox::MessageHandler,
                 public gloox::IqHandler,
                 public gloox::MessageSessionHandler,
                 public gloox::Jingle::SessionHandler {
    Q_OBJECT

public:
    explicit IMJingle(IM* im, QObject* parent = nullptr);
    ~IMJingle() override;

    IM* getIM() { return _im; }

    virtual void handleMessageSession(gloox::MessageSession* session) override;
    virtual void handleMessage(const gloox::Message& msg,
                               gloox::MessageSession* session = 0) override;

protected:
    virtual void handleJingleMessage(const IMPeerId& peerId,
                                     const gloox::Jingle::JingleMessage* jm) = 0;

    bool handleIq(const gloox::IQ& iq) override;

    void handleIqID(const gloox::IQ& iq, int context) override;

    void handleSessionAction(gloox::Jingle::Action action, gloox::Jingle::Session* session,
                             const gloox::Jingle::Session::Jingle* jingle) override;

    void handleSessionActionError(gloox::Jingle::Action action, gloox::Jingle::Session* session,
                                  const gloox::Error* error) override;

    void handleIncomingSession(gloox::Jingle::Session* session) override;

    // receiver -> sid
    QMap<IMPeerId, QString> m_friendSessionMap;

    std::unique_ptr<gloox::Jingle::SessionManager> _sessionManager;

    virtual void sessionOnAccept(const QString& sId,
                                 gloox::Jingle::Session* session,
                                 const IMPeerId& peerId,
                                 const gloox::Jingle::Session::Jingle* jingle) = 0;

    virtual void sessionOnTerminate(const QString& sId, const IMPeerId& peerId) = 0;

    virtual void sessionOnInitiate(const QString& sId,
                                   gloox::Jingle::Session* session,
                                   const gloox::Jingle::Session::Jingle* jingle,
                                   const IMPeerId& peerId) = 0;

    virtual void clearSessionInfo(const QString& sId) = 0;

    IM* _im;

    // 传输文件、传输视频会话的区分
    QList<QString> m_invalid_sId;

    void addInvalidSid(const QString& sid) { m_invalid_sId.append(sid); }

    bool isInvalidSid(const QString& sid) { return m_invalid_sId.contains(sid); }

private:
    QString getSessionByFriendId(const QString& friendId);

    void doSessionInitiate(gloox::Jingle::Session* session,        //
                           const gloox::Jingle::Session::Jingle*,  //
                           const IMPeerId&);

    void doSessionTerminate(gloox::Jingle::Session* session,        //
                            const gloox::Jingle::Session::Jingle*,  //
                            const IMPeerId&);

    void doSessionAccept(gloox::Jingle::Session* session,        //
                         const gloox::Jingle::Session::Jingle*,  //
                         const IMPeerId&);

    virtual void doSessionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doContentAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doContentRemove(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doContentModify(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doContentAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doContentReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doTransportInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doTransportAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doTransportReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doTransportReplace(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doSecurityInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doDescriptionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual void doInvalidAction(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;

    QList<gloox::Jingle::Content*> m_ices;

protected slots:
    virtual void onImStarted();
};

}  // namespace lib::messenger
#endif  // IMJINGLE_H
