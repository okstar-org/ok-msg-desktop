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
#include <jinglecontent.h>
#include <jinglefiletransfer.h>
#include <jinglegroup.h>
#include <jingleibb.h>
#include <jingleiceudp.h>
#include <jinglertp.h>
#include <jinglesession.h>
#include <jinglesessionmanager.h>

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

    auto client = _im->getClient();
    client->registerMessageHandler(this);
    client->registerStanzaExtension(new Jingle::JingleMessage());

    // jingle session
    _sessionManager = std::make_unique<SessionManager>(client, this);
    _sessionManager->registerPlugin(new Content());

    auto disco = client->disco();
    // jingle
    disco->addFeature(XMLNS_JINGLE);
    disco->addFeature(XMLNS_JINGLE_MESSAGE);
    disco->addFeature(XMLNS_JINGLE_ERRORS);

    // jingle file
    disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER);
    disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER4);
    disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER5);
    disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER_MULTI);
    disco->addFeature(XMLNS_JINGLE_IBB);
    _sessionManager->registerPlugin(new FileTransfer());
    _sessionManager->registerPlugin(new IBB());

    // jingle av
    disco->addFeature(XMLNS_JINGLE_ICE_UDP);
    disco->addFeature(XMLNS_JINGLE_APPS_DTLS);
    disco->addFeature(XMLNS_JINGLE_APPS_RTP);
    disco->addFeature(XMLNS_JINGLE_FEATURE_AUDIO);
    disco->addFeature(XMLNS_JINGLE_FEATURE_VIDEO);
    disco->addFeature(XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(XMLNS_JINGLE_APPS_RTP_FB);
    disco->addFeature(XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(XMLNS_JINGLE_APPS_RTP_HDREXT);
    disco->addFeature(XMLNS_JINGLE_APPS_GROUP);
    _sessionManager->registerPlugin(new ICEUDP());
    _sessionManager->registerPlugin(new Group());
    _sessionManager->registerPlugin(new RTP());

    auto rtcManager = OkRTCManager::getInstance();

    std::list<ExtDisco::Service> discos;

    ExtDisco::Service disco0;
    disco0.type = "turn";
    disco0.host = "chuanshaninfo.com";
    disco0.port = 34780;
    disco0.username = "gaojie";
    disco0.password = "hncs";
    discos.push_back(disco0);

    ExtDisco::Service disco1;
    disco1.type = "stun";
    disco1.host = "stun.l.google.com";
    disco1.port = 19302;

    discos.push_back(disco1);

    for (const auto& item : discos) {
        ortc::IceServer ice;
        ice.uri = item.type + ":" + item.host + ":" + std::to_string(item.port);
        //              "?transport=" + item.transport;
        ice.username = item.username;
        ice.password = item.password;
        qDebug() << "Add ice:" << ice.uri.c_str();
        rtcManager->addIceServer(ice);
    }

    qDebug() << __func__ << ("Created");
}

IMJingle::~IMJingle() {
    auto client = _im->getClient();
    client->removeMessageHandler(this);
    qDebug() << __func__ << "Destroyed";
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
    //
    //    auto ws = findSession(sid);
    //    if (!ws) {
    //        qWarning() << "session is no existing.";
    //        return;
    //    }
    //
    sessionOnTerminate(sid, peerId);

    //    ws->onTerminate();

    //  int ri = 0;
    //  for (auto &file : m_waitSendFiles) {
    //    if (qstring(session->sid()) == file.sId) {
    //      // TODO 需要处理没有terminate的信令的清理
    //      // 清理待发文件
    //      qDebug()<<"session is terminate."<<file.id;
    ////      doStopFileSendTask(session, file);
    //      m_waitSendFiles.removeAt(ri);
    //      return;
    //    }
    //    ri++;
    //  }

    /*
     *<jingle action='session-terminate'
     sid='8cfd5b65c45b16822da6b2448f8debb7afa5e0e300000005'
     xmlns='urn:xmpp:jingle:1'> <reason><busy/></reason>
        </jingle>
        reason busy:正忙 decline：拒绝
     */
    //  auto state = FINISHED;
    //  auto reason = jingle->tag()->findChild("reason");
    //  if (reason) {
    //    if (reason->findChild("busy")) {
    //      state = SENDING_A;
    //    }
    //  }
    // rtc
    //  auto s = findSession(sid);
    //  if (s) {
    //    auto rtcManager = OkRTCManager::getInstance();
    //    if (rtcManager) {
    //      rtcManager->quit(stdstring(peerId.toString()));
    //    }
    //  }

    //    clearSessionInfo(session);
    //  emit receiveFriendHangup(peerId.toFriendId(), (int)state);
}

void IMJingle::doSessionAccept(Jingle::Session* session,
                               const Jingle::Session::Jingle* jingle,
                               const IMPeerId& peerId) {
    auto sid = qstring(jingle->sid());
    qDebug() << __func__ << sid << "peerId:" << peerId.toString();
    sessionOnAccept(sid, session, peerId, jingle);
}

// void IMJingle::doSessionInfo(const Session::Jingle* jingle, const IMPeerId& friendId) {
//     qDebug() << "jingle:%1 peerId:%2"   //
//              << qstring(jingle->sid())  //
//              << friendId.toString();
// }

void IMJingle::doContentRemove(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("jingle:%1 peerId:%2") << (qstring(jingle->sid())) << ((peerId.toString()));

    //  JID peerJID;
    //  std::map<std::string, Session> sdMap;
    //  const PluginList &plugins = jingle->plugins();
    //  for (const auto p : plugins) {
    //    qDebug()<<("Plugin:%1")<<((QString::fromStdString(p->filterString())));
    //    JinglePluginType pt = p->pluginType();
    //    switch (pt) {
    //    case JinglePluginType::PluginContent: {
    //      break;
    //    }
    //    default:
    //      break;
    //    }
    //  }
    //
    //  _rtcManager->ContentRemove(sdMap, this);
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

// IMJingleSession* IMJingle::findSession(const QString& sId) { return m_sessionMap.value(sId); }

}  // namespace messenger
}  // namespace lib
