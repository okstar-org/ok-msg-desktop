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
#include <jinglegroup.h>
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
    client->registerIqHandler(this, ExtIBB);
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
        doJingleMessage(IMPeerId(msg.from().full()), jm);
    }
}

void IMJingle::doJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm) {
    qDebug() << __func__ << "peerId:" << peerId.toString() << "sId:" << qstring(jm->id())
             << "action:" << Jingle::ActionValues[jm->action()];

    auto friendId = peerId.toFriendId();
    qDebug() << "friendId:" << friendId;

    auto sId = qstring(jm->id());

    switch (jm->action()) {
        case Jingle::JingleMessage::reject: {
            /**
             * 对方拒绝
             */
            //      mPeerRequestMedias.clear();
            auto ms = jm->medias();
            emit receiveCallStateRejected(peerId, sId, ms.size() > 1);
            //      emit receiveFriendHangup(friendId, 0);
            break;
        }
        case Jingle::JingleMessage::propose: {
            // 被对方发起呼叫
            qDebug() << "On call from:" << peerId.toString();

            // 获取呼叫类型
            bool audio = false;
            bool video = false;
            for (auto& m : jm->medias()) {
                if (m == Jingle::audio) {
                    audio = true;
                } else if (m == Jingle::video) {
                    video = true;
                }
            }

            emit receiveCallRequest(peerId, sId, audio, video);
            //      emit receiveCallAcceptByOther(sId, peerId);
            break;
        }
        case Jingle::JingleMessage::retract: {
            /**
             * 撤回(需要判断是对方还是自己其它终端)
             */
            emit receiveCallRetract(friendId, 0);
            break;
        }
        case Jingle::JingleMessage::accept: {
            // 自己其它终端接受，挂断自己
            if (peerId != _im->getSelfPeerId()) {
                emit receiveFriendHangup(friendId, 0);
            } else {
                // 自己终端接受，不处理
                //            OkRTCManager::getInstance()->getRtc()->CreateAnswer(peerId.toString());
            }
            break;
        }
        case Jingle::JingleMessage::proceed: {
            // 对方接受
            auto removed = m_sidVideo.remove(sId);  // 确定发起的是否是视频？
            emit receiveCallStateAccepted(peerId, sId, removed == 1);
            break;
        }
        case Jingle::JingleMessage::finish:
            break;
    }
}

IMJingleSession* IMJingle::cacheSessionInfo(Jingle::Session* session,
                                            lib::ortc::JingleCallType callType) {
    auto& responder = session->remote();
    auto sId = qstring(session->sid());
    auto peer = IMPeerId(responder);

    m_friendSessionMap.insert(peer, sId);

    auto ws = new IMJingleSession(_im, peer, sId, callType, session);

    m_sessionMap.insert(sId, ws);

    //  connect(ws,
    //          &IMJingleSession::sendFileInfo,
    //          [&](const JID &m_friendId, const File &m_file, int m_seq,
    //          int m_sentBytes, bool end){
    //
    //  });

    return ws;
}

void IMJingle::clearSessionInfo(Jingle::Session* session) {
    auto sId = qstring(session->sid());

    qDebug() << __func__ << sId;

    auto& responder = session->remote();
    m_friendSessionMap.remove(IMPeerId(responder));
    m_sessionMap.remove(sId);

    _sessionManager->discardSession(session);
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

    auto* ws = findSession(sid);
    if (ws) {
        ws->setJingle(jingle);
    }

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

    sessionOnInitiate(sid, jingle, peerId);

    //    bool isFile = false;
    //    for (auto p : jingle->plugins()) {
    //        auto pt = p->pluginType();
    //        switch (pt) {
    //            case JinglePluginType::PluginContent: {
    //                auto file = p->findPlugin<FileTransfer>(PluginFileTransfer);
    //                auto ibb = p->findPlugin<IBB>(PluginIBB);
    //                if (file && ibb) {
    //                    for (auto f : file->files()) {
    //                        auto id = qstring(ibb->sid());
    //                        auto sId = qstring(session->sid());
    //
    //                        qDebug() << "receive file:" << id << sId;
    //
    //                        //                  File file = {.id= id,
    //                        //                               .sId= sId,
    //                        //                               .name = qstring(f.name),
    //                        //                               .path= {},
    //                        //                               .size = (quint64)f.size,
    //                        //                               .status = FileStatus::INITIALIZING,
    //                        //                               .direction=FileDirection::RECEIVING};
    //                        //                  qDebug() << "receive file:" << file.toString();
    //                        //                  emit receiveFileRequest(peerId.toFriendId(),
    //                        file);
    //                    }
    //                    isFile = true;
    //                } else {
    //                    // av
    //                    //                auto group = p->findPlugin<FileTransfer>(PluginGroup);
    //                }
    //                break;
    //            }
    //            default: {
    //            }
    //        }
    //    }
    //
    //    if (isFile) {
    //        cacheSessionInfo(session, lib::ortc::JingleCallType::file);
    //        return;
    //    } else {
    //        // av
    //        cacheSessionInfo(session, lib::ortc::JingleCallType::av);
    //
    //        OJingleContentAv cav;
    //        cav.parse(jingle);
    //        cav.sdpType = lib::ortc::JingleSdpType::Offer;
    //        OkRTCManager::getInstance()->getRtc()->CreateAnswer(stdstring(peerId.toString()),
    //        cav);
    //    }

    //  bool isVideo = lib::ortc::JingleCallType::video == callType;
    //  auto _rtcManager = s->getRtcManager();
    //  if (s->isAccepted()) {

    //    OJingleContent answer(lib::ortc::JingleSdpType::Answer,
    //                         stdstring(peerId.username), session->sid(),
    //                         SESSION_VERSION, context.getContents());

    //    _rtcManager->CreateAnswer(stdstring(peerId.toString()), answer);
    //    emit receiveFriendHangup((peerId.username),
    //                             isVideo ? SENDING_V
    //                                     : SENDING_A);
    //  } else {

    //    emit receiveFriendCall((peerId.username), qstring(session->sid()), true,
    //                           isVideo);
    //  }
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

    clearSessionInfo(session);
    _im->endJingle();
    //  emit receiveFriendHangup(peerId.toFriendId(), (int)state);
}

void IMJingle::doSessionAccept(Jingle::Session* session,
                               const Jingle::Session::Jingle* jingle,
                               const IMPeerId& peerId) {
    auto sid = qstring(jingle->sid());
    qDebug() << __func__ << sid << "peerId:" << peerId.toString();
    sessionOnAccept(sid, peerId);
}

void IMJingle::doSessionInfo(const Session::Jingle* jingle, const IMPeerId& friendId) {
    qDebug() << "jingle:%1 peerId:%2"   //
             << qstring(jingle->sid())  //
             << friendId.toString();
}

void IMJingle::doContentAdd(const Session::Jingle* jingle, const IMPeerId& friendId) {
    qDebug() << "jingle:%1 peerId:%2" << qstring(jingle->sid()) << friendId.toString();
    //  _rtcManager->ContentAdd(sdMap, this);
}

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

void IMJingle::doTransportInfo(const Session::Jingle* jingle, const IMPeerId& peerId) {
    auto sid = qstring(jingle->sid());
    qDebug() << __func__ << "sId:" << sid << "peerId:" << peerId.toString();

    auto s = findSession(sid);
    if (!s) {
        qWarning() << ("Session is no existing.");
        return;
    }

    OJingleContentAv content;
    content.parse(jingle);

    for (auto& it : content.contents) {
        OkRTCManager::getInstance()->getRtc()->setTransportInfo(stdstring(peerId.toString()),
                                                                jingle->sid(), it.iceUdp);
    }
}

void IMJingle::doTransportReject(const Session::Jingle*, const IMPeerId&) {}

void IMJingle::doTransportReplace(const Session::Jingle*, const IMPeerId&) {}

void IMJingle::doSecurityInfo(const Session::Jingle*, const IMPeerId&) {}

void IMJingle::doDescriptionInfo(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("sessionId:%1 from:%2") << (qstring(jingle->sid())) << ((peerId.toString()));
}

void IMJingle::doInvalidAction(const Session::Jingle* jingle, const IMPeerId& peerId) {
    qDebug() << ("sessionId:%1 from:%2") << (qstring(jingle->sid())) << ((peerId.toString()));
}

void IMJingle::proposeJingleMessage(const QString& friendId, const QString& callId, bool video) {
    qDebug() << __func__ << "friend:" << friendId << callId;

    StanzaExtensionList exts;
    auto jm = new JingleMessage(JingleMessage::propose, stdstring(callId));
    jm->addMedia(Jingle::Media::audio);
    if (video) {
        jm->addMedia(Jingle::Media::video);
        m_sidVideo.insert(callId, true);
    }
    exts.push_back(jm);

    auto jid = JID{stdstring(friendId)};
    Message m(Message::Chat, jid, {}, {});
    for (auto ext : exts) m.addExtension(ext);

    _im->getClient()->send(m);
}

void IMJingle::rejectJingleMessage(const QString& peerId, const QString& callId) {
    qDebug() << __func__ << "friend:" << peerId << callId;

    StanzaExtensionList exts;
    auto reject = new Jingle::JingleMessage(Jingle::JingleMessage::reject, stdstring(callId));
    exts.push_back(reject);

    auto jid = JID{stdstring(peerId)};
    Message m(Message::Chat, jid, {}, {});
    for (auto ext : exts) m.addExtension(ext);

    _im->getClient()->send(m);
}

void IMJingle::acceptJingleMessage(const IMPeerId& peerId, const QString& callId, bool video) {
    qDebug() << __func__ << "friend:" << peerId.toFriendId() << callId;

    auto proceed = new Jingle::JingleMessage(Jingle::JingleMessage::proceed, stdstring(callId));
    Message proceedMsg(gloox::Message::Chat, JID(stdstring(peerId.toString())));
    proceedMsg.addExtension(proceed);
    _im->getClient()->send(proceedMsg);
    qDebug() << "Sent proceed=>" << peerId.toString();

    // 发送给自己其它终端
    auto accept = new Jingle::JingleMessage(Jingle::JingleMessage::accept, stdstring(callId));

    auto self = _im->self().bareJID();

    Message msg(gloox::Message::Chat, self);
    msg.addExtension(accept);
    _im->getClient()->send(msg);
    qDebug() << "Sent accept=>" << qstring(self.full());

    // 设置状态为接受
    auto ws = findSession(callId);
    if (!ws) {
        ws = createSession(peerId, callId, JingleCallType::av);
    }
    ws->setAccepted(true);
}

void IMJingle::retractJingleMessage(const QString& friendId, const QString& callId) {
    qDebug() << __func__ << "friend:" << friendId << callId;

    auto* jm = new Jingle::JingleMessage(Jingle::JingleMessage::retract, stdstring(callId));

    auto jid = JID{stdstring(friendId)};
    Message m(Message::Chat, jid, {}, {});
    m.addExtension(jm);

    _im->getClient()->send(m);
}

bool IMJingle::handleIq(const IQ& iq) {
    const auto* ibb = iq.findExtension<InBandBytestream::IBB>(ExtIBB);
    if (ibb) {
        IMContactId friendId(qstring(iq.from().bare()));
        qDebug() << __func__ << QString("IBB stream id:%1").arg(qstring(ibb->sid()));

        switch (ibb->type()) {
            case InBandBytestream::IBBOpen: {
                qDebug() << __func__ << QString("Open");
                break;
            }
                //      case InBandBytestream::IBBData: {
                //        qDebug() << __func__ << QString("Data seq:%1").arg(ibb->seq());
                //        emit receiveFileChunk(friendId, qstring(ibb->sid()), ibb->seq(),
                //        ibb->data()); break;
                //      }
                //      case InBandBytestream::IBBClose: {
                //        qDebug() << __func__ << QString("Close");
                //        emit receiveFileFinished(friendId, qstring(ibb->sid()));
                //        break;
                //      }
            default: {
            }
        }

        IQ riq(IQ::IqType::Result, iq.from(), iq.id());
        _im->getClient()->send(riq);
    }

    return true;
    //    auto services = iq.tag()->findChild("services", "xmlns",
    //    XMLNS_EXTERNAL_SERVICE_DISCOVERY); if (services) {
    //      mExtDisco = ExtDisco(services);
    //    }
}

void IMJingle::handleIqID(const IQ& iq, int context) {}

IMJingleSession* IMJingle::findSession(const QString& sId) { return m_sessionMap.value(sId); }

IMJingleSession* IMJingle::createSession(const IMPeerId& to, const QString& sId,
                                         JingleCallType ct) {
    auto s = _sessionManager->createSession(JID(stdstring(to.toString())), this, stdstring(sId));

    auto ws = cacheSessionInfo(s, ct);
    return ws;
}

}  // namespace messenger
}  // namespace lib
