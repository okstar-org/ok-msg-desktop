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

#include "messenger.h"

#include "base/logs.h"
#include "base/xmls.h"

#include "lib/messenger/IM.h"
#include "lib/messenger/IMConference.h"
#include "lib/messenger/IMJingle.h"
#include "lib/plugin/pluginmanager.h"

#include <QThread>
#include <jid.h>
#include <memory>

namespace lib {
namespace messenger {


Messenger::Messenger(QObject *parent)
    : QObject(parent),                                       //
      _delayer(std::make_unique<::base::DelayedCallTimer>()) //
{
    qDebug() << __func__;

  qRegisterMetaType<File>("File");
  qRegisterMetaType<FriendId>("FriendId");
  qRegisterMetaType<PeerId>("PeerId");
  qRegisterMetaType<std::string>("std::string");
  qRegisterMetaType<IMMessage>("IMMessage");

  connect(this, &Messenger::disconnect, this, &Messenger::onDisconnect);

  /**
   * IM
   */
  connectIM();

  /**
   * Jingle
   */
  connectJingle();

  QStringList features;
#ifdef OK_PLUGIN
  auto pm = ok::plugin::PluginManager::instance();
  auto features0 = pm->pluginFeatures();
  features << features0;

  auto _session = ok::session::AuthSession::Instance();
  int acc = pm->addAccount(_session->account(), this);
  qDebug() << "PluginManager account id=>"<<acc;

  ok::session::AuthSession::Instance();
  auto _im = _session->im();
  connect(_im, &IM::connected, this, [pm,_session]() {
//    pm->accountBeforeLogin(acc);
    pm->startLogin(_session->account());
  });

#endif
}

Messenger::~Messenger() {
    qDebug() << __func__;
}

//Messenger *Messenger::getInstance() {
//  static Messenger *self = nullptr;
//  if (!self)
//    self = new Messenger();
//  return self;
//}

void Messenger::start() {
qDebug() << __func__;
}

void Messenger::setMute(bool mute) { _jingle->setMute(mute); }

void Messenger::setRemoteMute(bool mute) { _jingle->setRemoteMute(mute); }

void Messenger::sendChatState(const QString &friendId, int state) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->sendChatState(friendId, static_cast<ChatStateType>(state));
}

void Messenger::onConnectResult(IMStatus status) {
  qDebug() << ("status:") << qstring(IMStatusToString(status));
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  if (status == IMStatus::DISCONNECTED) {
    _delayer->call(1000 * 5, [&]() {
      qDebug(("Retry connect..."));
      _im->doConnect();
    });
  }
}

void Messenger::onStarted() {
  qDebug() << "connected...";

  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();

//  _im->enableRosterManager();
//  _im->sendPresence();
//  _im->sendServiceDiscoveryItems();

#ifdef OK_PLUGIN
  qDebug() << "Initialize plugin manager...";
  qRegisterMetaType<QDomDocument>("QDomDocument");
  connect(_im, &IM::exportEncryptedMessage, this, &Messenger::onEncryptedMessage);
  qDebug() << "Initialized plugin manager successfully";
#endif
  qDebug() << "connected completed";
}

void Messenger::onStopped() { qDebug() << "onStopped..."; }

bool Messenger::connectIM( ) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();

  connect(_im, &IM::connected, this, [&]() {
    emit connected();
  });

  connect(_im, &IM::started, this, &Messenger::onStarted);
//  connect(this, &Messenger::stopped, this, &Messenger::onStopped);


//  connect(_im, &IM::onStopped, this, [&]() {
//    onStopped();
//  });

  connect(_im, &IM::incoming, this,
          [=, this](QString xml) { emit incoming(xml); }, Qt::QueuedConnection);

  /**
   * selfHandlers
   */
  connect(_im, &IM::selfNicknameChanged, this,
          [&](const QString &nickname) {
            for (auto handler : selfHandlers) {
              handler->onSelfNameChanged(nickname);
            }
          });
  connect(_im, &IM::selfAvatarChanged, this,
          [&](const std::string avatar) {
            for (auto handler : selfHandlers) {
              handler->onSelfAvatarChanged(avatar);
            }
          });
  connect(_im, &IM::selfStatusChanged, this,
          [&](int type, const std::string &status) {
            for (auto handler : selfHandlers) {
              handler->onSelfStatusChanged(static_cast<Tox_User_Status>(type),
                                           status);
            }
          });
  connect(
      _im, &IM::selfIdChanged, this,
      [&](QString id) {
        for (auto handler : selfHandlers) {
          handler->onSelfIdChanged(id);
        }
      });

  /**
   * friendHandlers
   */
  connect(_im, &IM::receiveFriend, this,
          [&](const Friend& frnd){
            for (auto handler : friendHandlers) {
              handler->onFriend(frnd);
            }
          });

  connect(_im, &IM::receiveFriendRequest, this,
          [&](const QString friendId, QString msg) -> void {
            for (auto handler : friendHandlers) {
              handler->onFriendRequest(friendId, msg);
            }
          });

  connect(_im, &IM::receiveMessageReceipt, this,
          [&](QString friendId, QString receipt) -> void {
            for (auto handler : friendHandlers) {
              handler->onMessageReceipt(friendId, receipt);
            }
          });

  connect(_im, &IM::receiveFriendStatus, this,
          [&](QString friendId, int type) -> void {
            for (auto handler : friendHandlers) {
              handler->onFriendStatus(friendId, static_cast<Tox_User_Status>(type));
            }
          });

  connect(_im, &IM::receiveFriendChatState, this,
          [&](QString friendId, int state) -> void {
            for (auto handler : friendHandlers) {
              handler->onFriendChatState(friendId, state);
            }
          });

  connect(_im, &IM::receiveFriendMessage, this,
          [&](QString friendId, IMMessage msg) -> void {
            for (auto handler : friendHandlers) {
              handler->onFriendMessage(friendId, msg);
            }
          });

  connect(_im, &IM::receiveFriendMessageSession, this,
          [&](QString friendId, QString sid) -> void {
            for (auto handler : friendHandlers) {
              handler->onFriendMessageSession(friendId, sid);
            }
          });

  connect(_im, &IM::receiveNicknameChange, this,
          [&](QString friendId, QString nickname) {
            for (auto handler : friendHandlers) {
              handler->onFriendNameChanged(friendId, nickname);
            }
          });

  connect(_im, &IM::receiveFriendAvatarChanged, this,
          [&](QString friendId, std::string avatar) {
            for (auto handler : friendHandlers) {
              handler->onFriendAvatarChanged(friendId, avatar);
            }
          });

  connect(_im, &IM::receiveFriendAliasChanged, this,
          [&](const JID& friendId, const std::string& alias) {
            for (auto handler : friendHandlers) {
              handler->onFriendAliasChanged(FriendId(friendId.bareJID()), qstring(alias));
            }
          });

  /**
   * callHandlers
   */
  connect(_im, &IM::receiveCallRequest, this,
          [&](QString friendId, QString callId, bool audio, bool video) {
            for (auto handler : callHandlers) {
              handler->onCall(friendId, callId, audio, video);
            }
          });

  connect(_im, &IM::receiveCallRetract, this,
          [&](QString friendId, int state) {
            for (auto handler : callHandlers) {
              handler->onCallRetract(friendId, state);
            }
          });

  connect(_im, &IM::receiveCallAcceptByOther, this,
          [&](const QString& callId, const PeerId& peerId) {
            for (auto handler : callHandlers) {
              handler->onCallAcceptByOther(callId, peerId);
            }
          });

  connect(_im, &IM::receiveCallStateAccepted, this,
          [&](PeerId friendId, QString callId, bool video) {
            for (auto handler : callHandlers) {
              handler->receiveCallStateAccepted(friendId, callId, video);
            }
          });

  connect(_im, &IM::receiveCallStateRejected, this,
          [&](PeerId friendId, QString callId, bool video) {
            for (auto handler : callHandlers) {
              handler->receiveCallStateRejected(friendId, callId, video);
            }
          });

  connect(_im, &IM::receiveFriendHangup, this,
          [&](QString friendId, int state) {
            for (auto handler : callHandlers) {
              handler->onHangup(
                  friendId, (TOXAV_FRIEND_CALL_STATE)state);
            }
          });

  //group
  connect(_im, &IM::groupInvite, this,
          [&](const QString &groupId, const QString &peerId,
              const QString &message) {
            for (auto handler : groupHandlers) {
              handler->onGroupInvite(groupId, peerId, message);
            }
          });


  connect(_im, &IM::groupSubjectChanged, this,
          [&](const JID &group, const std::string &subject) -> void {
            for (auto handler : groupHandlers) {
              handler->onGroupSubjectChanged(qstring(group.bare()), qstring(subject));
            }
          });

  connect(_im, &IM::groupReceived, this,
          &Messenger::onGroupReceived);

  connect(_im, &IM::receiveRoomMessage, this,
          [&](QString groupId, PeerId peerId, IMMessage msg) -> void {
            for (auto handler : groupHandlers) {
              handler->onGroupMessage(groupId, peerId, msg);
            }
          });

  connect(_im, &IM::groupOccupants, this,
          [&](const QString &groupId, const uint size) -> void {
            for (auto handler : groupHandlers) {
              handler->onGroupOccupants(groupId, size);
            }
          });

  connect(
      _im, &IM::groupOccupantStatus, this,
      [&](const QString &groupId, const GroupOccupant &go) -> void {
        for (auto handler : groupHandlers) {
          handler->onGroupOccupantStatus(groupId, go);
        }
      });

  qRegisterMetaType<GroupInfo>("GroupInfo");
  connect(_im, &IM::groupRoomInfo, this,
          [&](const QString &groupId, const  GroupInfo info) -> void {
            for (auto handler : groupHandlers) {
              handler->onGroupInfo(groupId, info);
            }
          });

  /*file handler*/
  connect(_im, &IM::receiveFileChunk, this,
          [&](const FriendId &friendId, const QString &sId,
              int seq, const std::string& chunk) -> void {
            for (auto handler : fileHandlers) {
              handler->onFileRecvChunk(friendId.toString(), sId, seq, chunk);
            }
          });
  connect(
      _im, &IM::receiveFileFinished, this,
      [&](const FriendId &friendId,const QString &sId) -> void {
        for (auto handler : fileHandlers) {
              handler->onFileRecvFinished(friendId.toString(), sId);
        }
      });
  return true;
}

bool Messenger::connectJingle() {
  qDebug()<<"connectJingle...";

  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();

  _jingle = std::make_unique<IMJingle>(_im);

  connect(_jingle.get(), &IMJingle::receiveFriendHangup, this,
          [&](QString friendId, int state) {
            for (auto handler : callHandlers) {
              handler->onHangup(friendId, (TOXAV_FRIEND_CALL_STATE)state);
            }
          });

  connect(_jingle.get(), &IMJingle::receiveFriendVideoFrame,
          this,
          [&](const QString &friendId, //
              uint16_t w, uint16_t h,  //
              const uint8_t *y, const uint8_t *u, const uint8_t *v,
              int32_t ystride, int32_t ustride, int32_t vstride) {
            emit receiveFriendVideoFrame(friendId, //
                                         w, h,     //
                                         y, u, v,  //
                                         ystride, ustride, vstride);
          });

  connect(_jingle.get(), &IMJingle::receiveSelfVideoFrame, this,
          [&](uint16_t w, uint16_t h, //
              const uint8_t *y, const uint8_t *u, const uint8_t *v,
              int32_t ystride, int32_t ustride, int32_t vstride) {
            emit receiveSelfVideoFrame(w, h,    //
                                       y, u, v, //
                                       ystride, ustride, vstride);
          });

  connect(_jingle.get(), &IMJingle::receiveFileRequest, this,
          [&](const QString &friendId, const File &file) {
            for (auto h : fileHandlers) {
              h->onFileRequest(friendId, file);
            }
          });
  connect(_jingle.get(), &IMJingle::sendFileInfo, this,
          [&](const QString &friendId, const File &file, int m_seq,
              int m_sentBytes, bool end) {
            for (auto h : fileHandlers) {
              h->onFileSendInfo(friendId, file, m_seq, m_sentBytes, end);
            }
          });
  connect(_jingle.get(), &IMJingle::sendFileAbort, this,
          [&](const QString &friendId, const File &file,
              int m_sentBytes) {
            for (auto h : fileHandlers) {
              h->onFileSendAbort(friendId, file, m_sentBytes);
            }
          });
  connect(_jingle.get(), &IMJingle::sendFileError, this,
          [&](const QString &friendId, const File &file,
              int m_sentBytes) {
            for (auto h : fileHandlers) {
              h->onFileSendError(friendId, file, m_sentBytes);
            }
          });

  qDebug()<<"connectJingle done";
  return true;
}

bool Messenger::initRoom() {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->loadGroupList();
  return true;
}

void Messenger::onReceiveGroupMessage(IMMessage msg) {
  emit receivedGroupMessage(msg);
}

QString Messenger::genUniqueId() {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return qstring(_im->getClient()->getID());
}

bool Messenger::sendToGroup(const QString &g,   //
                            const QString &msg, //
                            QString &receiptNum) {
  Q_UNUSED(receiptNum)
  qDebug() << QString("sendToGroup=>%1 msg:%2").arg(g).arg(msg);
  sentCount++;

  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();

  auto id = _im->sendToRoom(g, msg, "");
  if(id.isEmpty())
      return false;

  receiptNum = id;
  return true;
}

bool Messenger::sendFileToFriend(const QString &f,
                                 const File &file) {
  return _jingle->sendFile(f, file);
}

bool Messenger::sendToFriend(const QString &f, const QString &msg,
                             QString &receiptNum, bool encrypt) {
  qDebug() << QString("msg:%1=>%2").arg(msg).arg(f);
  sentCount++;

  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->makeId(receiptNum);

  bool y = false;
  if (encrypt) {
#ifdef OK_PLUGIN
    auto _session = ok::session::AuthSession::Instance();
    base::Jid ownJid(qstring(_im->self().full()));
    _session->account()->setJid(ownJid);

    auto pm = ok::plugin::PluginManager::instance();
    pm->addAccount(_session->account(), this);

    auto dom = _im->buildMessage(f, msg, receiptNum);
    auto ele = dom.documentElement();

    if (pm->encryptMessageElement(_session->account(), ele)) {
      qDebug()<<"encryptMessageElement=>"<<ele.ownerDocument().toString();
      auto xml = ::base::Xmls::format(ele);
      _im->send(xml);
      y = true;
    }
#endif
  }
  if (!y) {
    y = _im->sendTo(f, msg, receiptNum);
    qDebug() << QString("sendTo=>%1").arg(y);
  }
  return y;
}

void Messenger::receiptReceived(const QString &f, QString receipt) {
  qDebug() << "receiptReceived receiver:" << f << "receiptNum:" << receipt;
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return _im->sendReceiptReceived(f, receipt);
}

bool Messenger::callToFriend(const QString &f, const QString &sId, bool video) {
  qDebug() << QString("friend:%1 video:%2").arg((f)).arg(video);
  return _jingle->startCall(f, sId, video);
}

bool Messenger::createCallToPeerId(const PeerId &to,
                                   const QString &sId, bool video) {

  qDebug() << QString("peerId:%1 video:%2").arg((to.toString())).arg(video);
  return _jingle->createCall(to, sId, video);
}

bool Messenger::answerToFriend(const QString &f, const QString &callId,
                               bool video) {
  qDebug() << QString("friend:%1 video:%2").arg((f)).arg(video);
  return _jingle->answer(f, callId, video);
}

bool Messenger::cancelToFriend(const QString &f, const QString &callId) {
  _jingle->cancelCall(f, callId);
  return true;
}

void Messenger::sendFriendRequest(const QString &f,const QString &nick, const QString &message) {
  qDebug() <<__func__ << f << nick << message;
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->addFriend(JID(stdstring(f)), nick, message);
}

void Messenger::acceptFriendRequest(const QString &f) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->acceptFriendRequest(f);
}

void Messenger::rejectFriendRequest(const QString &f) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->rejectFriendRequest(f);
}

void Messenger::getFriendVCard(const QString &f) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->fetchFriendVCard(f);
}

bool Messenger::removeFriend(const QString &f) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return _im->removeFriend(JID(f.toStdString()));
}

size_t Messenger::getFriendCount() {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return _im->getRosterCount();
}

void Messenger::getFriendList(std::list<Friend>& list) {
  auto _session = ok::session::AuthSession::Instance();
  _session->im()->getRosterList(list);
}

void Messenger::setFriendAlias(const QString &f, const QString &alias)
{
    auto _session = ok::session::AuthSession::Instance();
    _session->im()->setFriendAlias(JID(stdstring(f)), stdstring(alias));
}

Tox_User_Status Messenger::getFriendStatus(const QString &f) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
 return _im->getFriendStatus(f);
}


void Messenger::addSelfHandler(SelfHandler *handler) {
  selfHandlers.push_back(handler);
}

void Messenger::addFriendHandler(FriendHandler *handler) {
  friendHandlers.push_back(handler);
}

void Messenger::addGroupHandler(GroupHandler *handler) {
  groupHandlers.push_back(handler);
}

void Messenger::addCallHandler(CallHandler *handler) {
  callHandlers.emplace_back(handler);
}

void Messenger::addFileHandler(FileHandler *handler) {
  fileHandlers.emplace_back(handler);
}

void Messenger::stop() {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->stop();
}

void Messenger::send(const QString &xml) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->send(xml);
}

PeerId Messenger::getSelfId() const {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return _im->getSelfPeerId();
}

Tox_User_Status Messenger::getSelfStatus() const {
  auto pt = gloox::Presence::PresenceType::Available;
  // _im->getPresenceType();
  return static_cast<Tox_User_Status>(pt);
}

void Messenger::setSelfNickname(const QString &nickname) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->setNickname(nickname);
}

QString Messenger::getSelfUsername() const {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return _im->getSelfUsername();
}

QString Messenger::getSelfNick() const {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return _im->getNickname();
}

void Messenger::changePassword(const QString &password) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->changePassword(password);
}

void Messenger::onDisconnect() {
  _delayer->call(1000 * 5, [&]() {
    qDebug(("retry connect..."));
    auto _session = ok::session::AuthSession::Instance();
    auto _im = _session->im();
    _im->start();
  });
}

void Messenger::onEncryptedMessage(QString xml) {
#ifdef OK_PLUGIN
  if(xml.isNull())
  {
    qWarning()<<"Empty encryptedMessage!";
    return;
  }
  auto dom = ::base::Xmls::parse(xml);

  qDebug() << "onEncryptedMessage:"<<dom.toString();
  auto _session = ok::session::AuthSession::Instance();
  auto info = _session->getSignInInfo();
  auto _im = _session->im();
  _session->account()->setJid(qstring(_im->self().full()));
  auto pm = ok::plugin::PluginManager::instance();
  pm->addAccount(_session->account(), this);
  auto ele= dom.documentElement();
  bool decrypted = pm->decryptMessageElement(_session->account(), ele);
  if (!decrypted){
    qWarning()<<"decrypted failed.";
    return;
  }
  auto body = ele.firstChildElement("body").text();
  if (body.isEmpty()){
    qWarning()<<"Empty body!";
    return;
  }

  auto from = ele.attribute("from");
  auto to = ele.attribute("to");
  auto id = ele.attribute("id");

  auto msg = IMMessage{MsgType::Chat, id, from, to, body, QDateTime::currentDateTime()};

  for (auto handler : friendHandlers) {
    handler->onFriendMessage(qstring(JID(stdstring(from)).bareJID().bare()), msg);
  }
#endif
}

void Messenger::loadGroupList() {
  ok::session::AuthSession::Instance()->im()->loadGroupList();
}

bool Messenger::createGroup(const QString &group) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->createRoom(JID(stdstring(group)));
  return true;
}

bool Messenger::inviteGroup(const QString &group, const QString &f) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return _im->inviteToRoom(JID(stdstring(group)), JID(stdstring(f)));
}

bool Messenger::leaveGroup(const QString &group) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return _im->leaveGroup(group);
}


bool Messenger::destroyGroup(const QString &group) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  return _im->destroyGroup(group);
}

void Messenger::setRoomName(const QString &group, const QString &nick) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->setRoomName(group, stdstring(nick));
}

void Messenger::setRoomDesc(const QString &group, const QString &nick)
{
    auto _session = ok::session::AuthSession::Instance();
    auto _im = _session->im();
    _im->setRoomDesc(group, stdstring(nick));
}

void Messenger::setRoomSubject(const QString &group, const QString &notice) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->setRoomSubject(group, stdstring(notice));
}

void Messenger::setRoomAlias(const QString &group, const QString &alias)
{
    auto _session = ok::session::AuthSession::Instance();
    _session->im()->setRoomAlias(group, stdstring(alias));
}

bool Messenger::callToGroup(const QString &g) {
  Q_UNUSED(g)
  return true;
}

void Messenger::joinGroup(const QString &group) {
  qDebug() << QString("group:%1").arg(group);
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->joinRoom(group);
}

void Messenger::setSelfAvatar(const QByteArray &avatar) {
  auto _session = ok::session::AuthSession::Instance();
  auto _im = _session->im();
  _im->setAvatar(avatar);
}

void Messenger::rejectFileRequest(QString friendId, const File &file) {
  _jingle->rejectFileRequest(friendId, file);
}

void Messenger::acceptFileRequest(QString friendId, const File &file) {
  _jingle->acceptFileRequest(friendId, file);
}

void Messenger::cancelFile(QString fileId) {
  qDebug() << QString("fileId:%1").arg(fileId);
}

void Messenger::finishFileRequest(QString friendId, const QString &sId) {
  qDebug() << __func__<<sId;
  _jingle->finishFileRequest(friendId, sId);
}

void Messenger::finishFileTransfer(QString friendId,
                                   const QString &sId) {
    qDebug() << __func__<<sId;
  _jingle->finishFileTransfer(friendId, sId);
}

void Messenger::requestBookmarks() {
  auto session = ok::session::AuthSession::Instance();
  auto im = session->im();
  im->requestVCards();

 // im->requestBookmarks();
  //im->enablePubSubManager();
}

void Messenger::setUIStarted(){
  auto session = ok::session::AuthSession::Instance();
  auto im = session->im();
  im->setUIStarted();
}

void Messenger::onGroupReceived(QString groupId, QString name) {
    for (auto handler : groupHandlers) {
      handler->onGroup(groupId, name);
    }
}

QString File::toString() const
{
    return QString("{id:%1, sId:%2, name:%3, path:%4, size:%5, status:%6, direction:%7}")
           .arg(id).arg(sId).arg(name).arg(path).arg(size)
           .arg((int)status).arg((int)direction);
}

QDebug &operator<<(QDebug &debug, const File &f) {
  QDebugStateSaver saver(debug);
  debug.nospace() << f.toString();
  return debug;
}


} // namespace messenger
} // namespace lib
