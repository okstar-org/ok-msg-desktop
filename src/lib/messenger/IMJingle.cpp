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

#include "IMJingle.h"

#include <QUuid>

#include <capabilities.h>
#include <extdisco.h>
#include <jinglesession.h>

#include "IM.h"
#include "base/logs.h"

namespace lib {
namespace messenger {

using namespace gloox;
using namespace Jingle;
using namespace lib::ortc;

IMJingle::IMJingle(IM* im, QObject* parent) : QObject(parent), _im(im) {
    qDebug() << __func__ << "Creating";

    qRegisterMetaType<std::string>("std::string");
    connect(im, &IM::started, this, &IMJingle::onImStarted);
    qDebug() << __func__ << ("Created");
}

IMJingle::~IMJingle() {
    auto client = _im->getClient();
    client->removeMessageHandler(this);
    qDebug() << __func__ << "Destroyed";
}

void IMJingle::onImStarted() {
    auto client = _im->getClient();
    assert(client);

    client->registerMessageHandler(this);
    client->registerStanzaExtension(new Jingle::JingleMessage());

    auto disco = client->disco();
    // jingle
    disco->addFeature(XMLNS_JINGLE);
    disco->addFeature(XMLNS_JINGLE_MESSAGE);
    disco->addFeature(XMLNS_JINGLE_ERRORS);

}

void IMJingle::handleMessageSession(MessageSession* session) {
    //  session->registerMessageHandler(this);
}

void IMJingle::handleMessage(const Message& msg, MessageSession* session) {
    qDebug() << __func__ << "...";

    /**
     * 处理jingle-message消息
     * https://xmpp.org/extensions/xep-0353.html
     */
    auto jm = msg.findExtension<Jingle::JingleMessage>(ExtJingleMessage);
    if (jm) {
        handleJingleMessage(IMPeerId(msg.from().full()), jm);
    }
}



QString IMJingle::getSessionByFriendId(const QString& friendId) {
    qDebug() << ("getSessionId:%1") << ((friendId));

    return m_friendSessionMap.value(IMPeerId{friendId}, {});
}
/**
 * Jingle sessions
 */

void IMJingle::handleSessionActionError(Action action, Session* session,
                                        const gloox::Error* error) {
    qDebug() << __func__ << "sid:" << qstring(session->sid())
             << "action:" << static_cast<int>(action)
             << "remote:" << qstring(session->remote().full())
             << "error:" << qstring(error->text());
}

void IMJingle::handleIncomingSession(Session* session) {
    auto sid = qstring(session->sid());
    qDebug() << __func__ << "sId" << sid;
}

// Session
void IMJingle::handleSessionAction(Action action, Session* session, const Session::Jingle* jingle) {
    auto from = session->remote();
    auto friendId = IMPeerId(from);
    auto sid = qstring(jingle->sid());

    qDebug() << __func__ << static_cast<int>(action) << qstring(from.full()) << sid;

    switch (action) {
        case Action::SessionInitiate: {
            doSessionInitiate(session, jingle, friendId);
            break;
        }
        case Action::SessionInfo: {
            doSessionInfo(jingle, friendId);
            break;
        }
        case Action::SessionTerminate: {
            doSessionTerminate(session, jingle, friendId);
            break;
        }
        case Action::SessionAccept: {
            doSessionAccept(session, jingle, friendId);
            break;
        }
        case Action::ContentAccept: {
            doContentAccept(jingle, friendId);
            break;
        }
        case Action::ContentAdd: {
            // source-add|content-add
            doContentAdd(jingle, friendId);
            break;
        }
        case Action::ContentRemove: {
            doContentRemove(jingle, friendId);
            break;
        }
        case Action::ContentModify: {
            doContentModify(jingle, friendId);
            break;
        }
        case Action::ContentReject: {
            doContentReject(jingle, friendId);
            break;
        }
        case Action::TransportAccept: {
            doTransportAccept(jingle, friendId);
            break;
        }
        case Action::TransportInfo: {
            doTransportInfo(jingle, friendId);
            break;
        }
        case Action::TransportReject: {
            doTransportReject(jingle, friendId);
            break;
        }
        case Action::TransportReplace: {
            doTransportReplace(jingle, friendId);
            break;
        }
        case Action::SecurityInfo: {
            doSecurityInfo(jingle, friendId);
            break;
        }
        case Action::DescriptionInfo: {
            doDescriptionInfo(jingle, friendId);
            break;
        }
        case Action::InvalidAction:
            doInvalidAction(jingle, friendId);
            break;
    }
}

void IMJingle::doSessionInitiate(Jingle::Session* session,
                                 const Jingle::Session::Jingle* jingle,
                                 const IMPeerId& peerId) {
    auto sid = qstring(session->sid());
    qDebug() << __func__ << "sId:" << sid << "peerId:" << peerId.toString();
    sessionOnInitiate(sid, session, jingle, peerId);
}

void IMJingle::doSessionTerminate(Jingle::Session* session,
                                  const Session::Jingle* jingle,
                                  const IMPeerId& peerId) {
    auto sid = qstring(jingle->sid());
    qDebug() << __func__ << "sId:" << sid << "peerId" << peerId.toString();
    sessionOnTerminate(sid, peerId);
}

void IMJingle::doSessionAccept(Jingle::Session* session,
                               const Jingle::Session::Jingle* jingle,
                               const IMPeerId& peerId) {
    auto sid = qstring(jingle->sid());
    qDebug() << __func__ << sid << "peerId:" << peerId.toString();
    sessionOnAccept(sid, session, peerId, jingle);
}

void IMJingle::doContentRemove(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("jingle:%1 peerId:%2") << (qstring(jingle->sid())) << ((peerId.toString()));
}

void IMJingle::doContentModify(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("jingle:%1 peerId:%2")  //
             << ((qstring(jingle->sid()))) << ((peerId.toString()));
}

void IMJingle::doContentAccept(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("jingle:%1 peerId:%2")  //
             << ((qstring(jingle->sid()))) << ((peerId.toString()));
}

void IMJingle::doContentReject(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("jingle:%1 peerId:%2")  //
             << ((qstring(jingle->sid()))) << ((peerId.toString()));
}

void IMJingle::doTransportAccept(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("jingle:%1 peerId:%2")  //
             << qstring(jingle->sid()) << peerId.toString();
}


void IMJingle::doDescriptionInfo(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("sessionId:%1 from:%2") << (qstring(jingle->sid())) << ((peerId.toString()));
}

void IMJingle::doInvalidAction(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("sessionId:%1 from:%2") << (qstring(jingle->sid())) << ((peerId.toString()));
}

bool IMJingle::handleIq(const IQ& iq) {
    return true;
}

void IMJingle::handleIqID(const IQ& iq, int context) {}


}  // namespace messenger
}  // namespace lib
