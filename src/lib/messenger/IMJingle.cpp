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

inline IM* getIM(){
  return  ok::session::AuthSession::Instance()->im();
}


IMJingle::IMJingle()
//    : QObject(parent), im(im_), call{call}, fileHandlers{fileHandlers}
{
  qDebug() << __func__ << "Creating";

  qRegisterMetaType<std::string>("std::string");

  auto client = getIM()->getClient();
  client->registerMessageHandler(this);
  client->registerIqHandler(this, ExtIBB);
  client->registerStanzaExtension(new Jingle::JingleMessage());

  //jingle session
  _sessionManager = std::make_unique<SessionManager>(client, this);
  _sessionManager->registerPlugin(new Content());

  auto disco = client->disco();
  //jingle
  disco->addFeature(XMLNS_JINGLE);
  disco->addFeature(XMLNS_JINGLE_MESSAGE);
  disco->addFeature(XMLNS_JINGLE_ERRORS);

  //jingle file
  disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER);
  disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER4);
  disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER5);
  disco->addFeature(XMLNS_JINGLE_FILE_TRANSFER_MULTI);
  disco->addFeature(XMLNS_JINGLE_IBB);
  _sessionManager->registerPlugin(new FileTransfer());
  _sessionManager->registerPlugin(new IBB());

  //jingle av
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



  qDebug() << __func__ << ("Created");
}



IMJingle *IMJingle::getInstance()
{
    static IMJingle * jingle = nullptr;
    if(!jingle){
       jingle = new IMJingle();
    }
    return jingle;
}

IMJingle::~IMJingle() {
    auto client = getIM()->getClient();
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
    auto rtcManager = OkRTCManager::getInstance();
    if(rtcManager)
        rtcManager->setMute(mute);
  }
}

void IMJingle::setRemoteMute(bool mute) {
  for (auto it : m_sessionMap) {
    auto rtcManager = OkRTCManager::getInstance();
    if(rtcManager)
        rtcManager->setRemoteMute(mute);
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
      // 被对方发起呼叫
      qDebug() << "On call from:" << peerId.toString();

      //获取呼叫类型
      bool audio = false;
      bool video = false;
      for(auto& m : jm->medias()){
          if(m == Jingle::audio){
             audio = true;
          }else if(m == Jingle::video){
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
    case Jingle::JingleMessage::accept:{
        //自己其它终端接受，挂断自己
        if(peerId != getIM()->getSelfPeerId()){
            emit receiveFriendHangup(friendId, 0);
        }else{
            //自己终端接受，不处理
//            OkRTCManager::getInstance()->getRtc()->CreateAnswer(peerId.toString());
        }
        break;
    }
    case Jingle::JingleMessage::proceed:
      //对方接受
      emit receiveCallStateAccepted(peerId, sId, jm->medias().size() > 1);
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


  std::list<ortc::IceServer> iceSrvs;

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
      iceSrvs.push_back(ice);
    }


  auto ws = new IMJingleSession(getIM(), peer, sId, callType,
                                  session, iceSrvs,
                                  fileHandlers,
                                  this);

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
            }else{
                //av
//                auto group = p->findPlugin<FileTransfer>(PluginGroup);

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
    }else{
        //av
         cacheSessionInfo(session, lib::ortc::JingleCallType::av);

         OJingleContentAv cav;
        cav.parse(jingle);
        cav.sdpType = lib::ortc::JingleSdpType::Answer;

        OkRTCManager::getInstance()->getRtc()
                ->CreateAnswer(stdstring(peerId.toString()), session->sid(), cav);

    }


//  bool isVideo = lib::ortc::JingleCallType::video == callType;
//  auto _rtcManager = s->getRtcManager();
//  if (s->isAccepted()) {

//    OJingleContent answer(lib::ortc::JingleSdpType::Answer,
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
//  auto s = findSession(sid);
//  if (s) {
//    auto rtcManager = OkRTCManager::getInstance();
//    if (rtcManager) {
//      rtcManager->quit(stdstring(peerId.toString()));
//    }
//  }

  clearSessionInfo(session);
  getIM()->endJingle();
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
    qDebug()<<__func__ << "sId:" << sid << "peerId:" << peerId.toString();

    auto s = findSession(sid);
    if (!s) {
        qWarning()<<("Session is no existing.");
        return;
    }

    OJingleContentAv content;
    content.parse(jingle);

    for (auto &it : content.contents) {
        OkRTCManager::getInstance()->getRtc()
            ->setTransportInfo(stdstring(peerId.toString()), jingle->sid(), it.iceUdp);
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

void IMJingle::onCreatePeerConnection(const std::string &sId,
                                      const std::string &peerId,
                                      bool ok) {
  auto p = qstring(peerId);
  auto s = qstring(sId);

  qDebug()<<__func__<< "sId:" << s
                    << "peerId:" << p
                    << "isOk=>" << ok;

//  emit call->sig_createPeerConnection(s, p, ok);
}

void IMJingle::onRTP(const std::string &sid,    //
                     const std::string &peerId, //
                     const lib::ortc::OJingleContentAv &oContext) {
  auto sId = qstring(sid);
  qDebug()<<__func__ << "sId:"<<sId<<"peerId:"<<qstring(peerId);

  PluginList plugins;
  oContext.toPlugins(plugins);

  auto pSession = findSession(sId);
  if (!pSession) {
    qWarning() << "Unable to find session" << &sId;
    return;
  }

  if (pSession->direction() == CallDirection::CallIn) {
    pSession->getSession()->sessionAccept(plugins);
  } else if (pSession->direction() == CallDirection::CallOut) {
    pSession->getSession()->sessionInitiate(plugins);
  }
}

void IMJingle::onIce(const std::string &sId,    //
                     const std::string &peerId, //
                     const OIceUdp &oIceUdp) {

  auto sid = qstring(sId);

  qDebug()<< __func__
          << "sId:"<< sid
          << "peerId:" << qstring(peerId)
          << "mid:" << qstring(oIceUdp.mid)
          << "mline:" << oIceUdp.mline;

  auto *session = findSession(sid);
  if (!session) {
    qWarning() << "Unable to find session:" << &sId;
    return;
  }


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

  getIM()->getClient()->send(m);

}

void IMJingle::rejectJingleMessage(const QString &peerId, const QString &callId) {

  qDebug() <<__func__<<"friend:"<<peerId << callId;

  StanzaExtensionList exts;
  auto reject = new Jingle::JingleMessage(Jingle::JingleMessage::reject, stdstring(callId));
  exts.push_back(reject);

  auto jid = JID{stdstring(peerId)};
  Message m( Message::Chat, jid, {}, {});
  for(auto ext: exts)
      m.addExtension(ext);

  getIM()->getClient()->send(m);
}

void IMJingle::acceptJingleMessage(const IMPeerId &peerId, const QString &callId, bool video) {
  qDebug() <<__func__<<"friend:"<< peerId.toFriendId() << callId;

  auto proceed = new Jingle::JingleMessage(Jingle::JingleMessage::proceed, stdstring(callId));
  Message proceedMsg(gloox::Message::Chat, JID(stdstring(peerId.toString())));
  proceedMsg.addExtension(proceed);
  getIM()->getClient()->send(proceedMsg);
  qDebug() << "Sent proceed=>"<<peerId.toString();

  // 发送给自己其它终端
  auto accept = new Jingle::JingleMessage(Jingle::JingleMessage::accept, stdstring(callId));

  auto self = getIM()->self().bareJID();

  Message msg(gloox::Message::Chat, self);
  msg.addExtension(accept);
  getIM()->getClient()->send(msg);
  qDebug() << "Sent accept=>" << qstring(self.full());

  // 设置状态为接受
  auto ws = findSession( callId );
  if(!ws){
      ws = createSession(peerId, callId, JingleCallType::av);
  }
  ws->setAccepted(true);
}

void IMJingle::retractJingleMessage(const QString &friendId, const QString &callId) {
  qDebug() <<__func__<<"friend:"<<friendId << callId;

  auto *jm = new Jingle::JingleMessage(Jingle::JingleMessage::retract, stdstring(callId));

  auto jid = JID{stdstring(friendId)};
  Message m( Message::Chat, jid, {}, {});
  m.addExtension(jm);

  getIM()->getClient()->send(m);
}

bool IMJingle::sendCallToResource(const QString &friendId, const QString &sId,
                                  bool video) {

  proposeJingleMessage(friendId, sId, video);
  return true;
}


// startCall
bool IMJingle::startCall(const QString &friendId, const QString &sId, bool video) {
  qDebug()<<__func__<<"friendId:"<<friendId<<"video:"<<video;

  auto resources = getIM()->getOnlineResources(stdstring(friendId));
  if (resources.empty()) {
    qWarning() << "目标用户不在线！";
    return false;
  }

  sendCallToResource(friendId, sId, video);
  return true;
}

bool IMJingle::createCall(const IMPeerId &to, const QString &sId, bool video) {
  qDebug()<<__func__<< "to:" << to.toString() << "sId:" << sId;

  auto ws = createSession(to, sId, JingleCallType::av);

  auto rtc = OkRTCManager::getInstance()->getRtc();
  rtc->addRTCHandler(this);

  std::list<ortc::IceServer> iceServers;

    std::list<ExtDisco::Service> discos;

    ExtDisco::Service disco0;
    disco0.type="turn";
    disco0.host = "chuanshaninfo.com";
    disco0.port=34780;
    disco0.username="gaojie";
    disco0.password="hncs";
    discos.push_back(disco0);

    ExtDisco::Service disco1;
    disco1.type="stun";
    disco1.host = "stun.l.google.com";
    disco1.port=19302;

    discos.push_back(disco1);


//    for (const auto &item :  discos) {
//      ortc::IceServer ice;
//      ice.uri = item.type + ":" + item.host + ":" + std::to_string(item.port);
//      //              "?transport=" + item.transport;
//      ice.username = item.username;
//      ice.password = item.password;
//      qDebug() <<"Add ice:" << ice.uri.c_str();
//      iceServers.push_back(ice);

//      rtc->addIceServer(ice);
//    }

  bool createdCall = rtc->call(stdstring(to.toString()), stdstring(sId), video);
  if(createdCall){
      ws->createOffer(stdstring(to.toString()));
  }

  return createdCall;
}

void IMJingle::cancel(const QString &friendId) {
  qDebug()<<__func__<<friendId;

  auto sId = m_friendSessionMap.value(IMPeerId(friendId));

    auto session = m_sessionMap.value(sId);
    if (session) {
        cancelCall(IMContactId{friendId}, qstring(session->getSession()->sid()));
      clearSessionInfo(session->getSession());
    }

}

void IMJingle::cancelCall(const IMContactId &friendId, const QString &sId) {
  qDebug()<< __func__ << friendId.toString() << sId;

  IMJingleSession *s = findSession(sId);
  if (s) {
    s->doTerminate();
    clearSessionInfo(s->getSession());
  } else {
    // jingle-message
    if (s->direction() == CallDirection:: CallOut) {
      retractJingleMessage(friendId.toString(), sId);
    } else if (s->direction() == CallDirection:: CallIn) {
      rejectJingleMessage(friendId.toString(), sId);
    }
  }
  s->setCallStage(CallStage::StageNone);
}

void IMJingle::rejectCall(const IMPeerId &peerId, const QString &sId)
{
    qDebug()<< __func__ << peerId.toString() << sId;

    IMJingleSession *s = findSession(sId);
    if (s) {
      s->doTerminate();
      clearSessionInfo(s->getSession());
    }else
    {
        rejectJingleMessage(peerId.toString(), sId);
    }
}

bool IMJingle::answer(const IMPeerId &peerId,
                      const QString &callId,
                      bool video) {

  qDebug()<<__func__<< "peer:" << peerId.toString() << "video:"<<  video ;

  auto rtc = OkRTCManager::getInstance()->getRtc();
  rtc->addRTCHandler(this);

  acceptJingleMessage(peerId, callId, video);

  return true;
}

/**
 * 文件传输
 * @param friendId
 * @param file
 */
void IMJingle::rejectFileRequest(const QString &friendId,
                                 const QString &sId) {
    cancelCall(IMContactId{friendId}, sId);
}

void IMJingle::acceptFileRequest(const QString &friendId,
                                 const File &file) {
  qDebug()<< __func__
                << file.name
                << file.sId
                << file.id;

  auto *ws = findSession(file.sId);
  if (!ws) {
    qWarning() <<"Unable to find session sId:" << file.sId;
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
  auto resources = getIM()->getOnlineResources(bare);
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
      getIM()->getClient()->send(riq);
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

IMJingleSession *IMJingle::createSession(const IMPeerId &to, const QString &sId, JingleCallType ct)
{

    auto s = _sessionManager->createSession(
                                    JID(stdstring(to.toString())),
                                    this,
                                    stdstring(sId));

    auto ws = cacheSessionInfo(s, ct);
    return ws;

}


} // namespace messenger
} // namespace lib
