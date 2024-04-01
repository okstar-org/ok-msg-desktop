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

#include "IM.h"

// TODO resolve conflict DrawText in WinUser.h
#undef DrawText

#include "base/hashs.h"
#include "base/logs.h"
#include "base/xmls.h"

#include <list>
#include <string>
#include <thread>
#include <utility>

#include <avatar.h>
#include <attention.h>
#include <base64.h>
#include <capabilities.h>
#include <chatstate.h>
#include <conference.h>
#include <dataformitem.h>
#include <disco.h>
#include <extdisco.h>
#include <inbandbytestream.h>
#include <message.h>
#include <nickname.h>
#include <pubsubevent.h>
#include <receipt.h>
#include <rostermanager.h>
#include <vcardupdate.h>

namespace lib {
namespace messenger {

using namespace gloox;

#define DISCO_CTX_CONF 0
#define DISCO_CTX_ROSTER 1
#define DISCO_CTX_BOOKMARKS 2

/**
 * 聊天通讯核心类
 * @param user
 * @param pwd
 * @param features_
 */
IM::IM(QString host, QString user, QString pwd,
       QStringList features_)                           //
    : osInfo(::base::SystemInfo::instance()->osInfo()), //
      features(std::move(features_)),                   //
      _host(stdstring(host)),                           //
      _username(stdstring(user)),                       //
      _password(stdstring(pwd)),                        //
      _status(IMStatus::DISCONNECTED),                  //
      mExtDisco{ExtDisco()} {

  qDebug() << ("Create messenger instance...");

  setObjectName("IM-Connect");
  // 生成本机resource. 格式:OkEDU.<HOST>.[VER].[UNIQUE]
  _resource = QString("%1.%2.[%3].%4")         //
                  .arg(APPLICATION_SHORT_NAME, //
                       osInfo.hostName,
                       GIT_DESCRIBE, //
                       osInfo.uniqueId.mid(0, 6))
                  .toStdString();
  qDebug() << "Generate self resource:" << _resource.c_str();

  // qRegisterMetaType
  qRegisterMetaType<JID>("JID");
  qRegisterMetaType<FriendId>("FriendId");
  qRegisterMetaType<PeerId>("PeerId");
  qRegisterMetaType<IMMessage>("IMMessage");

  startTimer(20 * 1000);
  qDebug() << ("Create messenger instance is successfully");

  connect(this, &IM::selfNicknameChanged, this, &IM::onSelfNicknameChanged);
}

IM::~IM() {
  DEBUG_LOG(("IM::~IM..."));
  disconnect(this, &IM::selfNicknameChanged, this, &IM::onSelfNicknameChanged);
}

void IM::run() { doConnect(); }

void IM::timerEvent(QTimerEvent *e) {}

std::unique_ptr<Client> IM::makeClient() {
  JID loginJid(_username + "@" + _host + "/" + _resource);

  DEBUG_LOG(("Using Jid:%1").arg(qstring(loginJid.full())));

  /**
   * Client
   */
  auto client = std::make_unique<Client>(loginJid, _password);
  auto disco = client->disco();

  disco->setVersion("disco", APPLICATION_VERSION, stdstring(osInfo.prettyName));
  disco->setIdentity("client", "pc", APPLICATION_RELEASE);
  disco->addIdentity("pubsub", "pep");

  disco->addFeature(XMLNS_PUBSUB);
  disco->addFeature(XMLNS_PUBSUB_EVENT);
  disco->addFeature(XMLNS_PUBSUB_OWNER);
  disco->addFeature(XMLNS_PUBSUB_PUBLISH_OPTIONS);
  disco->addFeature(XMLNS_PUBSUB_AUTO_SUBSCRIBE);
  disco->addFeature(XMLNS_PUBSUB_AUTO_CREATE);

  client->registerStanzaExtension(new VCardUpdate);
  client->registerStanzaExtension(new Capabilities);
  client->registerStanzaExtension(new Disco::Items);

  client->registerStanzaExtension(new Jingle::JingleMessage());
  client->registerStanzaExtension(new InBandBytestream::IBB);
  client->registerStanzaExtension(new ChatState(nullptr));
  client->registerStanzaExtension(new Receipt(nullptr));
  client->registerStanzaExtension(new Forward());
  client->registerStanzaExtension(new Carbons());
  client->registerStanzaExtension(new Attention());
  client->registerStanzaExtension(new DelayedDelivery());
  client->registerStanzaExtension(new ExtDisco());
  client->registerStanzaExtension(new Addresses());
  client->registerStanzaExtension(new Nickname(nullptr));
  client->registerStanzaExtension(new PubSub::Event());

  /**
   *
   */
  disco->addFeature(XMLNS_X_CONFERENCE);
  client->registerStanzaExtension(new Conference);

  /**
   * XEP-0402: PEP Native Bookmarks
   * urn:xmpp:bookmarks:1
   */
  disco->addFeature(XMLNS_BOOKMARKS2);
  disco->addFeature(XMLNS_BOOKMARKS2_NOTIFY);
  disco->addFeature(XMLNS_BOOKMARKS2_COMPAT);
  disco->addFeature("urn:xmpp:bookmarks-conversion:0");
  m_nativeBookmark = std::make_unique<NativeBookmarkStorage>(client.get());
  m_nativeBookmark->registerBookmarkHandler(this);

  for (const auto &feat : features) {
    qDebug() << "addFeature:" << feat;
    disco->addFeature(stdstring(feat));
  }
  // urn:xmpp:avatar:data
  disco->addFeature(XMLNS_AVATAR);
  // urn:xmpp:avatar:metadata
  disco->addFeature(XMLNS_META_AVATAR);
  // urn:xmpp:avatar:metadata+notify
  disco->addFeature(XMLNS_META_AVATAR + "+notify");
  client->registerStanzaExtension(new Avatar);

  disco->addFeature(XMLNS_CHAT_STATES);
  disco->addFeature(XMLNS_MUC);
  disco->addFeature(XMLNS_MUC_ADMIN);
  disco->addFeature(XMLNS_MUC_OWNER);
  disco->addFeature(XMLNS_MUC_ROOMS);
  disco->addFeature(XMLNS_MUC_ROOMINFO);
  disco->addFeature(XMLNS_MUC_USER);
  disco->addFeature(XMLNS_MUC_UNIQUE);
  disco->addFeature(XMLNS_MUC_REQUEST);

  disco->addFeature(XMLNS_DISCO_INFO);
  disco->addFeature(XMLNS_DISCO_ITEMS);
  disco->addFeature(XMLNS_DISCO_PUBLISH);
  disco->addFeature(XMLNS_CAPS);

  disco->addFeature(XMLNS_STANZA_FORWARDING);

  disco->addFeature(XMLNS_BOOKMARKS);
  disco->addFeature(XMLNS_PRIVATE_XML);
  // XMLNS_RECEIPTS
  disco->addFeature(XMLNS_RECEIPTS);
  disco->addFeature(XMLNS_MESSAGE_CARBONS);
  disco->addFeature("urn:xmpp:carbons:rules:0");
  disco->addFeature(XMLNS_ADDRESSES);

  // 基本Jingle功能
  disco->addFeature(XMLNS_IBB);
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
  // NICK
  disco->addFeature(XMLNS_NICKNAME);
  disco->addFeature(XMLNS_NICKNAME + "+notify");

  client->setCompression(false);
  client->registerIncomingHandler(this);
  client->registerIqHandler(this, ExtIBB);

  /**
   * listeners
   */
  client->registerConnectionListener(this);

  client->registerPresenceHandler(this);
  client->registerMessageHandler(this);
  client->registerMessageSessionHandler(this);

#ifdef LOG_XMPP
  client->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);
#endif

  vCardManager = std::make_unique<VCardManager>(client.get());
  pubSubManager = std::make_unique<PubSub::Manager>(client.get());
  bookmarkStorage = std::make_unique<gloox::BookmarkStorage>(client.get());
  return std::move(client);
}

void IM::stop() {
  if (_status != IMStatus::CONNECTED) {
    return;
  }
  doDisconnect();
  emit onStopped();
}

void IM::onConnect() {
  DEBUG_LOG(("connected"));
  assert(_client);

  _status = IMStatus::CONNECTED;

  auto res = _client->resource();
  DEBUG_LOG(("resource:%1").arg(qstring(res)));

  emit selfIdChanged(qstring(_client->username()));
  emit connectResult(_status);
  fetchVCard(qstring(self().bare()));
  //  emit onStarted();
}

void IM::doConnect() {
  DEBUG_LOG(("connecting ..."));
  if (_status == IMStatus::CONNECTED || _status == IMStatus::CONNECTING) {
    return;
  }
  // set status to connecting
  _status = IMStatus::CONNECTING;

  qDebug() << "Create IM client...";
  _client = makeClient();
  qDebug() << "Create IM client is:" << _client.get();
  _client->connect(true);
}

void IM::send(const QString &xml) {
  if (xml.isEmpty()) {
    qWarning() << "send xml content is empty";
    return;
  }
  _client->send(stdstring(xml));
}

void IM::interrupt() {
  _status = IMStatus::DISCONNECTED;
  emit connectResult(_status);
  if (_client) {
    _client->disconnect();
  }
}

gloox::JID IM::wrapRoomJid(const QString &group) const {
  return gloox::JID(group.toStdString() + "@" + _host);
}

gloox::JID IM::wrapJid(const QString &f) const {
  return gloox::JID(f.toStdString() + "@" + _host);
}

IMMessage IM::from(MsgType type, const gloox::Message &msg) {

  IMMessage imMsg(type,                       //
                  qstring(msg.from().full()), //
                  qstring(msg.body()));

  if (!msg.id().empty()) {
    imMsg.id = (qstring(msg.id()));
  }
  return imMsg;
}

void IM::enableDiscoManager() {
  qDebug() << "enableDiscoManager";

  auto disco = _client->disco();

  for (const auto &feat : features) {
    qDebug() << "addFeature:" << feat;
    disco->addFeature(stdstring(feat));
  }
  // urn:xmpp:avatar:data
  disco->addFeature(XMLNS_AVATAR);
  // urn:xmpp:avatar:metadata
  disco->addFeature(XMLNS_META_AVATAR);
  disco->addFeature(XMLNS_META_AVATAR + "+notify");

  disco->addFeature(XMLNS_CHAT_STATES);
  disco->addFeature(XMLNS_MUC);
  disco->addFeature(XMLNS_MUC_ADMIN);
  disco->addFeature(XMLNS_MUC_OWNER);
  disco->addFeature(XMLNS_MUC_ROOMS);
  disco->addFeature(XMLNS_MUC_ROOMINFO);
  disco->addFeature(XMLNS_MUC_USER);
  disco->addFeature(XMLNS_MUC_UNIQUE);
  disco->addFeature(XMLNS_MUC_REQUEST);

  disco->addFeature(XMLNS_DISCO_INFO);
  disco->addFeature(XMLNS_DISCO_ITEMS);
  disco->addFeature(XMLNS_DISCO_PUBLISH);
  disco->addFeature(XMLNS_CAPS);

  disco->addFeature(XMLNS_STANZA_FORWARDING);

  disco->addFeature(XMLNS_BOOKMARKS);
  disco->addFeature(XMLNS_PRIVATE_XML);
  // XMLNS_RECEIPTS
  disco->addFeature(XMLNS_RECEIPTS);
  disco->addFeature(XMLNS_MESSAGE_CARBONS);
  disco->addFeature("urn:xmpp:carbons:rules:0");
  disco->addFeature(XMLNS_ADDRESSES);

  // 基本Jingle功能
  disco->addFeature(XMLNS_IBB);
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
  // NICK
  disco->addFeature(XMLNS_NICKNAME);
  disco->addFeature(XMLNS_NICKNAME + "+notify");

  auto client = _client.get();

  client->registerStanzaExtension(new Avatar);
  client->registerStanzaExtension(new VCardUpdate);
  client->registerStanzaExtension(new Capabilities);
  client->registerStanzaExtension(new Disco::Items);

  client->registerStanzaExtension(new Jingle::JingleMessage());
  client->registerStanzaExtension(new InBandBytestream::IBB);
  client->registerStanzaExtension(new ChatState(nullptr));
  client->registerStanzaExtension(new Receipt(nullptr));
  client->registerStanzaExtension(new Forward());
  client->registerStanzaExtension(new Carbons());
  client->registerStanzaExtension(new Attention());
  client->registerStanzaExtension(new DelayedDelivery());
  client->registerStanzaExtension(new ExtDisco());
  client->registerStanzaExtension(new Addresses());
  client->registerStanzaExtension(new Nickname(nullptr));

  disco->registerDiscoHandler(this);
  disco->registerNodeHandler(this, EmptyString);
}

void IM::requestVCards() {
  /**
   * VCard
   */
  qDebug() << "requestVCards";
  vCardManager->fetchVCard(self().bareJID(), this);
}

void IM::enableRosterManager() {

  /**
   * extensions
   */
  auto client = _client.get();

  auto disco = _client->disco();

  /**
   * Registration
   */
  mRegistration = std::make_unique<Registration>(client);
  mRegistration->registerRegistrationHandler(this);

  auto pRosterManager = _client->enableRoster();
  pRosterManager->registerRosterListener(this);

  // enable carbons（多终端支持）
  IQ iq(IQ::IqType::Set, JID(), "server");
  iq.addExtension(new Carbons(gloox::Carbons::Enable));
  _client->send(iq);

  // request ext server disco
  IQ iq2(gloox::IQ::Get, JID(_host));
  auto t = iq2.tag();
  t->addChild(gloox::ExtDisco::newRequest());
  _client->send(t);
}

void IM::doDisconnect() {
  assert(_client.get());
  _client->disconnect();
}

QDomElement IM::buildMessage(const QString &to,  //
                             const QString &msg, //
                             QString &id) {

  std::string msgId = id.isEmpty() ? _client->getID() : id.toStdString();

  gloox::Message m(gloox::Message::MessageType::Chat, JID(stdstring(to)),
                   msg.toStdString());
  m.setFrom(_client->jid());
  m.setID(msgId);
  m.addExtension(new Receipt(Receipt::Request));

  id.clear();
  id.append(qstring(msgId));

  return ::base::Xmls::parse(qstring(m.tag()->xml()));
}

bool IM::sendTo(const QString &friendId, const QString &msg, QString &id) {
  qDebug() << "sendTo:" << friendId;
  qDebug() << "msg:" << msg;

  std::string msgId = id.isEmpty() ? _client->getID() : id.toStdString();
  qDebug() << "msgId:" << qstring(msgId);

  gloox::Message m(gloox::Message::MessageType::Chat, JID(stdstring(friendId)),
                   msg.toStdString());
  m.setFrom(_client->jid());
  m.setID(msgId);
  m.addExtension(new Receipt(Receipt::Request));

  id.clear();
  id.append(qstring(msgId));

  _client->send(m);
  return true;
}

void IM::makeId(QString &id) {
  if (id.isEmpty())
    id.append(qstring(_client->getID()));
}

// Handle Message session
void IM::handleMessageSession(MessageSession *session) {
  qDebug() << "Message session from:" << qstring(session->target().full());

  // 放入最新的session

  // m_messageEventFilter = std::make_unique<MessageEventFilter>(session);
  // m_messageEventFilter->registerMessageEventHandler(this);
  // m_pepEventFilter =
  // std::make_unique<PersonalEventingProtocolFilter>(session);

  // 注册类别
  //  m_pepEventFilter->registerPlugin(new SmartBoard::DrawLine());
  //  m_pepEventFilter->registerPlugin(new SmartBoard::DrawText());
  //  m_pepEventFilter->registerPlugin(new SmartBoard::DrawFile());
  //  m_pepEventFilter->registerPlugin(new SmartBoard::DrawMove());
  //  m_pepEventFilter->registerPlugin(new SmartBoard::DrawRemove());

  // m_pepEventFilter->registerPlugin(new SmartBoard::ControllerSelect());
  // m_pepEventFilter->registerPlugin(new SmartBoard::ControllerVoice());

  //    m_pepEventFilter->registerPersonalEventingProtocolHandler(this);
  //  auto jid = session->target();
  //  sessionMap.emplace(std::pair(jid.bare(), session));
  session->registerMessageHandler(this);

  // 聊天状态过滤器，获取：正在中等输入状态
  if (m_chatStateFilters.size() > 1000) {
    return;
  }
  auto csf = new ChatStateFilter(session);
  csf->registerChatStateHandler(this);
  m_chatStateFilters.emplace(session->target().bare(), csf);
  // TODO delete csf
}

void IM::handleMessage(const gloox::Message &msg, MessageSession *session) {
  if (!session) {
    DEBUG_LOG(("session is NULL"))
    return;
  }
  auto from = msg.from();
  auto threadId = qstring(session->threadID());
  auto peerId = qstring(from.full());
  auto friendId = qstring(from.bare());
  auto body = qstring(msg.body());

  DEBUG_LOG(("from:%1 msgType:%2").arg(peerId).arg((int)msg.subtype()));

  sessionIdMap.emplace(friendId.toStdString(), threadId.toStdString());
  sessionMap.emplace(threadId.toStdString(), session);

  gloox::Message::MessageType msgType = msg.subtype();
  switch (msgType) {
  case gloox::Message::Chat: {
    doMessageChat(msg, friendId, body);
    break;
  }
  case gloox::Message::Headline: {
    doMessageHeadline(msg, friendId, body);
    break;
  }
  case gloox::Message::Normal:
    // Normal
    doMessageNormal(msg, friendId);
    break;
  case gloox::Message::Groupchat:
    // ignore
    break;
  case gloox::Message::Error:
  case gloox::Message::Invalid: {
    // error
    break;
  }
  }

  auto conf = msg.findExtension<Conference>(ExtConference);
  if (conf) {
    auto jid = conf->jid();
    qDebug() << "Received Room" << qstring(jid.full());
    requestBookmarks();
  }

  auto chatState = msg.findExtension<ChatState>(ExtChatState);
  if (chatState) {
    handleChatState(msg.from(), chatState->state());
  }

  auto pEvent = msg.findExtension<PubSub::Event>(ExtPubSubEvent);
  if (pEvent) {
    doPubSubEvent(pEvent, msg, friendId);
    // msleep(100);
    emit doPubSubEventDone();
  }

  auto delay = msg.when();
  if (delay) {
    qDebug() << "delay:" << qstring(delay->stamp());
  }

  auto address = msg.address();
  if (address) {
    for (const auto &item : address->addresses()) {
      qDebug() << "address type:" << qstring(item.type)
               << "jid:" << qstring(item.jid.full());
    }
  }

  /**
   * 处理jingle-message消息
   * https://xmpp.org/extensions/xep-0353.html
   */
  auto jm = msg.findExtension<Jingle::JingleMessage>(ExtJingleMessage);
  if (jm) {
    doJingleMessage(PeerId(msg.from().full()), jm);
  }
}

void IM::doMessageHeadline(const Message &msg, QString &friendId,
                           const QString &body) {

  auto mu = msg.findExtension<MUCRoom::MUCUser>(ExtMUCUser);
  if (mu) {
    // 群聊邀请
    if (MUCRoom::OpInviteFrom == mu->operation()) {
      emit groupInvite(qstring(JID(*mu->jid()).username()), friendId, body);
    }
  }
}

/**
 * 处理聊天消息
 *
 */
void IM::doMessageChat(const Message &msg, QString &friendId,
                       const QString &body) {
  qDebug() << "doMessageChat from:" << friendId;
  if (!body.isEmpty()) {
    if (!msg.encrypted()) {
      qDebug() << "body:" << body;
      IMMessage imMsg = from(MsgType::Chat, msg);
      emit receiveFriendMessage(friendId, imMsg);
    } else {
      qDebug() << "encrypted message.";
      QString xml = qstring(msg.tag()->xml());
      QDomElement dom = ::base::Xmls::parse(xml);
      emit exportEncryptedMessage(dom);
    }
  }
  // 从 message.receipt 提取接收确认ID
  auto pReceipt = msg.findExtension<Receipt>(ExtReceipt);
  if (pReceipt && !pReceipt->id().empty()) {
    emit receiveMessageReceipt(friendId, qstring(pReceipt->id()));
  }
  // forwarded/message
  auto pCarbons = msg.findExtension<Carbons>(ExtCarbons);
  if (pCarbons && pCarbons->embeddedStanza()) {
    qDebug() << "Parse carbon message";
    Message *eMsg = static_cast<Message *>(pCarbons->embeddedStanza());
    if (eMsg) {
      auto ebody = eMsg->body();
      if (!ebody.empty()) {
        emit receiveFriendMessage(qstring(eMsg->to().full()),
                                  IMMessage(MsgType::Chat,
                                            qstring(eMsg->from().full()),
                                            qstring(ebody), ""));
      }
    }
  }
}

/**
 * 处理常规消息
 * @param msg
 * @param friendId
 */
void IM::doMessageNormal(const Message &msg, QString &friendId) {
  qDebug() << "doMessageNormal from:" << friendId;
  auto conf = msg.findExtension<Conference>(ExtConference);
  if (conf) {
    auto jid = conf->jid().bare();
    qDebug() << "New conference is arrival" << qstring(jid);
    onAddRoom(jid);
  }
}

void IM::handleMessageEvent(const JID &from, const MessageEvent *et) {
  qDebug() << ("JID:") << qstring(from.full()) << " MessageEvent:%2" << et;
}

void IM::doPubSubEvent(const gloox::PubSub::Event *pse, //
                       const Message &msg,              //
                       QString &friendId) {
  qDebug() << "doPubSubEvent:" << friendId;
  qDebug() << "selfId:" << getSelfId().toString();

  auto isSelf = friendId == getSelfId().toString();

  for (auto &item : pse->items()) {
    qDebug() << "item:" << qstring(item->item);
    auto nickTag = item->payload->findChild("nick");
    if (nickTag) {
      Nickname nickname(nickTag);
      auto newNick = qstring(nickname.nick());
      if (isSelf && _nick != newNick) {
        _nick = newNick;
        qDebug() << "Received self nick:" << _nick;
        emit selfNicknameChanged(newNick);
      } else {
        qDebug() << "Received friend:" << friendId << "nick:" << newNick;
        emit receiveNicknameChange(friendId, newNick);
      }
    }
    auto avatarData = item->payload->findChild("data", XMLNS, XMLNS_AVATAR);
    if (avatarData) {
      auto base64 = avatarData->cdata();
      if (!base64.empty()) {
        std::string::size_type pos = 0;
        while ((pos = base64.find('\n')) != std::string::npos)
          base64.erase(pos, 1);
        while ((pos = base64.find('\r')) != std::string::npos)
          base64.erase(pos, 1);

        auto avt = Base64::decode64(base64);
        if (isSelf) {
          qDebug() << "Receive self avatar size" << avt.size();
          emit selfAvatarChanged(avt);
        } else {
          qDebug() << "Receive friend avatar" << friendId << "size"
                   << avt.size();
          emit receiveFriendAvatarChanged(friendId, avt);
        }
      }
    }

    auto avatarMetaData = item->payload->findChild("metadata", //
                                                   XMLNS,      //
                                                   XMLNS_META_AVATAR);
    if (avatarMetaData) {

      auto itemId = avatarMetaData->findChild("info")->findAttribute("id");

      ItemList items;
      Item *item0 = new Item();
      item0->setID(itemId);
      items.emplace_back(item0);

      if (!pubSubManager.get()) {
        qWarning() << "pubSubManager is null";
        return;
      }
      pubSubManager->requestItems(msg.from(), XMLNS_AVATAR, "", items, this);
    }

    auto devices = item->payload->findChild("list", //
                                            XMLNS,  //
                                            "eu.siacs.conversations.axolotl");
    if (devices) {
      for (auto dev : devices->findChildren("device")) {
        qDebug() << "device id:" << qstring(dev->findAttribute("id"));
      }
    }
  }
}

/**
 * 接收来自朋友聊天状态
 * @breif http://jabber.org/protocol/chatstates
 * @param from
 * @param state
 */
void IM::handleChatState(const JID &from, ChatStateType state) {
  DEBUG_LOG(("from:%1 state:%2")
                .arg(qstring(from.full()))
                .arg(static_cast<int>(state)));

  auto friendId = qstring(from.bare());
  emit receiveFriendChatState(friendId, state);
}

/**
 * 发送给朋友当前聊天状态
 * @param to
 * @param state 聊天状态
 */
void IM::sendChatState(const QString &to, ChatStateType state) {
  DEBUG_LOG(("to:%1 state:%2") //
                .arg((to))     //
                .arg(static_cast<int>(state)));
  auto csf = m_chatStateFilters[stdstring(to)];
  if (!csf) {
    return;
  }
  csf->setChatState(state);
}

#ifdef false
/**
 * 加入房间事件
 * @brief IM::onJoinedRoom
 * @param roomJid
 * @param userJID
 */
void IM::onJoinedRoom(const JID &roomJid, const UserJID &userJID) {

  DEBUG_LOG(("roomJid:%1 userJid:%2")         //
                .arg(qstring(roomJid.full())) //
                .arg((userJID.id))            //
  );

  std::lock_guard<std::mutex> lock(_join_room_mutex);

  //  if (!_joined_room) {
  // 加入会议室，改变成会员角色
  //         if(_userManager->isMember()){
  //             setAffiliation(userJID,
  //             MUCRoomAffiliation::AffiliationMember); setRole(userJID,
  //             MUCRoomRole::RoleVisitor);
  //         }else{
  //             setAffiliation(userJID,
  //             MUCRoomAffiliation::AffiliationOwner); setRole(userJID,
  //             MUCRoomRole::RoleModerator);
  //         }

  // Jingle
  // _jingle->join(roomJid);
  // SmartBoard
  // DEBUG_LOG(("end for:%1").arg(qstring(roomJid.full())));

  //    _joined_room = true;
  //  }
}
#endif

// MUC handler -- handleMUCParticipantPresence
void IM::handleMUCParticipantPresence(gloox::MUCRoom *room,                 //
                                      const MUCRoomParticipant participant, //
                                      const Presence &presence) {

  qDebug() << "handleMUCParticipantPresence" << qstring(room->jid().full())
           << "nick" << participant.nick->resource().c_str();

  auto mucUser = presence.tag()->findChild("x", XMLNS, XMLNS_MUC_USER);
  if (mucUser) {
    //<x xmlns='http://jabber.org/protocol/muc#user'><item affiliation='none'
    // role='none'/></x>
    auto item = mucUser->findChild("item");
    if (item)
      qDebug() << "affiliation: " << qstring(item->findAttribute("affiliation"))
               << "role: " << qstring(item->findAttribute("role"));
  }

#if 0
  JID *nick = participant.nick;
  DEBUG_LOG(("participant:%1 presence=>%2") //
                .arg(qstring(nick->resource()))
                .arg(presence.presence()));

//TODO 此处导致界面卡死，暂时注释
  switch (presence.presence()) {
  case Presence::Unavailable:
  case Presence::Invalid:
  case Presence::Error: {
    // 非在线
    emit groupOccupantStatus(roomName, peerId, false);
    break;
  }
  default: {
    // 在线
    emit groupOccupantStatus(roomName, peerId, true);
    break;
  }
  }
#endif
}

void IM::handleMUCMessage(MUCRoom *room, const gloox::Message &msg, bool priv) {
  auto body = qstring(msg.body());
  auto msgId = qstring(msg.id());
  auto roomId = qstring(room->jid().full());

  PeerId peerId(msg.from());
  qDebug() << "room:" << qstring(room->name());

  DEBUG_LOG(("roomId:%1 from:%2 msgId:%3") //
                .arg(roomId)               // room
                .arg(peerId.toString())    // from
                .arg(msgId)                // msg id
  );

  qDebug() << "msg:" << body;

  for (const auto &id : sendIds) {
    if (id == msg.id()) {
      qDebug() << "自己发出消息，忽略！";
      sendIds.remove(id);
      return;
    }
  }

  auto mu = msg.findExtension<MUCRoom::MUCUser>(ExtMUCUser);
  if (mu) {
    if (mu->flags() & UserRoomConfigurationChanged) {
      room->getRoomInfo();
    }

    /**
     * <message from='test8@conference.meet.chuanshaninfo.com'
     * to='18510248810@meet.chuanshaninfo.com'>
     * <x xmlns='http://jabber.org/protocol/muc#user'>
     *    <invite from='test8@conference.meet.chuanshaninfo.com/高杰2395'>
     *        <reason/>
     *    </invite>
     * </x>
     * <x jid='test8@conference.meet.chuanshaninfo.com'
     * xmlns='jabber:x:conference'/>
     * <body>test8@conference.meet.chuanshaninfo.com/高杰2395
     * invited you to the room test8@conference.meet.chuanshaninfo.com
     * </body>
     * </message>
     */
    if (MUCRoom::OpInviteFrom == mu->operation()) {
      emit groupInvite(qstring(JID(*mu->jid()).bare()), roomId,
                       qstring(msg.body()));
    }

    return;
  }

  if (body.isEmpty()) {
    return;
  }

  IMMessage imMsg = from(MsgType::Groupchat, msg);
  const DelayedDelivery *dd = msg.when();
  if (dd) {
    // yyyy-MM-dd HH:mm:ss 20230614T12:11:43Z
    imMsg.time =
        QDateTime::fromString(qstring(dd->stamp()), "yyyy-MM-ddTHH:mm:ssZ");
  }

  auto addresses = msg.address();
  if (addresses) {
    for (auto &item : addresses->addresses()) {
      // 群消息来源辨别多终端
      if (item.type == "ofrom") {
        imMsg.from = qstring(item.jid.full());
      }
    }
  }

  emit receiveRoomMessage(roomId, peerId, imMsg);
}

bool IM::handleMUCRoomCreation(MUCRoom *room) {
  qDebug() << "handleMUCRoomCreation" << room->jid().full().c_str();

  //  room->requestRoomConfig();

  // 添加到缓存
  auto roomId = qstring(room->jid().bare());
  m_roomMap.insert(roomId, IMRoomInfo{room, {}});

  // 群聊增加
  emit groupListReceived(roomId, qstring(room->name()));

  msleep(100);
  joinRoom(roomId);

  //  bookmarkStorage->requestBookmarks();
  return true;
}

void IM::handleMUCSubject(MUCRoom *room,           //
                          const std::string &nick, //
                          const std::string &subject) {
  DEBUG_LOG(("MUCRoom name:%1").arg(qstring(room->name())));
  DEBUG_LOG(("nick:%1").arg(qstring(nick)));
  DEBUG_LOG(("subject:%2").arg(qstring(subject)));
  room->getRoomInfo();
}

void IM::handleMUCInviteDecline(MUCRoom *room, const JID &invitee,
                                const std::string &reason) {
  DEBUG_LOG(("MUCRoom:%1").arg(qstring(room->name())));
  DEBUG_LOG(("invitee:%1 reason:%2")
                .arg(qstring(invitee.full()))
                .arg(qstring(reason)));
}

void IM::handleMUCError(MUCRoom *room, StanzaError error) {
  DEBUG_LOG(("MUCRoom:%1 error:%2")
                .arg(qstring(room->name()))
                .arg(static_cast<int>(error)));
}

void IM::handleMUCInfo(MUCRoom *room,              //
                       int features,               //
                       const std::string &name,    //
                       const DataForm *infoForm) { //

  auto roomId = qstring(room->jid().full());
  auto roomName = qstring(name);

  qDebug() << "handleMUCInfo" << roomName;

  GroupInfo groupInfo;
  groupInfo.name = name;

  if (infoForm) {
    auto info = const_cast<IMRoomInfo *>(findRoom(roomId));

    for (auto field : infoForm->fields()) {
      qDebug() << "field name:" << qstring(field->name())
               << qstring(field->value());

      if (field->name() == "muc#roominfo_occupants") {
        groupInfo.occupants = std::stoi(field->value());
      } else if (field->name() == "muc#roominfo_roomname") {
        groupInfo.name = (field->value());
      } else if (field->name() == "muc#roominfo_subject") {
        groupInfo.subject = field->value();
      } else if (field->name() == "muc#roominfo_description") {
        groupInfo.description = field->value();
      } else if (field->name() == "muc#roominfo_creationdate") {
        groupInfo.creationdate = field->value();
      }
    }

    if (info->name.empty()) {
      info->info.name = name;
    }

    info->info = groupInfo;
  }

  emit groupRoomInfo(roomId, groupInfo);
}

void IM::handleMUCItems(MUCRoom *room, const Disco::ItemList &items) {
  DEBUG_LOG(("MUCRoom:%1").arg(qstring(room->name())));
  DEBUG_LOG(("items:%1").arg(items.size()));
}

// config
void IM::handleMUCConfigList(MUCRoom *room,                //
                             const MUCListItemList &items, //
                             MUCOperation operation) {
  DEBUG_LOG(("IM::handleMUCConfigList"));
}

void IM::handleMUCConfigForm(MUCRoom *room, const DataForm &form) {
  qDebug() << "handleMUCConfigForm room:" << room->jid().full().c_str();

  auto roomId = qstring(room->jid().full());

  auto find = m_roomMap.find(roomId);
  if (find == m_roomMap.end()) {
    qWarning() << "Unable find room from cache" << roomId;
    return;
  }

  /**
   * 将缓存的房间配置放入`DataForm`
   */
  int changed = 0;
  auto &roomInfo = find.value();
  for (auto item : form.fields()) {
    for (const auto &change : roomInfo.changes) {
      if (item->name() == change.first) {
        item->setValue(change.second);
        changed += 1;
        break;
      }
    }
  }

  if (changed) {

    // 清空修改信息
    roomInfo.changes.clear();

    auto *mform = new DataForm(form);
    mform->setType(FormType::TypeSubmit);
    room->setRoomConfig(mform);
    qDebug() << "Update room config items" << changed;
  }
};

void IM::handleMUCConfigResult(MUCRoom *room, bool success,
                               MUCOperation operation) {
  qDebug() << "handleMUCConfigResult room" << qstring(room->jid().full())
           << "success:" << success << "operation:" << operation;
};

void IM::handleMUCRequest(MUCRoom *room, const DataForm &form) {
  qDebug() << "handleMUCRequest room" << qstring(room->jid().full());
  for (const auto &item : form.items()) {
    qDebug() << "item" << item;
    for (const auto &item_j : item->fields()) {
      qDebug() << "field:" << item_j->name().c_str() << "=>"
               << item_j->value().c_str();
    }
  }
};

/**
 * 初始化房间并加入
 */
void IM::createRoom(const JID &jid, const std::string &password) {

  auto nick = (getNickname());
  qDebug() << "createRoom"                  //
           << "jid:" << qstring(jid.full()) //
           << "nick:" << nick;              //

  JID &roomJid = const_cast<JID &>(jid);
  roomJid.setResource(stdstring(nick));

  auto roomId = qstring(roomJid.bare());
  qDebug() << ("roomId:") << (roomId);

  auto room = new MUCRoom(_client.get(), roomJid, this, this);
  room->join();
  room->instantRoom(MUCOperation::CreateInstantRoom);
  qDebug() << "Creating the room" << roomId;

  ConferenceListItem item;
  item.name = room->name();
  item.jid = room->jid().full();
  item.autojoin = true;
  item.nick = stdstring(getNickname());
  // 添加到书签列表
  mConferenceList.emplace_back(item);
  // 存储书签列表
  bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);
  qDebug() << "Store bookmarks is successful for room" << roomId;
}

/**
 * 要求成员加入房间
 * @param roomJid
 * @param peerId
 */
bool IM::inviteToRoom(const JID &roomJid, const JID &peerId) {
  auto roomInfo = findRoom(qstring(roomJid.bare()));
  if (!roomInfo) {
    return false;
  }

  roomInfo->room->invite(peerId, "--");
  return true;
}

const IMRoomInfo *IM::findRoom(const QString &groupId) const {
  auto it = m_roomMap.find(groupId);
  if (it == m_roomMap.end()) {
    qWarning() << "Unable find room" << groupId;
    return nullptr;
  }
  return &(it.value());
}

void IM::joinRoom(MUCRoom *room) {
  room->setNick(stdstring(getNickname()));
  room->join();
  room->getRoomInfo();
  _client->disco()->getDiscoItems(room->jid(), XMLNS_BOOKMARKS, this,
                                  DISCO_CTX_BOOKMARKS);
}

void IM::onAddRoom(const std::string &jid, const std::string &name) {
  qDebug() << "onAddRoom" << qstring(jid);

  JID roomJid(jid);
  auto roomId = qstring(roomJid.bare());
  auto room = new MUCRoom(_client.get(), roomJid, this, this);
  m_roomMap.insert(roomId, IMRoomInfo{room, {}});

  qDebug() << "emit room:" << roomId;
  emit groupListReceived(roomId, name.empty() ? qstring(roomJid.username())
                                              : qstring(name));
  msleep(100);
  joinRoom(room);
}

void IM::joinRoom(const QString &roomJid) {
  qDebug() << "joinRoom" << roomJid;
  qDebug() << "nick" << getNickname();

  auto it = m_roomMap.find(roomJid);
  if (it == m_roomMap.end()) {
    qWarning() << "Unable find room from cache" << roomJid;
    return;
  }

  joinRoom(it.value().room);
}

void IM::joinRooms() {
  qDebug() << "joinRooms";
  for (const auto &item : m_roomMap) {
    joinRoom(item.room);
  }
}

bool IM::sendToRoom(const QString &to, const QString &msg, const QString &id) {
  qDebug() << "sendToRoom=>" << to;
  qDebug() << "msg:" << msg;

  auto pRoomInfo = findRoom((to));
  if (!pRoomInfo) {
    qDebug() << "The room is not exist!";
    return false;
  }

  std::string msgId = !id.isEmpty() ? id.toStdString() : getClient()->getID();
  qDebug() << "msgId:" << qstring(msgId);
  sendIds.emplace_back(msgId);

  pRoomInfo->room->send(msg.toStdString());
  return true;
}

void IM::setRoomSubject(const QString &groupId, const std::string &nick) {
  auto room = findRoom(groupId);
  if (room) {
    room->room->setSubject(nick);
  }
}

void IM::setRoomName(const QString &groupId, const std::string &roomName) {
  qDebug() << "setRoomName" << groupId << roomName.c_str();
  const IMRoomInfo *pRoomInfo = findRoom(groupId);
  if (!pRoomInfo) {
    qDebug() << "room is not exist." << groupId;
    return;
  }

  auto info = const_cast<IMRoomInfo *>(pRoomInfo);
  info->changes.insert(std::make_pair("muc#roomconfig_roomname", roomName));
  info->name = roomName;
  // 获取新配置（handleMUCConfigForm处理）
  info->room->requestRoomConfig();

  //  ConferenceListItem item;
  //  item.name = room->name();
  //  item.jid = room->jid().full();
  //  item.autojoin = true;
  //  item.nick = stdstring(getNickname());
  bool update = false;
  // 添加到书签列表
  for (auto &item : mConferenceList) {
    if (item.jid == groupId.toStdString()) {
      item.name = roomName;
      update = true;
      break;
    }
  }

  // 存储书签列表
  if (update) {
    bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);
    qDebug() << "Store bookmarks is successful for room" << groupId;
  }
}

#ifdef WANT_PING
void IM::handlePing(const gloox::PingHandler::PingType type,
                    const std::string &body) {
  qDebug() << "ping" << type << qstring(body);
  if (type != websocketPong) {
    IQ iq(IQ::IqType::Result, JID(_host));
    _client->send(iq);
  }
}
#endif

/**
 * 处理个人信息（VCard）
 * @param jid
 * @param vcard
 */
void IM::handleVCard(const JID &jid, const VCard *vcard) {
  DEBUG_LOG(("jid：%1").arg(qstring(jid.full())));

  VCard::Photo photo = vcard->photo();
  if (!photo.binval.empty()) {
    DEBUG_LOG(("photo binval size:%1").arg(photo.binval.size()));
    emit receiveFriendAvatarChanged(qstring(jid.bare()), photo.binval);
  }

  auto &nickname = vcard->nickname();
  if (!nickname.empty()) {
    DEBUG_LOG(("nickname:%1").arg(qstring(nickname)));
    emit receiveNicknameChange(qstring(jid.bare()), qstring(nickname));
  }
}

void IM::handleVCardResult(VCardContext context, const JID &jid,
                           StanzaError error) {
  DEBUG_LOG(("context:%1 jid:%2").arg(context).arg(qstring(jid.full())));
  if (error) {
    return;
  }
}

void IM::fetchVCard(const QString &friendId) {
  // 获取联系人个人信息
  qDebug() << "fetchVCard" << friendId;
  JID jid(stdstring(friendId));
  vCardManager->fetchVCard(jid, this);
  pubSubManager->subscribe(jid, "", this);
  //  _client->rosterManager()->subscribe(jid.bareJID());
}

void IM::handleTag(Tag *tag) {
  DEBUG_LOG(("tag：%1").arg(qstring(tag->xml())));
}

bool IM::handleIq(const IQ &iq) {
  qDebug() << "iq" << qstring(iq.xmlLang());

  FriendId friendId(iq.from());
  const auto *ibb = iq.findExtension<InBandBytestream::IBB>(ExtIBB);
  if (ibb) {
    DEBUG_LOG(("ibb流:%1").arg(qstring(ibb->sid())));

    switch (ibb->type()) {
    case InBandBytestream::IBBOpen: {
      DEBUG_LOG(("Open"))
      break;
    }
    case InBandBytestream::IBBData: {
      DEBUG_LOG(("Data seq:%1").arg(ibb->seq()))
      emit receiveFileChunk(friendId, qstring(ibb->sid()), ibb->seq(),
                            ibb->data());
      break;
    }
    case InBandBytestream::IBBClose: {
      DEBUG_LOG(("Close"))
      emit receiveFileFinished(friendId, qstring(ibb->sid()));
      break;
    }
    default: {
    }
    }

    IQ riq(IQ::IqType::Result, iq.from(), iq.id());
    _client->send(riq);
  }
  return true;
}

void IM::handleIqID(const IQ &iq, int context) {}

void IM::requestBookmarks() {
  /**
   * bookmark
   */
  bookmarkStorage->registerBookmarkHandler(this);
  bookmarkStorage->requestBookmarks();
}

void IM::handleBookmarks(const BookmarkList &bList,   //
                         const ConferenceList &cList) //
{
  qDebug() << "handleBookmarks...";
  qDebug() << "ConferenceList:" << cList.size();
  qDebug() << "BookmarkList:" << bList.size();

  //  缓存群聊书签列表（新增加群聊加入该书签一起保存）
  mConferenceList = cList;
  mBookmarkList = bList;

  for (auto &c : cList) {
    auto name = (!c.name.empty() ? c.name : JID(c.jid).username());
    qDebug() << "room:" << qstring(name) << "jid:" << qstring(c.jid)
             << "nick:" << qstring(c.nick) << "autojoin:" << c.autojoin;

    // 缓存到本地
    onAddRoom(c.jid, name);
  }

  //  joinRooms();
  emit groupListReceivedDone();
}

void IM::handleBookmarks(const BMConferenceList &cList) {
  qDebug() << "handleBookmarks";
  for (const auto &conf : cList) {
    qDebug() << "conference:" << qstring(conf.jid);
  }
}

// Disco handler
void IM::handleDiscoInfo(const JID &from,         //
                         const Disco::Info &info, //
                         int context) {

  QString _from = QString::fromStdString(from.full());
  DEBUG_LOG(("from=%1 context=%2").arg(_from).arg(context));

  const StringList features = info.features();
  for (auto feature : features) {
    DEBUG_LOG(("feature=%1").arg(QString::fromStdString(feature)));
  }

  const Disco::IdentityList &identities = info.identities();
  for (auto identity : identities) {
    DEBUG_LOG(("identity=%1").arg(qstring(identity->name())));
  }
}

/**
 * 获取服务发现
 * @param from
 * @param items
 * @param context
 */
void IM::handleDiscoItems(const JID &from,           //
                          const Disco::Items &items, //
                          int context) {

  QString _from = QString::fromStdString(from.full());
  DEBUG_LOG(("from=%1 context=%2").arg(_from).arg(context));

  const Disco::ItemList &localItems = items.items();
  for (auto item : localItems) {

    if (context == DISCO_CTX_ROSTER) {
      pubSubManager->subscribe(item->jid(), XMLNS_NICKNAME, this);
#ifndef Q_OS_WIN
      pubSubManager->subscribe(item->jid(), XMLNS_AVATAR, this);
#endif // !Q_OS_WIN
    }

    if (context == DISCO_CTX_CONF) {
      /**
       * 处理服务发现的群聊列表
       */
      auto roomId = qstring(item->jid().bare());
      auto name = qstring(item->name());
      qDebug() << "room:" << name << roomId;
      //      if (m_roomMap.find(roomId) == m_roomMap.end()) {
      //
      //        auto room = new MUCRoom(_client.get(), item->jid(), this, this);
      //        m_roomMap.insert(roomId, room);
      //
      //        qDebug() << "emit room:" << name;
      //        emit groupListReceived(roomId, name);
      //        msleep(100);
      //      }
    }
  }

  if (context == DISCO_CTX_CONF) {
    // 4-加入群聊
    //    joinRooms();
    //    emit groupListReceivedDone();
  }
}

void IM::handleDiscoError(const JID &from,           //
                          const gloox::Error *error, //
                          int context) {

  QString _from = qstring(from.full());
  DEBUG_LOG(("from=%1 context=%2 error=%3")
                .arg(_from)
                .arg(context)
                .arg(qstring(error->text())));
}

// Presence Handler
void IM::handlePresence(const Presence &presence) {
  qDebug() << "handlePresence from:" << qstring(presence.from().full())
           << "presence" << presence.presence();

  updateOnlineStatus(presence.from().bare(), presence.from().resource(),
                     presence.presence());
}

/**
 * Roster
 */
/**
 * 好友本地增加事件
 *
 * @param jid
 * @return
 */
void IM::handleItemAdded(const gloox::JID &jid) {
  qDebug() << qstring(jid.full());

  auto rosterManager = _client->rosterManager();
  /**
   * 订阅好友
   */
  if (m_addFriendMsg.isEmpty()) {
    rosterManager->subscribe(jid);
  } else {
    rosterManager->subscribe(jid, jid.username(), StringList(),
                             stdstring(m_addFriendMsg));
  }
}

/**
 * 好友被删除事件
 * @param jid
 */
void IM::handleItemRemoved(const JID &jid) {
  /**
   * subscription='remove'
   * TODO 需要通知到页面
   */
  DEBUG_LOG(("已删除好友:%1").arg(qstring(jid.full())));
  emit receiveFriendRemoved(qstring(jid.username()));
}

// 好友更新
void IM::handleItemUpdated(const JID &jid) {
  DEBUG_LOG(("jid: %1").arg(qstring(jid.full())));

  auto userId = qstring(jid.username());

  auto item = _client->rosterManager()->getRosterItem(jid);
  auto subType = item->subscription();
  DEBUG_LOG(("type: %1").arg(subType));

  switch (subType) {

  case gloox::S10nNone: {
    DEBUG_LOG(("none"))
    //    _client->rosterManager()->subscribe(jid);
    break;
  }
  case gloox::S10nNoneOut: {
    DEBUG_LOG(("已发送订阅请求，等待对方确认！"))
    break;
  }
  case gloox::S10nNoneIn: {
    DEBUG_LOG(("收到对方订阅请求，等待自己确认！"))
    break;
  }
  case S10nNoneOutIn: {
    DEBUG_LOG(("双方发起订阅，等待双方接受！"))
    break;
  }
  case gloox::S10nTo: {
    DEBUG_LOG(("已订阅对方，等待对方接受！"))
    // 加到联系人列表
    emit receiveFriend(qstring(jid.bare()));
    break;
  }
  case gloox::S10nToIn: {
    DEBUG_LOG(("已订阅对方，对方接受订阅，等待自己确认！"))
    //    _client->rosterManager()->subscribe(jid);
    break;
  }
  case gloox::S10nFrom: {
    DEBUG_LOG(("对方已订阅自己！"))
    break;
  }
  case gloox::S10nFromOut: {
    DEBUG_LOG(("对方已订阅自己，已接受对方订阅，待自己确认！"))
    break;
  }
  case gloox::S10nBoth: {
    DEBUG_LOG(("互相订阅成功！"))
    emit receiveFriend(qstring(jid.bare()));
    break;
  }
  }
}

/**
 * 订阅好友
 * @param jid
 */
void IM::handleItemSubscribed(const JID &jid) {
  DEBUG_LOG(("jid:%1").arg(qstring(jid.full())));
}

/**
 * 取消订阅好友
 * @param jid
 */
void IM::handleItemUnsubscribed(const JID &jid) {
  DEBUG_LOG(("jid:%1").arg(qstring(jid.full())));
}

void IM::updateOnlineStatus(const std::string &bare,
                            const std::string &resource,
                            Presence::PresenceType presenceType) {

  if (resource.empty()) {
    qWarning() << "resource is empty.";
    return;
  }

  auto friendId = qstring(bare);
  int status = -1;

  auto it = onlineMap.find(bare);
  if (it == onlineMap.end()) { // 第一次
    if (presenceType != gloox::Presence::Unavailable) {
      std::set<std::string> resources;
      resources.insert(resource);
      onlineMap.emplace(bare, resources);
      status = gloox::Presence::Available;
    }
  } else { // 第二次+
    std::set<std::string> &resources = it->second;
    if (presenceType != gloox::Presence::Unavailable) {
      // multi online endpoint
      resources.insert(resource);
      onlineMap.emplace(bare, resources);
      status = gloox::Presence::Available;
    } else {
      // one offline
      resources.erase(resource);
      if (resources.empty()) {
        // all offline
        onlineMap.erase(bare);
        status = gloox::Presence::Unavailable;
      }
    }
  }
  emit receiveFriendStatus(friendId, status);
}

bool IM::removeFriend(JID jid) {
  qDebug() << "removeFriend:" << qstring(jid.full());
  _client->rosterManager()->remove(jid);
  return true;
}

void IM::addRosterItem(const QString &username, const QString &nick,
                       const QString &msg) {
  qDebug() << "addRosterItem" << username << nick << msg;
  m_addFriendMsg = msg;
  StringList group;
  _client->rosterManager()->add(wrapJid(username), stdstring(nick), group);
}

void IM::acceptFriendRequest(const QString &friendId) {
  DEBUG_LOG(("friend:%1").arg(friendId))
  _client->rosterManager()->ackSubscriptionRequest(wrapJid(friendId), true);
}

void IM::rejectFriendRequest(const QString &friendId) {
  DEBUG_LOG(("friend:%1").arg(friendId))
  _client->rosterManager()->ackSubscriptionRequest(wrapJid(friendId), false);
}

size_t IM::getRosterCount() {
  return _client->rosterManager()->roster()->size();
}

void IM::getRosterList(std::list<FriendId> &list) {
  auto rosterManager = _client->rosterManager();
  gloox::Roster *rosterMap = rosterManager->roster();
  for (const auto &itr : *rosterMap) {
    auto jid = itr.second;
    FriendId peerId(jid->jid());
    list.push_back(peerId);
  }
}

void IM::handleRoster(const Roster &roster) {
  DEBUG_LOG(("size:%1").arg(roster.size()));

  for (auto &it : roster) {
    auto &key = it.first;
    auto &jid = it.second->jid();

    qDebug() << "roster" << jid.full().c_str();
    emit receiveFriend(qstring(jid.bare()));
    msleep(100);
  }
  //  enableDiscoManager();

  loadGroupList();

  emit receiveFriendDone();
};

/**
 * 接收联系人状态信息（授予订阅请求）
 * @param item 联系人
 * @param resource 终端
 * @param presenceType
 * @param msg
 * @return
 */
void IM::handleRosterPresence(const RosterItem &item,              //
                              const std::string &resource,         //
                              Presence::PresenceType presenceType, //
                              const std::string &msg) {

  DEBUG_LOG(("item:%1 resource:%2 presenceType:%3 msg:%4")
                .arg(qstring(item.jid().full()))
                .arg(qstring(resource))
                .arg(presenceType)
                .arg(qstring(msg)));

  updateOnlineStatus(item.jid().bare(), resource, presenceType);

  //  if (presenceType == gloox::Presence::Available) {
  for (auto &it : item.resources()) {
    auto sk = it.first;
    auto sr = it.second;
    for (auto &ext : sr->extensions()) {
      switch (ext->extensionType()) {
        //        case ExtCaps: {
        //          auto caps = const_cast<Capabilities *>(
        //              static_cast<const Capabilities *>(ext));
        //          DEBUG_LOG(("caps:%1").arg(qstring(caps->node())))
        //          break;
        //        }
      case ExtVCardUpdate: {
        // VCard个人信息更新
        /**
         * <pubsub xmlns='http://jabber.org/protocol/pubsub'>
        <items node='urn:xmpp:avatar:data'>
          <item id='111f4b3c50d7b0df729d299bc6f8e9ef9066971f'/>
        </items>
        </pubsub>
         */
        auto vCardUpdate =
            const_cast<VCardUpdate *>(static_cast<const VCardUpdate *>(ext));
        if (vCardUpdate && vCardUpdate->hasPhoto()) {

          ItemList items;
          Item *item0 = new Item();
          item0->setID(vCardUpdate->hash());
          items.emplace_back(item0);

          pubSubManager->requestItems(item.jid(), XMLNS_AVATAR, "", items,
                                      this);
        }
        break;
      }
      }
    }
  }
  //  }
}

void IM::handleSelfPresence(const RosterItem &item,              //
                            const std::string &resource,         //
                            Presence::PresenceType presenceType, //
                            const std::string &msg) {

  DEBUG_LOG(("item:%1 resource:%2 presenceType:%3 msg:%4")
                .arg(qstring(item.jid().full()))
                .arg(qstring(resource))
                .arg(presenceType)
                .arg(qstring(msg)));

  selfPresType = presenceType;

  emit selfStatusChanged(_client->resource() == resource
                             ? selfPresType
                             : (int)gloox::Presence::Available,
                         msg);

  //  if (presenceType == gloox::Presence::Available) {
  for (auto &it : item.resources()) {
    auto sk = it.first;
    auto sr = it.second;
    for (auto &ext : sr->extensions()) {
      switch (ext->extensionType()) {
      case ExtVCardUpdate: {
        // VCard个人信息更新
        /**
         * <pubsub xmlns='http://jabber.org/protocol/pubsub'>
        <items node='urn:xmpp:avatar:data'>
          <item id='111f4b3c50d7b0df729d299bc6f8e9ef9066971f'/>
        </items>
        </pubsub>
         */
        auto vCardUpdate =
            const_cast<VCardUpdate *>(static_cast<const VCardUpdate *>(ext));
        if (vCardUpdate && vCardUpdate->hasPhoto()) {

          ItemList items;
          Item *item0 = new Item();
          item0->setID(vCardUpdate->hash());
          items.emplace_back(item0);

          pubSubManager->requestItems(item.jid().bareJID(), XMLNS_AVATAR, "",
                                      items, this);
        }
        break;
      }
      }
    }
  }
  //  }
};

/**
 * 好友订阅（加好友）请求
 * @param jid
 * @param msg
 * @return
 */
bool IM::handleSubscriptionRequest(const JID &jid, const std::string &msg) {
  DEBUG_LOG(("好友订阅（加好友）请求，来自:%1 消息:%2")
                .arg(qstring(jid.full()))
                .arg(qstring(msg)));
  emit receiveFriendRequest(qstring(jid.bare()), qstring(msg));
  return true;
};

/**
 * 好友取消订阅（被删除）请求
 * @param jid
 * @param msg
 * @return
 */
bool IM::handleUnsubscriptionRequest(const JID &jid, const std::string &msg) {
  DEBUG_LOG(("jid:%1 msg:%2").arg(qstring(jid.full())).arg(qstring(msg)));
  return true;
};

void IM::handleNonrosterPresence(const Presence &presence) {
  DEBUG_LOG(("presence:%1").arg(qstring(presence.from().full())));
};

void IM::handleRosterError(const IQ &iq) {
  DEBUG_LOG(("text:%1").arg(qstring(iq.error()->text())));
};

void IM::handleRosterItemExchange(const gloox::JID &from,
                                  const gloox::RosterX *items) {}

void IM::handleRegistrationFields(const JID &from, int fields,
                                  std::string instructions){};

/**
 * This function is called if @ref Registration::createAccount() was called on
 * an authenticated stream and the server lets us know about this.
 */
void IM::handleAlreadyRegistered(const JID &from){};

/**
 * This funtion is called to notify about the result of an operation.
 * @param from The server or service the result came from.
 * @param regResult The result of the last operation.
 */
void IM::handleRegistrationResult(const JID &from, RegistrationResult regResult,
                                  const Error *error){};

/**
 * This function is called additionally to @ref handleRegistrationFields() if
 * the server supplied a data form together with legacy registration fields.
 * @param from The server or service the data form came from.
 * @param form The DataForm containing registration information.
 */
void IM::handleDataForm(const JID &from, const DataForm &form){};

/**
 * This function is called if the server does not offer in-band registration
 * but wants to refer the user to an external URL.
 * @param from The server or service the referal came from.
 * @param oob The OOB object describing the external URL.
 */
void IM::handleOOB(const JID &from, const OOB &oob){};

FriendId IM::getSelfId() {
  FriendId fId(_client->jid());
  return fId;
}

PeerId IM::getSelfPeerId() {
  PeerId peerId(_client->jid());
  return peerId;
}

QString IM::getSelfUsername() { return qstring(self().username()); }

void IM::setNickname(const QString &nickname) {
  DEBUG_LOG(("nickname:%1").arg(nickname))
  if (_nick == nickname) {
    return;
  }
  _nick = nickname;

  gloox::Nickname nick(nickname.toStdString());

  ItemList items;
  Item *item = new Item();
  item->setID("current");
  item->setPayload(nick.tag());
  items.emplace_back(item);

  pubSubManager->publishItem(JID(), XMLNS_NICKNAME, items, nullptr, this);
}

QString IM::getNickname() {
  if (!_nick.isEmpty()) {
    return _nick;
  }
  return getSelfUsername();
}

void IM::setAvatar(const QByteArray &avatar) {
  if (avatar.isEmpty())
    return;

  QString sha1 = lib::base::Hashs::sha1(avatar);
  DEBUG_LOG(("avatar size:%1 sha1:%2").arg(avatar.size()).arg(sha1))

  auto base64 = avatar.toBase64().toStdString();

  /**
   * <data xmlns='urn:xmpp:avatar:data'>
qANQR1DBwU4DX7jmYZnncm...
</data>
   */
  std::string payload;
  int pos = 0;
  do {
    if (base64.size() <= pos) {
      //      std::remove(payload.begin(), payload.end(), payload.size()-1);
      break;
    }
    std::string line = base64.substr(pos, 76);
    payload += line + "\n";
    pos += 76;
  } while (true);

  AvatarData avt(payload);

  ItemList items;
  Item *item = new Item();
  item->setID(sha1.toStdString());
  item->setPayload(avt.tag());
  items.emplace_back(item);

  pubSubManager->publishItem(JID(), XMLNS_AVATAR, items, nullptr, this);
}

void IM::changePassword(const QString &password) {
  DEBUG_LOG(("password:%1").arg(password));
  if (password.isEmpty())
    return;

  // changing password
  mRegistration->changePassword(_client->username(), stdstring(password));
}

void IM::loadGroupList() {
  auto disco = _client->disco();
  disco->getDiscoItems(JID(XMPP_CONF_SERVER_HOST), "", this, DISCO_CTX_CONF);
  requestBookmarks();

  /**
   * openfire 服务器暂时不支持
   * m_nativeBookmark->retrievesAll();
   */

  loadRosterInfo();
}

void IM::loadRosterInfo() {
  _client->disco()->getDiscoItems(self().bareJID(), "", this, DISCO_CTX_ROSTER);
  auto m = _client->rosterManager()->roster();
  for (auto it = m->begin(); it != m->end(); it++) {
    msleep(100);
    _client->disco()->getDiscoItems(it->first, "", this, DISCO_CTX_ROSTER);
  }
}

Presence::PresenceType IM::getPresenceType() { return selfPresType; }

void IM::sendPresence() {
  //
  Presence pres(Presence::PresenceType::Available, JID());
  pres.addExtension(new Capabilities);
  _client->setPresence();
}

void IM::sendPresence(const JID &to) {
  _client->setPresence(to, Presence::PresenceType::Available, 0);
}

void IM::sendReceiptReceived(const QString &id, QString receiptNum) {

  // <received xmlns='urn:xmpp:receipts' id='richard2-4.1.247'/>

  Message m(gloox::Message::MessageType::Chat, JID(stdstring(id)));
  m.setFrom(_client->jid());

  m.addExtension(
      new Receipt(Receipt::ReceiptType::Received, receiptNum.toStdString()));

  _client->send(m);
}

void IM::sendServiceDiscoveryItems() {

  auto tag = new Tag("query");
  tag->setXmlns(XMLNS_DISCO_ITEMS);

  IQ *iq = new IQ(gloox::IQ::Get, self().server(), _client->getID());
  iq->setFrom(self());

  auto iqt = iq->tag();
  iqt->addChild(tag);
  _client->send(iqt);
}

void IM::sendServiceDiscoveryInfo(const JID &item) {

  auto tag = new Tag("query");
  tag->setXmlns(XMLNS_DISCO_INFO);

  IQ *iq = new IQ(gloox::IQ::Get, item, _client->getID());
  iq->setFrom(self());

  auto iqt = iq->tag();
  iqt->addChild(tag);
  _client->send(iqt);
}

void IM::handleItem(const JID &service, const std::string &node,
                    const Tag *entry) {
  DEBUG_LOG(
      ("service:%1 node:%2").arg(qstring(service.full())).arg(qstring(node)));
}

void IM::handleItems(const std::string &id,                   //
                     const JID &service,                      //
                     const std::string &node,                 //
                     const gloox::PubSub::ItemList &itemList, //
                     const gloox::Error *error) {
  if (error) {
    qDebug() << "Error:" << qstring(error->text());
    return;
  }

  auto friendId = qstring(service.bare());
  DEBUG_LOG(("id:%1 friendId:%2 service:%3 node:%4")
                .arg(qstring(id))
                .arg(friendId)
                .arg(qstring(service.full()))
                .arg(qstring(node)));

  auto isSelf = friendId == getSelfId().toString();

  for (auto &item : itemList) {

    auto data = item->payload();
    auto tagName = data->name();

    DEBUG_LOG(("PubSub::Item tagName:%1 node:%2")
                  .arg(qstring(tagName))
                  .arg(qstring(node)));

    if (node == XMLNS_NICKNAME) {
      qDebug() << "Parse nickname";
      gloox::Nickname nickname(data);
      auto nick = qstring(nickname.nick());
      qDebug() << "nick:" << nick;
      if (isSelf)
        emit selfNicknameChanged(nick);
      else {
        emit receiveNicknameChange(friendId, nick);
      }
    }

    if (node == XMLNS_AVATAR) {
      qDebug() << "Parse avatar";
      auto base64 = data->cdata();
      if (!base64.empty()) {
        std::string::size_type pos = 0;
        while ((pos = base64.find('\n')) != std::string::npos)
          base64.erase(pos, 1);
        while ((pos = base64.find('\r')) != std::string::npos)
          base64.erase(pos, 1);
        auto avt = Base64::decode64(base64);
        qDebug() << "Avatar size" << avt.size();
        if (isSelf) {
          qDebug() << "Receive self avatar";
          emit selfAvatarChanged(avt);
        } else {
          qDebug() << "Receive friend avatar" << friendId;
          emit receiveFriendAvatarChanged(friendId, avt);
        }
      }
    }
  }
}

void IM::handleItemPublication(const std::string &id,    //
                               const JID &service,       //
                               const std::string &node,  //
                               const ItemList &itemList, //
                               const gloox::Error *error) {
  DEBUG_LOG(("node:%1").arg(qstring(node)));
  if (node == XMLNS_AVATAR) {
    // 更新头像元信息
    //  https://xmpp.org/extensions/xep-0084.html#process-pubmeta
    for (auto &item : itemList) {
      DEBUG_LOG(("itemId:%1").arg(qstring(item->id())));
    }
  }
}

void IM::handleItemDeletion(const std::string &id, const JID &service,
                            const std::string &node, const ItemList &itemList,
                            const gloox::Error *error) {
  DEBUG_LOG(("id:%1 service:%2").arg(qstring(id)).arg(qstring(service.full())))
}

void IM::handleSubscriptionResult(const std::string &id, const JID &service,
                                  const std::string &node,
                                  const std::string &sid, const JID &jid,
                                  const gloox::PubSub::SubscriptionType subType,
                                  const gloox::Error *error) {

  DEBUG_LOG(("id:%1 jid:%2").arg(qstring(id)).arg(qstring(jid.full())))
  pubSubManager->requestItems(service, node, sid, 100, this);
}

void IM::handleUnsubscriptionResult(const std::string &id, const JID &service,
                                    const gloox::Error *error) {}
void IM::handleSubscriptionOptions(const std::string &id, const JID &service,
                                   const JID &jid, const std::string &node,
                                   const DataForm *options,
                                   const std::string &sid,
                                   const gloox::Error *error) {}
void IM::handleSubscriptionOptionsResult(const std::string &id,
                                         const JID &service, const JID &jid,
                                         const std::string &node,
                                         const std::string &sid,
                                         const gloox::Error *error) {}
void IM::handleSubscribers(const std::string &id, const JID &service,
                           const std::string &node,
                           const SubscriptionList &list,
                           const gloox::Error *error) {}
void IM::handleSubscribersResult(const std::string &id, const JID &service,
                                 const std::string &node,
                                 const SubscriberList *list,
                                 const gloox::Error *error) {}

void IM::handleAffiliates(const std::string &id, const JID &service,
                          const std::string &node, const AffiliateList *list,
                          const gloox::Error *error) {}

void IM::handleAffiliatesResult(const std::string &id, const JID &service,
                                const std::string &node,
                                const AffiliateList *list,
                                const gloox::Error *error) {}
void IM::handleNodeConfig(const std::string &id, const JID &service,
                          const std::string &node, const DataForm *config,
                          const gloox::Error *error) {}
void IM::handleNodeConfigResult(const std::string &id, const JID &service,
                                const std::string &node,
                                const gloox::Error *error) {}
void IM::handleNodeCreation(const std::string &id, const JID &service,
                            const std::string &node,
                            const gloox::Error *error) {}
void IM::handleNodeDeletion(const std::string &id, const JID &service,
                            const std::string &node,
                            const gloox::Error *error) {}
void IM::handleNodePurge(const std::string &id, const JID &service,
                         const std::string &node, const gloox::Error *error) {}
void IM::handleSubscriptions(const std::string &id, const JID &service,
                             const SubscriptionMap &subMap,
                             const gloox::Error *error) {}
void IM::handleAffiliations(const std::string &id, const JID &service,
                            const AffiliationMap &affMap,
                            const gloox::Error *error) {}

void IM::handleDefaultNodeConfig(const std::string &id, const JID &service,
                                 const DataForm *config,
                                 const gloox::Error *error) {
  Q_UNUSED(config);
  DEBUG_LOG(("id:%1 service:%2 error:%3")
                .arg(qstring(id))
                .arg(qstring(service.full()))
                .arg(qstring(error->text())));
}

std::string IM::getOnlineResource(const std::string &bare) {
  auto it = onlineMap.find(bare);
  if (it == onlineMap.end()) {
    return std::string{};
  }
  for (auto r : it->second) {
    return r;
  }
  return std::string{};
}

std::set<std::string> IM::getOnlineResources(const std::string &bare) {
  auto it = onlineMap.find(bare);
  if (it != onlineMap.end()) {
    return it->second;
  }
  return {};
}

void IM::doJingleMessage(const PeerId &peerId,
                         const Jingle::JingleMessage *jm) {

  DEBUG_LOG(("JingleMessage id:%1 action:%2")
                .arg(qstring(jm->id()))
                .arg(Jingle::ActionValues[jm->action()]));

  qDebug() << "peerId:" << peerId.toString();

  auto friendId = peerId.toFriendId();
  auto callId = qstring(jm->id());

  switch (jm->action()) {
  case Jingle::JingleMessage::propose: {
    /**
     * 接收到呼叫
     * 推送铃声
     * <message from='juliet@capulet.example/phone' to='romeo@montague.example'
     * type='chat'> <ringing xmlns='urn:xmpp:jingle-message:0'
     * id='ca3cf894-5325-482f-a412-a6e9f832298d'/> <store
     * xmlns="urn:xmpp:hints"/>
     * </message>
     */
    if (!mPeerRequestMedias.empty()) {
      rejectJingleMessage(friendId, callId);
      return;
    }
    mPeerRequestMedias.emplace(peerId, jm->medias());
    emit receiveCallRequest(friendId, callId, true, jm->medias().size() > 1);
    break;
  }
  case Jingle::JingleMessage::reject: {
    /**
     * 对方拒绝
     */
    mPeerRequestMedias.clear();
    auto ms = jm->medias();
    emit receiveCallStateRejected(peerId, callId, ms.size() > 1);
    emit receiveFriendHangup(friendId, 0);
    break;
  }
  case Jingle::JingleMessage::accept: {
    /**
     * 被自己账号其它终端接受
     */
    qDebug() << "Accepted by" << peerId.toString();
    emit receiveCallAcceptByOther(callId, peerId);
    mPeerRequestMedias.clear();
    break;
  }
  case Jingle::JingleMessage::retract: {
    /**
     * 撤回(发起者取消)，挂断自己
     */
    mPeerRequestMedias.clear();
    emit receiveCallRetract(friendId, 0);
    break;
  }
  case Jingle::JingleMessage::proceed:
    if (friendId == qstring(self().username())) {
      /**
       * 自己的其他终端接受处理，挂断自己
       */
      emit receiveFriendHangup(friendId, 0);
    } else {
      /**
       * 对方接受
       */
      auto medias = mPeerRequestMedias.find(peerId)->second;
      emit receiveCallStateAccepted(peerId, callId, medias.size() > 1);
    }
    mPeerRequestMedias.clear();
    break;
  case Jingle::JingleMessage::finish:
    mPeerRequestMedias.clear();
    break;
  }
}

void IM::proposeJingleMessage(const QString &friendId, const QString &callId,
                              bool video) {

  auto it = sessionIdMap.find(stdstring(friendId));
  if (it == sessionIdMap.end())
    return;

  auto sit = sessionMap.find(it->second);
  if (sit == sessionMap.end())
    return;

  StanzaExtensionList exts;
  auto *jm = new Jingle::JingleMessage(Jingle::JingleMessage::propose,
                                       stdstring(callId));
  jm->addMedia(Jingle::Media::audio);
  if (video) {
    jm->addMedia(Jingle::Media::video);
  }
  exts.push_back(jm);

  // 缓存发起媒体
  mPeerRequestMedias.emplace(PeerId(friendId), jm->medias());

  auto session = sit->second;
  session->setResource(self().resource());
  session->send("", "", exts);
}

void IM::rejectJingleMessage(const QString &friendId, const QString &callId) {

  auto it = sessionIdMap.find(stdstring(friendId));
  if (it == sessionIdMap.end())
    return;

  auto sit = sessionMap.find(it->second);
  if (sit == sessionMap.end())
    return;

  StanzaExtensionList exts;
  auto reject = new Jingle::JingleMessage(Jingle::JingleMessage::reject,
                                          stdstring(callId));
  exts.push_back(reject);

  auto session = sit->second;
  session->setResource(self().resource());
  session->send("", "", exts);

  mPeerRequestMedias.clear();
}

void IM::acceptJingleMessage(const QString &friendId, const QString &callId) {
  auto it = sessionIdMap.find(stdstring(friendId));
  if (it == sessionIdMap.end()) {
    qWarning() << "Unable to find friendId" << friendId << "session" << callId;
    return;
  }

  auto sit = sessionMap.find(it->second);
  if (sit == sessionMap.end()) {
    qWarning() << "Unable to find friendId" << friendId << "session" << callId;
    return;
  }

  auto proceed = new Jingle::JingleMessage(Jingle::JingleMessage::proceed,
                                           stdstring(callId));
  Message proceedMsg(gloox::Message::Chat, JID(stdstring(friendId)));
  proceedMsg.addExtension(proceed);
  _client->send(proceedMsg);
  qDebug() << "Sent proceed for jingle-message";

  // 发送给自己其它终端
  auto accept = new Jingle::JingleMessage(Jingle::JingleMessage::accept,
                                          stdstring(callId));
  Message msg(gloox::Message::Chat, self().bareJID());
  msg.addExtension(accept);
  _client->send(msg);
  qDebug() << "Sent accept for jingle-message";
}

void IM::retractJingleMessage(const QString &friendId, const QString &callId) {
  auto it = sessionIdMap.find(stdstring(friendId));
  if (it == sessionIdMap.end())
    return;

  auto sit = sessionMap.find(it->second);
  if (sit == sessionMap.end())
    return;

  auto session = sit->second;
  auto *jm1 = new Jingle::JingleMessage(Jingle::JingleMessage::retract,
                                        stdstring(callId));
  StanzaExtensionList exts1;
  exts1.push_back(jm1);
  session->send("", "", exts1);
}

void IM::retry() { doConnect(); }

bool IM::leaveGroup(const QString &groupId) {
  qDebug() << "leaveGroup" << groupId;
  auto r = findRoom(groupId);
  if (!r) {
    qWarning() << "Unable to find room" << groupId;
    return false;
  }
  r->room->leave();

  // 删除缓存
  m_roomMap.remove(groupId);

  // 从书签删除，再保存书签
  mConferenceList.remove_if(
      [&](ConferenceListItem &a) { return a.jid == stdstring(groupId); });

  bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);

  return true;
}

bool IM::destroyGroup(const QString &groupId) {
  qDebug() << "destroyGroup" << groupId;

  auto r = findRoom(groupId);
  if (!r) {
    qWarning() << "Unable to find room" << groupId;
    return false;
  }
  // 删除
  r->room->destroy();

  // 删除缓存
  m_roomMap.remove(groupId);

  // 从书签删除，再保存书签
  mConferenceList.remove_if(
      [&](ConferenceListItem &a) { return a.jid == stdstring(groupId); });

  bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);

  return true;
}

Disco::ItemList IM::handleDiscoNodeItems(const JID &from, const JID &to,
                                         const std::string &node) {

  DEBUG_LOG(("from:%1").arg(from.full().c_str()))

  return gloox::Disco::ItemList();
}

Disco::IdentityList IM::handleDiscoNodeIdentities(const JID &from,
                                                  const std::string &node) {
  return gloox::Disco::IdentityList();
}

StringList IM::handleDiscoNodeFeatures(const JID &from,
                                       const std::string &node) {
  return gloox::StringList();
}

void IM::onSelfNicknameChanged(const QString &nickname) {
  qDebug() << "onSelfNicknameChanged" << nickname;
  _nickChanged++;
  //  if (_nickChanged == 1) {
  //    for (auto &c : mConferenceList) {
  //      JID roomJid(c.jid);
  //      roomJid.setResource(stdstring(nickname));
  //      joinRoom(roomJid);
  //    }
  //  }
}

void IM::endJingle() { mPeerRequestMedias.clear(); }
void IM::handleIncoming(gloox::Tag *tag) {
  auto services =
      tag->findChild("services", "xmlns", XMLNS_EXTERNAL_SERVICE_DISCOVERY);
  if (services) {
    mExtDisco = ExtDisco(services);
  }
  emit incoming(::base::Xmls::parse(qstring(tag->xml())));
}

void IM::onDisconnect(ConnectionError e) {
  DEBUG_LOG(("error:%1").arg(e));
  switch (e) {
  case ConnAuthenticationFailed:
    _status = IMStatus::AUTH_FAILED;
    break;
  case ConnNoError:
    _status = IMStatus::DISCONNECTED;
    break;
  case ConnStreamError:
  case ConnStreamVersionError:
    _status = IMStatus::CONN_ERROR;
    break;
  case ConnStreamClosed:
    _status = IMStatus::DISCONNECTED;
    break;
  case ConnProxyGatewayTimeout:
    _status = IMStatus::TIMEOUT;
    break;
  case ConnProxyAuthRequired:
  case ConnProxyAuthFailed:
  case ConnProxyNoSupportedAuth:
  case ConnIoError:
  case ConnParseError:
  case ConnConnectionRefused:
  case ConnDnsError:
    _status = IMStatus::CONN_ERROR;
    break;
  case ConnOutOfMemory:
    _status = IMStatus::OUT_OF_RESOURCE;
    break;
  case ConnNoSupportedAuth:
    _status = IMStatus::NO_SUPPORT;
    break;
  case ConnTlsFailed:
  case ConnTlsNotAvailable:
    _status = IMStatus::TLS_ERROR;
    break;
  case ConnCompressionFailed:
  case ConnUserDisconnected:
  case ConnNotConnected:
    _status = IMStatus::DISCONNECTED;
    break;
  case ConnAttemptTimeout:
    break;
    _status = IMStatus::TIMEOUT;
  default:
    _status = IMStatus::DISCONNECTED;
    break;
  }

  emit connectResult(_status);
}

bool IM::onTLSConnect(const CertInfo &info) {
  DEBUG_LOG(("CertInfo:\n"))

  time_t from(info.date_from);
  time_t to(info.date_to);

  DEBUG_LOG(("status: %1\n"   //
             "issuer: %2\n"   //
             "peer: %3\n"     //
             "protocol: %4\n" //
             "mac: %5\n"      //
             "cipher: %6\n"   //
             "compression: %7\n")
                .arg((info.status))                     //
                .arg(qstring(info.issuer.c_str()))      //
                .arg(qstring(info.server.c_str()))      //
                .arg((info.protocol))                   //
                .arg(qstring(info.mac.c_str()))         //
                .arg(qstring(info.cipher.c_str()))      //
                .arg(qstring(info.compression.c_str())) //
  )

  DEBUG_LOG(("from:%1").arg(ctime(&from)));
  DEBUG_LOG(("to:%1").arg(ctime(&to)));

  return true;
}

void IM::handleLog(LogLevel level, LogArea area, const std::string &message) {
  //   DEBUG_LOG(("%1").arg(message.c_str()));
  switch (area) {
  case LogAreaXmlIncoming:
    DEBUG_LOG(("Received XML:\n%1").arg(message.c_str()));
    break;
  case LogAreaXmlOutgoing:
    DEBUG_LOG(("Sent XML:\n%1").arg(message.c_str()));
    break;
  case LogAreaClassConnectionBOSH:
    DEBUG_LOG(("BOSH:%1").arg(message.c_str()));
    break;
  case LogAreaClassClient:
    DEBUG_LOG(("Client: %1").arg(message.c_str()));
    break;
  case LogAreaClassDns:
    DEBUG_LOG(("dns: %1").arg(message.c_str()));
    break;
  default:
    DEBUG_LOG(("level: %1, area: %2 msg: %3")
                  .arg(level)
                  .arg(area)
                  .arg(message.c_str()));
  }
}

} // namespace messenger
} // namespace lib
