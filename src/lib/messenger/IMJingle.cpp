/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

#include "IMJingle.h"

#include <QUuid>

#include <capabilities.h>
#include <extdisco.h>
#include <jinglecontent.h>
#include <jinglecontent.h>
#include <jinglegroup.h>
#include <jingleiceudp.h>
#include <jinglertp.h>
#include <jinglesession.h>
#include <jinglesessionmanager.h>

#include "IM.h"
#include "IMFile.h"
#include "IMFileTask.h"
#include "IMCall.h"
#include "base/logs.h"

namespace lib {
namespace messenger {

using namespace gloox;
using namespace Jingle;
using namespace lib::ortc;

IMJingle::IMJingle(IM *im_,
                   std::vector<FileHandler *> *fileHandlers,
                   QObject *parent)
    : QObject(parent), im(im_), fileHandlers{fileHandlers}
{
  qDebug() << __func__ << "Creating";

  qRegisterMetaType<std::string>("std::string");

  auto client = im->getClient();
  client->registerMessageHandler(this);
  client->registerIqHandler(this, ExtIBB);
  client->registerIqHandler(this, ExtSrvDisco);

  auto s = client->m_messageHandlers.size();
  qDebug() <<"msgHandlers="<<s;

  client->registerStanzaExtension(new Jingle::JingleMessage());

  auto disco = client->disco();
  disco->addFeature(XMLNS_JINGLE);
  disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER);
  disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER4);
  disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER5);
  disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER_MULTI);

  disco->addFeature(XMLNS_JINGLE_IBB);
  disco->addFeature(XMLNS_JINGLE_ERRORS);
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
  disco->addFeature(XMLNS_JINGLE_MESSAGE);


  _sessionManager = std::make_unique<SessionManager>(client, this);
  _sessionManager->registerPlugin(new FileTransfer());
  _sessionManager->registerPlugin(new Content());
  _sessionManager->registerPlugin(new IBB());
  _sessionManager->registerPlugin(new ICEUDP());
  _sessionManager->registerPlugin(new Group());
  _sessionManager->registerPlugin(new RTP());

  qDebug() << __func__ << ("Created");
}

IMJingle::~IMJingle() {
    auto client = im->getClient();
    client->removeMessageHandler(this);
    qDebug() << __func__ << "Destroyed";
}

void IMJingle::handleMessageSession(MessageSession *session)
{
//  session->registerMessageHandler(this);
}

void IMJingle::handleMessage(const Message &msg, MessageSession *session)
{
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


void IMJingle::join(const JID &room) {
  qDebug()<<("...");

  // join conference
  //    JID focus(XMPP_CONFERENCE_FOCUS);
  // Conference* _conference2 =  new Conference
  // (client->client(), focus, this);
  //    _conference = std::make_shared<Conference>(client->client(), focus,
  //    this);
  //
  //  //加入会议
  //  _conference->join(
  //      room.bareJID(),
  //      QUuid::createUuid().toString().remove(0, 1).remove(36,
  //      1).toStdString());
}

void IMJingle::setMute(bool mute) {
  for (auto it : m_sessionMap) {
    auto m = it->getRtcManager();
    if(m)
        m->setMute(mute);
  }
}

void IMJingle::setRemoteMute(bool mute) {
  for (auto it : m_sessionMap) {
    auto m = it->getRtcManager();
    if(m)
        m->setRemoteMute(mute);
  }
}

void IMJingle::doJingleMessage(const IMPeerId &peerId, const gloox::Jingle::JingleMessage *jm)
{
    qDebug() << __func__
             <<"peerId:"<<peerId.toString()
             << "sId:"<< qstring(jm->id())
             << "action:"<< Jingle::ActionValues[jm->action()];

    auto friendId = peerId.toFriendId();
    qDebug() <<"friendId:" << friendId;

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
    case Jingle::JingleMessage::propose:
    {
      // 被自己账号其它终端接受
      qDebug() << "Accepted by" << peerId.toString();
      emit receiveCallAcceptByOther(sId, peerId);
      break;
    }
    case Jingle::JingleMessage::retract: {
      /**
       * 撤回(发起者取消)，挂断自己
       */
//      mPeerRequestMedias.clear();
      emit receiveCallRetract(friendId, 0);
      break;
    }
    case Jingle::JingleMessage::accept:
    case Jingle::JingleMessage::proceed:
      if (friendId == im->getSelfId().toString()) {
        /**
         * 自己的其他终端接受处理，挂断自己
         */
        emit receiveFriendHangup(friendId, 0);
      } else {
        //对方接受，创建session

        createCall(peerId, sId, jm->medias().size()>0);

        emit receiveCallStateAccepted(peerId, sId, jm->medias().size() > 1);
      }

      break;
    case Jingle::JingleMessage::finish:

      break;
    }
}

IMJingleSession* IMJingle::cacheSessionInfo(Jingle::Session *session,
                                            lib::ortc::JingleCallType callType) {

  auto &responder = session->remote();
  auto sId = qstring(session->sid());
  auto peer = IMPeerId (responder);

  m_friendSessionMap.insert(peer, sId);


  std::list<ortc::IceServer> l;

    std::list<ExtDisco::Service> discos;

    ExtDisco::Service disco;
    disco.type="turn";
    disco.host = "chuanshaninfo.com";
    disco.port=34780;
    disco.username="gaojie";
    disco.password="hncs";
    discos.push_back(disco);

    ExtDisco::Service disco1;
    disco1.type="stun";
    disco1.host = "stun.l.google.com";
    disco1.port=19302;

    discos.push_back(disco1);


    for (const auto &item :  discos) {
      ortc::IceServer ice;
      ice.uri = item.type + ":" + item.host + ":" + std::to_string(item.port);
      //              "?transport=" + item.transport;
      ice.username = item.username;
      ice.password = item.password;
      qDebug() <<"Add ice:" << ice.uri.c_str();
      l.push_back(ice);
    }


  auto ws = new IMJingleSession(im, peer, sId, callType,
                                  session, l,
                                  fileHandlers,
                                  this, this);

  m_sessionMap.insert(sId, ws);

//  connect(ws,
//          &IMJingleSession::sendFileInfo,
//          [&](const JID &m_friendId, const File &m_file, int m_seq,
//          int m_sentBytes, bool end){
//
//  });

  return ws;

}

void IMJingle::clearSessionInfo(Jingle::Session *session) {
    auto sId = qstring(session->sid());

  qDebug() << __func__ << sId;

  auto &responder = session->remote();
  m_friendSessionMap.remove(IMPeerId(responder));
  m_sessionMap.remove(sId);

  _sessionManager->discardSession(session);

}

QString IMJingle::getSessionByFriendId(const QString &friendId) {
  qDebug()<<("getSessionId:%1")<<((friendId));

  return m_friendSessionMap.value(IMPeerId{friendId}, {});
}
/**
 * Jingle sessions
 */

void IMJingle::handleSessionActionError(Action action, Session *session,
                                        const gloox::Error *error) {
  qDebug() << __func__
           << "sid:" << qstring(session->sid())
           << "action:" << static_cast<int>(action)
           << "remote:" << qstring(session->remote().full())
           << "error:" << qstring(error->text());
}

void IMJingle::handleIncomingSession(Session *session) {
  auto sid = qstring(session->sid());
  qDebug() << __func__ << "sId" << sid;

}

// Session
void IMJingle::handleSessionAction(Action action,
                                   Session *session,
                                   const Session::Jingle *jingle) {

  auto from = session->remote();
  auto friendId = IMPeerId(from);
  auto sid = qstring(jingle->sid());

  qDebug()<<__func__<<static_cast<int>(action)
                <<qstring(from.full())
                <<sid;

  auto *ws = findSession(sid);
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

void IMJingle::doSessionInitiate(Jingle::Session *session,
                                 const Jingle::Session::Jingle *jingle,
                                 const IMPeerId &peerId) {
  auto sid =  qstring(session->sid());
  qDebug() <<__func__   << "sId:" << sid
                        << "peerId:" << peerId.toString();

  bool isFile = false;
    for(auto p : jingle->plugins()){
        auto pt = p->pluginType();
        switch (pt) {
        case JinglePluginType::PluginContent: {
            auto file = p->findPlugin<FileTransfer>(PluginFileTransfer);
            auto ibb = p->findPlugin<IBB>(PluginIBB);
            if (file && ibb) {
                for (auto f : file->files()) {
                  auto id = qstring(ibb->sid());
                  auto sId = qstring(session->sid());
                  File file = {.id= id,
                               .sId= sId,
                               .name = qstring(f.name),
                               .path= {},
                               .size = (quint64)f.size,
                               .status = FileStatus::INITIALIZING,
                               .direction=FileDirection::RECEIVING};
                  qDebug() << "receive file:" << file.toString();
                  emit receiveFileRequest(peerId.toFriendId(), file);
                }
                isFile = true;
            }
            break;
        }
        default:{

        }
        }
    }


    if(isFile){
        cacheSessionInfo(session, lib::ortc::JingleCallType::file);
        return;
    }



//  bool isVideo = lib::ortc::JingleCallType::video == callType;
//  auto _rtcManager = s->getRtcManager();
//  if (s->isAccepted()) {

//    JingleContext answer(lib::ortc::JingleSdpType::Answer,
//                         stdstring(peerId.username), session->sid(),
//                         SESSION_VERSION, context.getContents());

//    _rtcManager->CreateAnswer(stdstring(peerId.toString()), answer);
//    emit receiveFriendHangup((peerId.username),
//                             isVideo ? TOXAV_FRIEND_CALL_STATE_SENDING_V
//                                     : TOXAV_FRIEND_CALL_STATE_SENDING_A);
//  } else {

//    emit receiveFriendCall((peerId.username), qstring(session->sid()), true,
//                           isVideo);
//  }
}

void IMJingle::doSessionTerminate(Jingle::Session *session,
                                  const Session::Jingle *jingle,
                                  const IMPeerId &peerId) {

    auto sid = qstring(jingle->sid());
    qDebug()<<__func__<<"sId:"<<sid<<"peerId"<<peerId.toString();

    auto ws = findSession(sid);
    if(!ws){
        qWarning() <<"session is no existing.";
        return;
    }

    ws->onTerminate();

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
  auto state = TOXAV_FRIEND_CALL_STATE_FINISHED;
  auto reason = jingle->tag()->findChild("reason");
  if (reason) {
    if (reason->findChild("busy")) {
      state = TOXAV_FRIEND_CALL_STATE_SENDING_A;
    }
  }
  // rtc
  auto s = findSession(sid);
  if (s) {
    auto _rtcManager = s->getRtcManager();
    if (_rtcManager) {
      _rtcManager->quit(stdstring(peerId.toString()));
    }
  }

  clearSessionInfo(session);
  im->endJingle();
  emit receiveFriendHangup(peerId.toFriendId(), (int)state);
}

void IMJingle::doSessionAccept(Jingle::Session *session,
                               const Jingle::Session::Jingle *jingle,
                               const IMPeerId &peerId) {

  auto sid = qstring(jingle->sid());
  qDebug() << __func__ << sid << peerId.toString();

  auto ws = findSession(sid);
  if(!ws){
      qWarning() <<"Unable to find session"<<sid;
      return;
  }

    ws->onAccept();

//  JingleContext answer(lib::ortc::JingleSdpType::Answer, //
//                       stdstring(peerId.toString()),     //
//                       session->sid(), SESSION_VERSION, jingle->plugins());

//  auto callType = answer.callType();
//  qDebug()<<("callType:%1")<<(((int)callType));

//  if (callType == lib::ortc::JingleCallType::file) {
//    // jingle-file
//    for (auto &file : m_waitSendFiles) {
//      if (qstring(session->sid()) == file.sId) {
//        doStartFileSendTask(session, file);
//        break;
//      }
//    }
//    // 返回截断后续处理
//    return;
//  }else{
//      // 发送传输信息
//      auto *ws = findSession(sid);
//      if (!ws) {
//        qWarning() << "Unable to find session:" << &session->sid();
//        return;
//      }

//      // RTC 接受会话
//      ws->getRtcManager()->SetRemoteDescription(stdstring(peerId.toString()),
//                                                answer);

//      emit receiveFriendHangup(
//          peerId.username, answer.hasVideo() ? TOXAV_FRIEND_CALL_STATE_SENDING_V
//                                             : TOXAV_FRIEND_CALL_STATE_SENDING_A);
//  }
}

void IMJingle::doSessionInfo(const Session::Jingle *jingle,
                             const IMPeerId &friendId) {
  qDebug() << "jingle:%1 peerId:%2" //
           << qstring(jingle->sid()) //
           << friendId.toString();
}

void IMJingle::doContentAdd(const Session::Jingle *jingle,
                            const IMPeerId &friendId) {

  qDebug() << "jingle:%1 peerId:%2" << qstring(jingle->sid())
           << friendId.toString();
  //  _rtcManager->ContentAdd(sdMap, this);
}

void IMJingle::doContentRemove(const Session::Jingle *jingle,
                               const IMPeerId &peerId) {
  qDebug()<<("jingle:%1 peerId:%2")
                <<(qstring(jingle->sid()))
                <<((peerId.toString()));

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

void IMJingle::doContentModify(const Session::Jingle *jingle,
                               const IMPeerId &peerId) {
  qDebug()<<("jingle:%1 peerId:%2") //
                <<((qstring(jingle->sid())))
                <<((peerId.toString()));
}

void IMJingle::doContentAccept(const Session::Jingle *jingle,
                               const IMPeerId &peerId) {
  qDebug()<<("jingle:%1 peerId:%2") //
                <<((qstring(jingle->sid())))
                <<((peerId.toString()));
}

void IMJingle::doContentReject(const Session::Jingle *jingle,
                               const IMPeerId &peerId) {
  qDebug()<<("jingle:%1 peerId:%2") //
                <<((qstring(jingle->sid())))
                <<((peerId.toString()));
}

void IMJingle::doTransportAccept(const Session::Jingle *jingle,
                                 const IMPeerId &peerId) {
  qDebug() << ("jingle:%1 peerId:%2") //
           << qstring(jingle->sid()) << peerId.toString();
}

void IMJingle::doTransportInfo(const Session::Jingle *jingle,
                               const IMPeerId &peerId) {
    auto sid = qstring(jingle->sid());

  qDebug()<<__func__
                <<sid
                <<peerId.toString();

  auto s = findSession(sid);
  if (!s) {
    qDebug()<<("Session is not exit.");
    return;
  }

  OIceUdp::OIceUdpMap map;
  OIceUdp::fromJingle(jingle->plugins(), map);

  for (auto &it : map) {
    s->getRtcManager()->SetTransportInfo(stdstring(peerId.toString()),
                                         it.second);
  }
}

void IMJingle::doTransportReject(const Session::Jingle *, const IMPeerId &) {}

void IMJingle::doTransportReplace(const Session::Jingle *, const IMPeerId &) {}

void IMJingle::doSecurityInfo(const Session::Jingle *, const IMPeerId &) {}

void IMJingle::doDescriptionInfo(const Session::Jingle *jingle,
                                 const IMPeerId &peerId) {
  qDebug()<<("sessionId:%1 from:%2")
                <<(qstring(jingle->sid()))
                <<((peerId.toString()));
}

void IMJingle::doInvalidAction(const Session::Jingle *jingle,
                               const IMPeerId &peerId) {
  qDebug()<<("sessionId:%1 from:%2")
                <<(qstring(jingle->sid()))
                <<((peerId.toString()));
}

void IMJingle::onCreatePeerConnection(const std::string &peerId,
                                      const std::string &sId, bool ok) {
  qDebug()<<("peerId:%1 sId:%2 => %3")
                <<(qstring(peerId))
                <<(qstring(sId))
                <<((ok ? "YES" : "NO"));

  /**
   * TODO 处理连接异常情况
   */
  if (!ok) {
    qDebug()<<("连接异常!");
  }
}

void IMJingle::onRTP(const std::string &sid,    //
                     const std::string &peerId, //
                     const lib::ortc::JingleContext &oContext) {
    auto sId = qstring(sid);
  qDebug()<<("peerId:%1 sId:%2")<<(qstring(peerId))<<sId;

  PluginList plugins = oContext.toJingleSdp();

  auto pSession = findSession(sId);
  if (!pSession) {
    qWarning() << "Unable to find session" << &sId;
    return;
  }

//  if (pSession->direction() == CallIn) {
//    pSession->getSession()->sessionAccept(plugins);
//  } else if (pSession->direction() == CallOut) {
//    pSession->getSession()->sessionInitiate(plugins);
//  }
}

void IMJingle::onIce(const std::string &sId,    //
                     const std::string &peerId, //
                     const OIceUdp &oIceUdp) {
auto sid = qstring(sId);
  qDebug()<< __func__ << sid << (qstring(peerId));

  auto *session = findSession(sid);
  if (!session) {
    qWarning() << "Unable to find session:" << &sId;
    return;
  }

  qDebug()<<("ice: mid:%1 mline:%2") //
                <<(qstring(oIceUdp.mid))
                <<((oIceUdp.mline));
  auto *iceUdp = new ICEUDP(oIceUdp.pwd, oIceUdp.ufrag, oIceUdp.candidates);
  iceUdp->setDtls(oIceUdp.dtls);

  PluginList pluginList;
  pluginList.emplace_back(iceUdp);
  auto c = new Jingle::Content(oIceUdp.mid, pluginList);
  session->getSession()->transportInfo(c);
}

/**
 * 视频渲染
 * @param peerId
 * @param image
 */
void IMJingle::onRender(const std::string &peerId,
                        lib::ortc::RendererImage image) {
  if (peerId.empty()) {
    emit receiveSelfVideoFrame(image.width_, image.height_, image.y, image.u,
                               image.v, image.ystride, image.ustride,
                               image.vstride);
  } else {
    emit receiveFriendVideoFrame(IMPeerId(peerId).username, image.width_,
                                 image.height_, image.y, image.u, image.v,
                                 image.ystride, image.ustride, image.vstride);
  }
}



void IMJingle::proposeJingleMessage(const QString &friendId, const QString &callId, bool video) {
    qDebug() <<__func__<<"friend:"<<friendId << callId;

  StanzaExtensionList exts;
  auto jm = new JingleMessage(JingleMessage::propose, stdstring(callId));
  jm->addMedia(Jingle::Media::audio);
  if (video) {
    jm->addMedia(Jingle::Media::video);
  }
  exts.push_back(jm);

  auto jid = JID{stdstring(friendId)};
  Message m( Message::Chat, jid, {}, {});
  for(auto ext: exts )
      m.addExtension( ext);

  im->getClient()->send(m);

}

void IMJingle::rejectJingleMessage(const QString &friendId, const QString &callId) {

  qDebug() <<__func__<<"friend:"<<friendId << callId;

  StanzaExtensionList exts;
  auto reject = new Jingle::JingleMessage(Jingle::JingleMessage::reject, stdstring(callId));
  exts.push_back(reject);

  auto jid = JID{stdstring(friendId)};
  Message m( Message::Chat, jid, {}, {});
  for(auto ext: exts )
      m.addExtension( ext);

  im->getClient()->send(m);
}

void IMJingle::acceptJingleMessage(const QString &friendId, const QString &callId) {
  qDebug() <<__func__<<"friend:"<<friendId << callId;

  auto proceed = new Jingle::JingleMessage(Jingle::JingleMessage::proceed, stdstring(callId));
  Message proceedMsg(gloox::Message::Chat, JID(stdstring(friendId)));
  proceedMsg.addExtension(proceed);
  im->getClient()->send(proceedMsg);
  qDebug() << "Sent proceed for jingle-message";

  // 发送给自己其它终端
  auto accept = new Jingle::JingleMessage(Jingle::JingleMessage::accept, stdstring(callId));

  Message msg(gloox::Message::Chat,im->self().bareJID());
  msg.addExtension(accept);
  im->getClient()->send(msg);
  qDebug() << "Sent accept for jingle-message";

  // 设置状态为接受
  auto ws = findSession( callId );
  ws->setAccepted(true);
}

void IMJingle::retractJingleMessage(const QString &friendId, const QString &callId) {
  qDebug() <<__func__<<"friend:"<<friendId << callId;

  auto *jm = new Jingle::JingleMessage(Jingle::JingleMessage::retract, stdstring(callId));

  auto jid = JID{stdstring(friendId)};
  Message m( Message::Chat, jid, {}, {});
  m.addExtension(jm);

  im->getClient()->send(m);
}


// startCall
bool IMJingle::startCall(const QString &friendId, const QString &sId, bool video) {
  qDebug()<<__func__<<"friendId:"<<friendId<<"video:"<<video;

  auto resources = im->getOnlineResources(stdstring(friendId));
  if (resources.empty()) {
    qWarning() << "目标用户不在线！";
    return false;
  }

  sendCallToResource(friendId, sId, video);
  return true;
}

bool IMJingle::sendCallToResource(const QString &friendId, const QString &sId,
                                  bool video) {
  im->proposeJingleMessage(friendId, sId, video);
  return true;
}

bool IMJingle::createCall(const IMPeerId &to, const QString &sId, bool video) {
  qDebug()<<__func__<< ("to:") << to.toString();

  auto sw = createSession(to, sId);
  sw->createOffer(stdstring(to.toString()));

  return true;
}

void IMJingle::cancel(const QString &friendId) {
  qDebug()<<__func__<<friendId;

  auto sId = m_friendSessionMap.value(IMPeerId(friendId));

    auto session = m_sessionMap.value(sId);
    if (session) {
      cancelCall(friendId, qstring(session->getSession()->sid()));
      clearSessionInfo(session->getSession());
    }

}

void IMJingle::cancelCall(const QString &friendId, const QString &sId) {
  qDebug()<< __func__ << friendId << sId;

  IMJingleSession *s = findSession(sId);
  if (s) {
    // session-terminate
    s->getSession()->sessionTerminate(
        new Session::Reason(Session::Reason::Reasons::Cancel));
    if (s->getRtcManager()) {
      IMPeerId peerId(s->getSession()->remote());
      s->getRtcManager()->quit(stdstring(peerId.toString()));
    }
    clearSessionInfo(s->getSession());
  } else {
    // jingle-message

    if (s->direction() == CallDirection:: CallOut) {
      im->retractJingleMessage(friendId, sId);
    } else if (s->direction() == CallDirection:: CallIn) {
      im->rejectJingleMessage(friendId, sId);
    }
  }
  s->setCallStage(CallStage::StageNone);
}

bool IMJingle::answer(const QString &friendId, const QString &callId,
                      bool video) {
  qDebug()<<("friend:%1 video:%2")<<(friendId)<<((video));
  IMPeerId friendId1(friendId);

  acceptJingleMessage(friendId, callId);



  return true;
}

/**
 * 文件传输
 * @param friendId
 * @param file
 */
void IMJingle::rejectFileRequest(const QString &friendId,
                                 const QString &sId) {
  cancelCall(friendId, sId);
}

void IMJingle::acceptFileRequest(const QString &friendId,
                                 const File &file) {
  qDebug()<< __func__
                << file.name
                << file.sId
                << file.id;

  auto *ws = findSession(file.sId);
  if (!ws) {
    return;
  }
  // 协议：https://xmpp.org/extensions/xep-0234.html#requesting

  PluginList pluginList;

  Jingle::FileTransfer::FileList files;
  files.push_back(Jingle::FileTransfer::File   {
                        .name = stdstring( file.name),
                        .size = (long)file.size
                    });

    auto ftf = new Jingle::FileTransfer(FileTransfer::Request, files);

    auto ibb = new Jingle::IBB(stdstring(file.txIbb.sid),
                                    file.txIbb.blockSize);

    pluginList.emplace_back(ftf);
    pluginList.emplace_back(ibb);


  auto c = new Jingle::Content("file", pluginList);
  ws->getSession()->sessionAccept(c);
}

void IMJingle::finishFileRequest(const QString &friendId,
                                 const QString &sId) {
  qDebug()<<__func__<<"sId:"<<(sId);
  auto *s = findSession(sId);
  if (!s) {
      qWarning() << "Can not find file session" << sId;
    return;
  }
  s->getSession()->sessionTerminate(new Session::Reason(Session::Reason::Success));
}

void IMJingle::finishFileTransfer(const QString &friendId,
                                  const QString &sId) {
    qDebug()<<__func__<<"sId:"<<(sId);
  finishFileRequest(friendId, sId);
}

bool IMJingle::sendFile(const QString &friendId,
                        const File &file) {
  qDebug()<<__func__ << (friendId) << (file.name);
  if (file.id.isEmpty()) {
    qWarning() << "file id is no existing";
    return false;
  }

  auto bare = stdstring(friendId);
  auto resources = im->getOnlineResources(bare);
  if (resources.empty()) {
    qWarning() << "目标用户不在线！";
    return false;
  }

  JID jid(bare);
  for (auto &r : resources) {
    jid.setResource(r);
    sendFileToResource(jid, file);
  }

  return true;
}

bool IMJingle::sendFileToResource(const JID &jid,
                                  const File &file) {

  qDebug()<<__func__<< qstring(jid.full()) << "sId:"<<file.sId;
  auto session = _sessionManager->createSession(jid, this, stdstring(file.sId));
  if (!session) {
    qDebug() << "Can not create session!";
    return false;
  }

  auto ws = cacheSessionInfo(session, JingleCallType::file);

  PluginList pl;

  // offer-file
  FileTransfer::FileList files;
  FileTransfer::File f;
  f.name = stdstring(file.name);
  f.size = file.size;
  files.emplace_back(f);
  auto ft = new FileTransfer(FileTransfer::Offer, files);
  pl.emplace_back(ft);

  // ibb
  auto ibb = new Jingle::IBB(stdstring(file.id), 4096);
  pl.emplace_back(ibb);

  // content
  auto jc = new Jingle::Content("offer-a-file", pl);
  session->sessionInitiate(jc);

  // 缓存文件
  auto &nf = const_cast<File &>(file);
  nf.sId = qstring(session->sid());
  ws->addFile(nf);

  return true;
}

bool IMJingle::handleIq(const IQ &iq)
{

    const auto *ibb = iq.findExtension<InBandBytestream::IBB>(ExtIBB);
    if (ibb) {
      IMContactId friendId(qstring(iq.from().bare()));
      qDebug() << __func__<<QString("IBB stream id:%1").arg(qstring(ibb->sid()));

      switch (ibb->type()) {
      case InBandBytestream::IBBOpen: {
        qDebug() << __func__ << QString("Open");
        break;
      }
      case InBandBytestream::IBBData: {
        qDebug() << __func__ << QString("Data seq:%1").arg(ibb->seq());
        emit receiveFileChunk(friendId, qstring(ibb->sid()), ibb->seq(), ibb->data());
        break;
      }
      case InBandBytestream::IBBClose: {
        qDebug() << __func__ << QString("Close");
        emit receiveFileFinished(friendId, qstring(ibb->sid()));
        break;
      }
      default: {
      }
      }

      IQ riq(IQ::IqType::Result, iq.from(), iq.id());
      im->getClient()->send(riq);
    }

    return true;
//    auto services = iq.tag()->findChild("services", "xmlns", XMLNS_EXTERNAL_SERVICE_DISCOVERY);
//    if (services) {
//      mExtDisco = ExtDisco(services);
//    }
}

void IMJingle::handleIqID(const IQ &iq, int context)
{

}

IMJingleSession *IMJingle::findSession(const QString &sId) {
  return m_sessionMap.value(sId);
}

IMJingleSession *IMJingle::createSession(const IMPeerId &to, const QString &sId)
{

    auto s = _sessionManager->createSession(JID(to.toString().toStdString()),
                                   this,
                                   stdstring(sId));
    auto ws = cacheSessionInfo(s, video ? JingleCallType::video : JingleCallType::audio);

    return ws;

}


} // namespace messenger
} // namespace lib
