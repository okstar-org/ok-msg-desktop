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

#include <gloox/capabilities.h>
#include <gloox/extdisco.h>
#include <gloox/jinglecontent.h>
#include <gloox/jinglegroup.h>
#include <gloox/jingleiceudp.h>
#include <gloox/jinglertp.h>
#include <gloox/jinglesession.h>
#include <gloox/jinglesessionmanager.h>

#include "IM.h"
#include "IMFile.h"
#include "base/logs.h"

namespace lib {
namespace messenger {

using namespace gloox;
using namespace Jingle;
using namespace lib::ortc;

IMJingle::IMJingle(IM *im_)
    : client(im_), m_callType(lib::ortc::JingleCallType::none),
      m_callStage(StageNone), m_callDirection(CallNone) {

  DEBUG_LOG(("Is be creating ..."));
  forClient(im_->getClient());


  connect(im_, &lib::messenger::IM::receiveCallRequest,
          [&](QString friendId, QString callId, bool audio, bool video) {
            m_callDirection = CallIn;
          });
  DEBUG_LOG(("Is be created"));
}

IMJingle::~IMJingle() { DEBUG_LOG(("...")); }

/**
 * 连接改变
 * @param client
 */
void IMJingle::clientChanged(gloox::Client *client) { forClient(client); }

/**
 * 连接进行初始化
 * @param client
 */
void IMJingle::forClient(Client *client) {
  DEBUG_LOG(("client:%1").arg(qstring(client->getID())));
  assert(client);

  // register extensions Jingle
  //  client->registerStanzaExtension(new Devices());
  //  client->registerStanzaExtension(new AudioMuted());
  //  client->registerStanzaExtension(new VideoMuted());
  //  client->registerStanzaExtension(new VideoType());
  //  client->registerStanzaExtension(new UserAgent());
  //  client->registerStanzaExtension(new RaisedHand());

  _sessionManager = std::make_unique<SessionManager>(client, this);
  _sessionManager->registerPlugin(new FileTransfer());
  _sessionManager->registerPlugin(new Content());
  _sessionManager->registerPlugin(new IBB());
  _sessionManager->registerPlugin(new ICEUDP());
  _sessionManager->registerPlugin(new Group());
  _sessionManager->registerPlugin(new RTP());
}

void IMJingle::join(const JID &room) {
  DEBUG_LOG(("..."));

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
    it.second->getRtcManager()->setMute(mute);
  }
}

void IMJingle::setRemoteMute(bool mute) {
  for (auto it : m_sessionMap) {
    it.second->getRtcManager()->setRemoteMute(mute);
  }
}

void IMJingle::cacheSessionInfo(Jingle::Session *session,
                                JingleCallType callType) {

  JID responder = session->remote();
  const std::string &sId = session->sid();

  PeerId peer(responder);
  auto fIt = m_friendSessionMap.find(peer);
  if (fIt == m_friendSessionMap.end()) {
    m_friendSessionMap.emplace(peer, sId);
  }

  std::list<ortc::IceServer> l;
  if (callType != JingleCallType::none) {
      //
      for (const auto &item : client->extDisco().services()) {
        ortc::IceServer ice;
        ice.uri = item.type + ":" + item.host + ":" + std::to_string(item.port) ;
    //              "?transport=" + item.transport;
        ice.username = item.username;
        ice.password = item.password;
        l.push_back(ice);
      }
  }

  auto wrapSession = new IMJingleSession(stdstring(peer.toString()), sId,
                                         callType, session, l, this, this);
  m_sessionMap.emplace(session->sid(), wrapSession);
}

void IMJingle::clearSessionInfo(Jingle::Session *session) {
  DEBUG_LOG(("Clear the session:%1").arg(qstring(session->sid())))

  auto responder = session->remote();
  m_friendSessionMap.erase(PeerId(responder));

  auto find = m_sessionMap.find(session->sid());
  if (find != m_sessionMap.end()) {
    m_sessionMap.erase(session->sid());
    _sessionManager->discardSession(session);
  }

  m_callDirection = CallNone;
  m_callStage = StageNone;
  m_callType = ortc::JingleCallType::none;

  DEBUG_LOG(("session has be destroyed."))
}

std::string IMJingle::getSessionByFriendId(const QString &friendId) {
  DEBUG_LOG(("getSessionId:%1").arg(friendId))

  auto it = m_friendSessionMap.find(PeerId(friendId));
  if (it != m_friendSessionMap.end()) {
    return it->second;
  }
  return {};
}
/**
 * Jingle sessions
 */

void IMJingle::handleSessionActionError(Action action, Session *session,
                                        const gloox::Error *error) {
  DEBUG_LOG(("sId:%1 action:%2 from:%3 error:%4")
                .arg(qstring(session->sid()))
                .arg(static_cast<int>(action))
                .arg(qstring(session->remote().full()))
                .arg(qstring(error->text())));
}

void IMJingle::handleIncomingSession(Session *session) {
  DEBUG_LOG(("sId:%1").arg(qstring(session->sid())))
  cacheSessionInfo(session, m_callType);
}

// Session
void IMJingle::handleSessionAction(Action action, Session *session,
                                   const Session::Jingle *jingle) {

  auto from = session->remote();
  auto friendId = PeerId(from);
  const std::string &sid = jingle->sid();

  DEBUG_LOG(("action:%1 from:%2 sid:%3")
                .arg(static_cast<int>(action))
                .arg(qstring(from.full()))
                .arg(qstring(sid)))

  auto *ws = findSession(session->sid());
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
                                 const PeerId &peerId) {

  qDebug() << "sId:" << qstring(session->sid());
  qDebug() << "peerId:" << (peerId.toString());



  auto s = findSession(session->sid());
  if (!s) {
    return;
  }
  s->setDirection(CallIn);

  JingleContext context(ortc::JingleSdpType::Answer,
                        stdstring(peerId.toString()), session->sid(),
                        SESSION_VERSION, jingle->plugins());

  auto callType = context.callType();

  if (callType == ortc::JingleCallType::file) {
    // jingle-file
    QList<QString> files;
    for (const auto &c : context.getContents()) {
      for (const auto &f : c.file.files) {
        FileHandler::File file = {qstring(c.file.ibb.sid()), qstring(f.name),
                                  qstring(session->sid()), "", (quint64)f.size};
        DEBUG_LOG(("sId:%1 file:%2 fileId:%3")
                      .arg(file.sId)
                      .arg(file.name)
                      .arg(file.id))
        emit receiveFileRequest(peerId.username + "@"+peerId.server, file);
      }
    }
    s->setContext(context);
    // 返回截断后续处理
    return;
  }

  bool isVideo = lib::ortc::JingleCallType::video == callType;
  auto _rtcManager = s->getRtcManager();
  if (s->isAccepted() || isAccepted) {

    JingleContext answer(lib::ortc::JingleSdpType::Answer,
                         stdstring(peerId.username), session->sid(),
                         SESSION_VERSION, context.getContents());

    _rtcManager->CreateAnswer(stdstring(peerId.toString()), answer);
    emit receiveFriendHangup((peerId.username),
                             isVideo ? TOXAV_FRIEND_CALL_STATE_SENDING_V
                                     : TOXAV_FRIEND_CALL_STATE_SENDING_A);
  } else {

    emit receiveFriendCall((peerId.username), qstring(session->sid()), true,
                           isVideo);
  }
}

void IMJingle::doSessionTerminate(Jingle::Session *session,
                                  const Session::Jingle *jingle,
                                  const PeerId &peerId) {

  DEBUG_LOG(("sId:%1 peerId:%2").arg(qstring(session->sid())).arg(peerId.toString()));
  int ri = 0;
  for (auto &file : m_waitSendFiles) {
    if (qstring(session->sid()) == file.sId) {
      // TODO 需要处理没有terminate的信令的清理
      // 清理待发文件
      DEBUG_LOG(("file:%1 session is terminate.").arg(file.id));
      doStopFileSendTask(session, file);
      m_waitSendFiles.removeAt(ri);
      return;
    }
    ri++;
  }

  /*
   *<jingle action='session-terminate'
   sid='8cfd5b65c45b16822da6b2448f8debb7afa5e0e300000005'
   xmlns='urn:xmpp:jingle:1'> <reason><busy/></reason>
      </jingle>
      reason busy:正忙 decline：拒绝
   */
  auto state = TOXAV_FRIEND_CALL_STATE_FINISHED;
  auto reason = jingle->tag()->findChild("reason");
  if (reason ) {
    if (reason->findChild("busy")) {
        state = TOXAV_FRIEND_CALL_STATE_SENDING_A;
    }
  }
  // rtc
  auto s = findSession(session->sid());
  if (s) {
    auto _rtcManager = s->getRtcManager();
    if (_rtcManager) {
      _rtcManager->quit(stdstring(peerId.toString()));
    }
  }

  clearSessionInfo(session);
  client->endJingle();
  emit receiveFriendHangup(peerId.toFriendId(), (int)state);
}

void IMJingle::doSessionAccept(Jingle::Session *session,
                               const Jingle::Session::Jingle *jingle,
                               const PeerId &peerId) {

  DEBUG_LOG(("sId:%1 friendId:%2") //
                .arg(qstring(session->sid()))
                .arg(peerId.toString()));

  JingleContext answer(lib::ortc::JingleSdpType::Answer, //
                       stdstring(peerId.toString()),     //
                       session->sid(), SESSION_VERSION, jingle->plugins());

  auto callType = answer.callType();
  DEBUG_LOG(("callType:%1").arg((int)callType));

  if (callType == lib::ortc::JingleCallType::file) {
    // jingle-file
    for (auto &file : m_waitSendFiles) {
      if (qstring(session->sid()) == file.sId) {
        doStartFileSendTask(session, file);
        break;
      }
    }
    // 返回截断后续处理
    return;
  }

  // 发送传输信息
  auto *ws = findSession(session->sid());
  if (!ws) {
    qWarning() << "Unable to find session:" << &session->sid();
    return;
  }

  // RTC 接受会话
  ws->getRtcManager()->SetRemoteDescription(stdstring(peerId.toString()),
                                            answer);

  emit receiveFriendHangup(
      peerId.username, answer.hasVideo() ? TOXAV_FRIEND_CALL_STATE_SENDING_V
                                         : TOXAV_FRIEND_CALL_STATE_SENDING_A);
}

void IMJingle::doSessionInfo(const Session::Jingle *jingle,
                             const PeerId &friendId) {
  DEBUG_LOG(("jingle:%1 peerId:%2") //
                .arg(qstring(jingle->sid()))
                .arg(friendId.toString()));
}

void IMJingle::doContentAdd(const Session::Jingle *jingle,
                            const PeerId &friendId) {

  DEBUG_LOG(("jingle:%1 peerId:%2")
                .arg(qstring(jingle->sid()))
                .arg(friendId.toString()));
  //  _rtcManager->ContentAdd(sdMap, this);
}

void IMJingle::doContentRemove(const Session::Jingle *jingle,
                               const PeerId &peerId) {
  DEBUG_LOG(("jingle:%1 peerId:%2")
                .arg(qstring(jingle->sid()))
                .arg(peerId.toString()));

  //  JID peerJID;
  //  std::map<std::string, Session> sdMap;
  //  const PluginList &plugins = jingle->plugins();
  //  for (const auto p : plugins) {
  //    DEBUG_LOG(("Plugin:%1").arg(QString::fromStdString(p->filterString())));
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
                               const PeerId &peerId) {
  DEBUG_LOG(("jingle:%1 peerId:%2") //
                .arg(qstring(jingle->sid()))
                .arg(peerId.toString()));
}

void IMJingle::doContentAccept(const Session::Jingle *jingle,
                               const PeerId &peerId) {
  DEBUG_LOG(("jingle:%1 peerId:%2") //
                .arg(qstring(jingle->sid()))
                .arg(peerId.toString()));
}

void IMJingle::doContentReject(const Session::Jingle *jingle,
                               const PeerId &peerId) {
  DEBUG_LOG(("jingle:%1 peerId:%2") //
                .arg(qstring(jingle->sid()))
                .arg(peerId.toString()));
}

void IMJingle::doTransportAccept(const Session::Jingle *jingle,
                                 const PeerId &peerId) {
  DEBUG_LOG(("jingle:%1 peerId:%2") //
                .arg(qstring(jingle->sid()))
                .arg(peerId.toString()));
}

void IMJingle::doTransportInfo(const Session::Jingle *jingle,
                               const PeerId &peerId) {
  DEBUG_LOG(("sId:%1 peerId:%2") //
                .arg(qstring(jingle->sid()))
                .arg(peerId.toString()));

  auto s = findSession(jingle->sid());
  if (!s) {
    DEBUG_LOG(("Session is not exit."));
    return;
  }

  OIceUdp::OIceUdpMap map;
  OIceUdp::fromJingle(jingle->plugins(), map);

  for (auto &it : map) {
    s->getRtcManager()->SetTransportInfo(stdstring(peerId.toString()),
                                         it.second);
  }
}

void IMJingle::doTransportReject(const Session::Jingle *, const PeerId &) {}

void IMJingle::doTransportReplace(const Session::Jingle *, const PeerId &) {}

void IMJingle::doSecurityInfo(const Session::Jingle *, const PeerId &) {}

void IMJingle::doDescriptionInfo(const Session::Jingle *jingle,
                                 const PeerId &peerId) {
  DEBUG_LOG(("sessionId:%1 from:%2")
                .arg(qstring(jingle->sid()))
                .arg(peerId.toString()));
}

void IMJingle::doInvalidAction(const Session::Jingle *jingle,
                               const PeerId &peerId) {
  DEBUG_LOG(("sessionId:%1 from:%2")
                .arg(qstring(jingle->sid()))
                .arg(peerId.toString()));
}

void IMJingle::onCreatePeerConnection(const std::string &peerId,
                                      const std::string &sId, bool ok) {
  DEBUG_LOG(("peerId:%1 sId:%2 => %3")
                .arg(qstring(peerId))
                .arg(qstring(sId))
                .arg(ok ? "YES" : "NO"))

  /**
   * TODO 处理连接异常情况
   */
  if (!ok) {
    DEBUG_LOG(("连接异常!"));
  }
}

void IMJingle::onRTP(const std::string &sId,    //
                     const std::string &peerId, //
                     const lib::ortc::JingleContext &oContext) {
  DEBUG_LOG(("peerId:%1 sId:%2").arg(qstring(peerId)).arg(qstring(sId)));

  PluginList plugins = oContext.toJingleSdp();

  auto pSession = findSession(sId);
  if (!pSession) {
    qWarning() << "Unable to find session" << &sId;
    return;
  }

  if (pSession->direction() == CallIn) {
    pSession->getSession()->sessionAccept(plugins);
  } else if (pSession->direction() == CallOut) {
    pSession->getSession()->sessionInitiate(plugins);
  }
}

void IMJingle::onIce(const std::string &sId,    //
                     const std::string &peerId, //
                     const OIceUdp &oIceUdp) {

  DEBUG_LOG(("sId:%1 peerId:%2").arg(qstring(sId)).arg(qstring(peerId)));

  auto *session = findSession(sId);
  if (!session) {
    qWarning() << "Unable to find session:" << &sId;
    return;
  }

  DEBUG_LOG(("ice: mid:%1 mline:%2")//
                .arg(qstring(oIceUdp.mid)).arg(oIceUdp.mline))
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
    emit receiveFriendVideoFrame(PeerId(peerId).username, image.width_,
                                 image.height_, image.y, image.u, image.v,
                                 image.ystride, image.ustride, image.vstride);
  }
}

// startCall
bool IMJingle::startCall(const QString &friendId, const QString &sId,
                         bool video) {
  DEBUG_LOG(("friendId:%1 video:%2").arg(friendId).arg(video))

  auto resources = client->getOnlineResources(stdstring(friendId));
  if (resources.empty()) {
    qWarning() << "目标用户不在线！";
    return false;
  }

  
  sendCallToResource(friendId, sId, video);
  return true;
}

bool IMJingle::sendCallToResource(const QString &friendId, const QString &sId,
                                  bool video) {
  m_callStage = StageMessage;
  m_callDirection = CallOut;
  client->proposeJingleMessage(friendId, sId, video);
  return true;
}

bool IMJingle::createCall(const PeerId &to, const QString &sId, bool video) {
  std::string peerId = stdstring(to.toString());

  DEBUG_LOG(("Create session:%1 isVideo:%2 to peerId:%3")
                .arg(sId)
                .arg(video)
                .arg(to.toString()));

  auto session =
      _sessionManager->createSession(JID(peerId), this, stdstring(sId));
  if (!session) {
    DEBUG_LOG(("Cannot create session!"))
    return false;
  }

  cacheSessionInfo(session,
                   video ? JingleCallType::video : JingleCallType::audio);

  auto ws = findSession(session->sid());
  if (ws->getRtcManager()) {
    ws->getRtcManager()->CreateOffer(peerId);
  }

  // 进入session阶段
  m_callStage = StageSession;
  return true;
}

void IMJingle::cancel(const QString &friendId) {
  DEBUG_LOG(("cancel:%1").arg(friendId))

  auto it = m_friendSessionMap.find(PeerId(friendId));
  if (it != m_friendSessionMap.end()) {
    auto sId = it->second;
    auto sIt = m_sessionMap.find(sId);
    if (sIt != m_sessionMap.end()) {
      auto *session = sIt->second;
      cancelCall(friendId, qstring(session->getSession()->sid()));
      clearSessionInfo(session->getSession());
    }
  }
}

void IMJingle::cancelCall(const QString &friendId, const QString &sId) {
  DEBUG_LOG(("friendId:%1 sId:%2").arg(friendId).arg(sId))

  IMJingleSession *s = findSession(stdstring(sId));
  if (s) {
    // session-terminate
    s->getSession()->sessionTerminate(
        new Session::Reason(Session::Reason::Reasons::Cancel));
    if (s->getRtcManager()) {
      PeerId peerId(s->getSession()->remote());
      s->getRtcManager()->quit(stdstring(peerId.toString()));
    }
    clearSessionInfo(s->getSession());
  } else {
    // jingle-message
    if (m_callDirection == CallOut) {
      client->retractJingleMessage(friendId, sId);
    } else if (m_callDirection == CallIn) {
      client->rejectJingleMessage(friendId, sId);
    }
  }

  // 重置呼叫状态
  m_callDirection = CallNone;
  m_callStage = StageNone;
  m_callType = ortc::JingleCallType::none;
}

bool IMJingle::answer(const QString &friendId, const QString &callId,
                      bool video) {
  DEBUG_LOG(("friend:%1 video:%2").arg(friendId).arg(video))
  PeerId friendId1(friendId);

  client->acceptJingleMessage(friendId, callId);
  isAccepted = true;
  m_callType = video ? JingleCallType::video : JingleCallType::audio;
  return true;
}

/**
 * 文件传输
 * @param friendId
 * @param file
 */
void IMJingle::rejectFileRequest(const QString &friendId,
                                 const FileHandler::File &file) {
  cancelCall(friendId, file.sId);
}

void IMJingle::acceptFileRequest(const QString &friendId,
                                 const FileHandler::File &file) {
  DEBUG_LOG(("sId:%1 file:%2 fileId:%3") //
                .arg(file.sId)
                .arg(file.name)
                .arg(file.id))

  auto *ws = findSession(stdstring(file.sId));
  if (!ws) {
    return;
  }

  PluginList pluginList;

  Jingle::FileTransfer::FileList files;
  auto context = ws->getContext();
  for (auto &content : context.getContents()) {
    for (auto &xf : content.file.files) {
      if (xf.name == stdstring(file.name)) {

        auto f =
            new Jingle::FileTransfer(FileTransfer::Request, content.file.files);

        auto i = new Jingle::IBB(content.file.ibb.sid(),
                                 content.file.ibb.blockSize());

        pluginList.emplace_back(f);
        pluginList.emplace_back(i);
      }
    }
  }
  auto c = new Jingle::Content("file", pluginList);
  ws->getSession()->sessionAccept(c);
}

void IMJingle::finishFileRequest(const QString &friendId,
                                 const FileHandler::File &file) {
  DEBUG_LOG(
      ("sId:%1 file:%2 fileId:%3").arg(file.sId).arg(file.name).arg(file.id))
  auto *s = (findSession(stdstring(file.sId)));
  if (!s) {
    return;
  }
  s->getSession()->sessionTerminate(
      new Session::Reason(Session::Reason::Success));
}

void IMJingle::finishFileTransfer(const QString &friendId,
                                  const FileHandler::File &file) {
  finishFileRequest(friendId, file);
}

bool IMJingle::sendFile(const QString &friendId,
                        const FileHandler::File &file) {
  DEBUG_LOG(("friendId:%1 name:%2").arg(friendId).arg(file.name))
  if (file.id.isEmpty()) {
    qWarning() << "文件缺少ID属性！";
    return false;
  }

  JID jid = client->wrapJid(friendId);
  auto resources = client->getOnlineResources(jid.bare());
  if (resources.empty()) {
    qWarning() << "目标用户不在线！";
    return false;
  }

  for (auto &r : resources) {
    jid.setResource(r);
    sendFileToResource(jid, file);
  }

  return true;
}

bool IMJingle::sendFileToResource(const JID &jid,
                                  const FileHandler::File &file) {
  DEBUG_LOG(("peerId:%1").arg(qstring(jid.full())))
  auto session = _sessionManager->createSession(jid, this);
  if (!session) {
    DEBUG_LOG(("Can not create session!"))
    return false;
  }

  cacheSessionInfo(session, JingleCallType::file);

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
  auto &nf = const_cast<FileHandler::File &>(file);
  nf.sId = qstring(session->sid());
  m_waitSendFiles.append(nf);

  return true;
}

IMJingleSession *IMJingle::findSession(const std::string &sId) {
  auto f = m_sessionMap.find((sId));
  if (f == m_sessionMap.end()) {
    DEBUG_LOG(("Can not find the session id is:%1").arg(qstring(sId)))
    return nullptr;
  }
  return f->second;
}

void IMJingle::doStartFileSendTask(const Session *session,
                                   const FileHandler::File &file) {
  DEBUG_LOG(
      ("sId:%1 file:%2 fileId:%3").arg(file.sId).arg(file.name).arg(file.id))

  auto *imFile = new IMFile(session->remote(), file, client);
  connect(imFile, &IMFile::fileSending,
          [&](const JID &m_friendId, const FileHandler::File &m_file, int m_seq,
              int m_sentBytes, bool end) {
            emit sendFileInfo(qstring(m_friendId.username()), m_file, m_seq,
                              m_sentBytes, end);
          });

  connect(imFile, &IMFile::fileAbort,
          [&](const JID &m_friendId, const FileHandler::File &m_file,
              int m_sentBytes) {
            emit sendFileAbort(qstring(m_friendId.username()), m_file,
                               m_sentBytes);
          });

  connect(imFile, &IMFile::fileError,
          [&](const JID &m_friendId, const FileHandler::File &m_file,
              int m_sentBytes) {
            emit sendFileError(qstring(m_friendId.username()), m_file,
                               m_sentBytes);
          });
  imFile->start();
  m_fileSenderMap.insert(file.sId, imFile);
  DEBUG_LOG(("Send file:%1 task has been stared.").arg(file.id));
}

void IMJingle::doStopFileSendTask(const Session *session,
                                  const FileHandler::File &file) {
  Q_UNUSED(session)
  auto *imFile = m_fileSenderMap.value(file.sId);
  if (!imFile) {
    return;
  }
  DEBUG_LOG(("Send file:%1 task will be clear.").arg(file.id));
  imFile->abort();
  if (imFile->isRunning()) {
    imFile->quit();
    imFile->wait();
  }
  disconnect(imFile);
  delete imFile;

  // 返回截断后续处理
  m_fileSenderMap.remove(file.sId);
  DEBUG_LOG(("Send file:%1 task has been clean.").arg(file.id));
}
} // namespace IM
} // namespace lib
