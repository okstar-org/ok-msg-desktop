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

#include "IM.h"
#include "IMJingle.h"

// TODO resolve conflict DrawText in WinUser.h
#undef DrawText

#include "base/hashs.h"
#include "base/logs.h"
#include "base/times.h"
#include "base/xmls.h"

#include <list>
#include <string>
#include <thread>
#include <utility>

#include <attention.h>
#include <avatar.h>
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
#include <rosteritemdata.h>
#include <rostermanager.h>
#include <vcardupdate.h>
#include <QStringList>

namespace lib::messenger {

using namespace gloox;

#define DISCO_CTX_CONF 0
#define DISCO_CTX_ROSTER 1
#define DISCO_CTX_BOOKMARKS 2
#define DISCO_CTX_CONF_MEMBERS 3

ConferenceList mConferenceList;
BookmarkList mBookmarkList;
bool __started = false;

/**
 * 聊天通讯核心类
 * @param user
 * @param pwd
 * @param features_
 */
IM::IM(QString host,
       QString user,
       QString pwd,
       QStringList features_)  //
        : features(std::move(features_))
        ,  //
        _host(std::move(host))
        ,  //
        _username(std::move(user))
        ,  //
        _password(std::move(pwd)) {
    qDebug() << __func__ << "Create instance...";

    setObjectName("IM-Connect");

    auto osInfo = ok::base::SystemInfo::instance()->osInfo();

    discoVal = osInfo.prettyName;

    // 生成本机resource. 格式:OkEDU.<HOST>.[VER].[UNIQUE]
    _resource = QString("%1.%2.[%3].%4")  //
                        .arg(APPLICATION_ALIAS)
                        .arg(osInfo.hostName)
                        .arg(GIT_DESCRIBE)
                        .arg(osInfo.uniqueId.mid(0, 6));

    qDebug() << "Generate self resource:" << _resource;

    // qRegisterMetaType
    qRegisterMetaType<JID>("JID");
    qRegisterMetaType<IMContactId>("IMContactId");
    qRegisterMetaType<IMFriend>("IMFriend");
    qRegisterMetaType<IMPeerId>("IMPeerId");
    qRegisterMetaType<IMMessage>("IMMessage");
    qRegisterMetaType<IMGroupOccupant>("IMGroupOccupant");

    qDebug() << "Create messenger instance is successfully";
}

IM::~IM() { qDebug() << __func__; }

void IM::run() {
    qDebug() << __func__;
    doConnect();
}

std::unique_ptr<Client> IM::makeClient() {
    JID loginJid(QString("%1@%2/%3").arg(_username).arg(_host).arg(_resource).toStdString());

    qDebug() << __func__ << "Using Jid:" << qstring(loginJid.full());

    /**
     * Client
     */
    auto client = std::make_unique<Client>(loginJid, _password.toStdString());
    auto disco = client->disco();

    disco->setVersion("disco", APPLICATION_VERSION, discoVal.toStdString());
    disco->setIdentity("client", "pc", APPLICATION_RELEASE);
    disco->addIdentity("pubsub", "pep");
    disco->addFeature(XMLNS_PUBSUB);
    disco->addFeature(XMLNS_PUBSUB_EVENT);
    disco->addFeature(XMLNS_PUBSUB_OWNER);
    disco->addFeature(XMLNS_PUBSUB_PUBLISH_OPTIONS);
    disco->addFeature(XMLNS_PUBSUB_AUTO_SUBSCRIBE);
    disco->addFeature(XMLNS_PUBSUB_AUTO_CREATE);
    client->registerStanzaExtension(new Disco::Items);
    client->registerStanzaExtension(new PubSub::Event());

    client->registerStanzaExtension(new VCardUpdate);
    client->registerStanzaExtension(new Capabilities);
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

    for (const auto& feat : features) {
        qDebug() << "addFeature:" << feat;
        disco->addFeature(stdstring(feat));
    }

    /**
     * 聊天相关
     */
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

    disco->addFeature(XMLNS_IBB);

    // NICK
    disco->addFeature(XMLNS_NICKNAME);
    disco->addFeature(XMLNS_NICKNAME + "+notify");

    client->setTls(TLSPolicy::TLSDisabled);
    client->setCompression(false);

    client->registerIqHandler(this, ExtPubSub);
    //  client->registerIncomingHandler(this);

    /**
     * listeners
     */
    client->registerConnectionListener(this);

    client->registerPresenceHandler(this);
    client->registerMessageSessionHandler(this);
    client->registerMessageHandler(this);
    //  client->setStreamManagement(true, true);

#ifdef LOG_XMPP
    client->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);
#endif

    vCardManager = std::make_unique<VCardManager>(client.get());
    pubSubManager = std::make_unique<PubSub::Manager>(client.get());
    bookmarkStorage = std::make_unique<gloox::BookmarkStorage>(client.get());
    return std::move(client);
}

void IM::stop() {
    qDebug() << __func__;
    doDisconnect();
    emit stopped();
}

void IM::onDisconnect(ConnectionError e) {
    qDebug() << __func__ << "error:" << e;

    if (!__started) {
        return;
    }

    __started = false;

    IMConnectStatus _status;
    switch (e) {
        case ConnAuthenticationFailed:
            _status = IMConnectStatus::AUTH_FAILED;
            break;
        case ConnNoError:
            _status = IMConnectStatus::DISCONNECTED;
            break;
        case ConnStreamError:
        case ConnStreamVersionError:
            _status = IMConnectStatus::CONN_ERROR;
            break;
        case ConnStreamClosed:
            _status = IMConnectStatus::DISCONNECTED;
            break;
        case ConnProxyGatewayTimeout:
            _status = IMConnectStatus::TIMEOUT;
            break;
        case ConnProxyAuthRequired:
        case ConnProxyAuthFailed:
        case ConnProxyNoSupportedAuth:
        case ConnIoError:
        case ConnParseError:
        case ConnConnectionRefused:
        case ConnDnsError:
            _status = IMConnectStatus::CONN_ERROR;
            break;
        case ConnOutOfMemory:
            _status = IMConnectStatus::OUT_OF_RESOURCE;
            break;
        case ConnNoSupportedAuth:
            _status = IMConnectStatus::NO_SUPPORT;
            break;
        case ConnTlsFailed:
        case ConnTlsNotAvailable:
            _status = IMConnectStatus::TLS_ERROR;
            break;
        case ConnCompressionFailed:
        case ConnUserDisconnected:
        case ConnNotConnected:
            _status = IMConnectStatus::DISCONNECTED;
            break;
        case ConnAttemptTimeout:
            _status = IMConnectStatus::TIMEOUT;
            break;
        default:
            _status = IMConnectStatus::DISCONNECTED;
            break;
    }

    emit connectResult(_status);
    emit stopped();
}

void IM::onConnect() {
    qDebug() << __func__ << "connected";
    if (__started) {
        return;
    }

    __started = true;

    auto res = _client->resource();
    qDebug() << __func__ << "resource:" << (qstring(res));

    //  fetchVCard(qstring(self().bare()));
    //  emit selfIdChanged(qstring(_client->username()));

    emit connectResult(IMConnectStatus::CONNECTED);
    emit started();

    qDebug() << __func__ << "Resume.";
}

void IM::doConnect() {
    qDebug() << __func__ << "Create IM client...";
    _client = makeClient();
    qDebug() << __func__ << "Create IM client is:" << _client.get();
    _client->connect(true);

    emit connectResult(IMConnectStatus::CONNECTING);
}

void IM::send(const QString& xml) {
    if (xml.isEmpty()) {
        qWarning() << "send xml content is empty";
        return;
    }
    _client->send(stdstring(xml));
}

void IM::interrupt() {}

IMMessage IM::fromXMsg(MsgType type, const gloox::Message& msg) {
    IMMessage imMsg = {.from = qstring(msg.from().full()),
                       .to = qstring(msg.to().full()),
                       .body = qstring(msg.body()),
                       .timestamp = ok::base::Times::now()};

    if (!msg.id().empty()) {
        imMsg.id = (qstring(msg.id()));
    }
    return imMsg;
}

void IM::enableDiscoManager() {
    qDebug() << "enableDiscoManager";

    auto client = _client.get();
    auto disco = client->disco();

    for (const auto& feat : features) {
        qDebug() << "addFeature:" << feat;
        disco->addFeature(stdstring(feat));
    }

    /**
     * 头像相关
     */
    //  client->registerStanzaExtension(new Avatar);
    //  // urn:xmpp:avatar:data
    //  disco->addFeature(XMLNS_AVATAR);
    //  // urn:xmpp:avatar:metadata
    //  disco->addFeature(XMLNS_META_AVATAR);
    //  // urn:xmpp:avatar:metadata+notify
    //  disco->addFeature(XMLNS_META_AVATAR + "+notify");

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

    /**
     * 昵称 NICK
     */
    disco->addFeature(XMLNS_NICKNAME);
    disco->addFeature(XMLNS_NICKNAME + "+notify");
    client->registerStanzaExtension(new Nickname(nullptr));

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

gloox::RosterManager* IM::enableRosterManager() {
    qDebug() << __func__;
    /**
     * roster
     */
    auto pRosterManager = _client->enableRoster();
    pRosterManager->registerRosterListener(this);
    return pRosterManager;
    //   enable carbons（多终端支持）
    IQ iq(IQ::IqType::Set, JID(), "server");
    iq.addExtension(new Carbons(gloox::Carbons::Enable));
    _client->send(iq);

    // request ext server disco
    IQ iq2(gloox::IQ::Get, JID(_host.toStdString()));
    auto t = iq2.tag();
    t->addChild(gloox::ExtDisco::newRequest());
    _client->send(t);
    //
    //  /**
    //   * Registration
    //   */
    //  mRegistration = std::make_unique<Registration>(_client.get());
    //  mRegistration->registerRegistrationHandler(this);
}

void IM::doDisconnect() {
    assert(_client.get());
    _client->disconnect();
}

QDomDocument IM::buildMessage(const QString& to,   //
                              const QString& msg,  //
                              const QString& id) {
    gloox::Message m(gloox::Message::MessageType::Chat, JID(stdstring(to)), msg.toStdString());
    m.setFrom(_client->jid());
    m.setID(stdstring(id));
    m.addExtension(new Receipt(Receipt::Request));

    return ok::base::Xmls::parse(qstring(m.tag()->xml()));
}

bool IM::sendTo(const QString& friendId, const QString& msg, const QString& id) {
    qDebug() << "sendTo:" << friendId;
    qDebug() << "msgId:" << id << "msg:" << msg;

    gloox::Message m(gloox::Message::MessageType::Chat, JID(stdstring(friendId)),
                     msg.toStdString());
    m.setFrom(_client->jid());
    m.setID(stdstring(id));
    m.addExtension(new Receipt(Receipt::Request));

    _client->send(m);
    return true;
}

// Handle Message session
void IM::handleMessageSession(MessageSession* session) {
    auto from = qstring(session->target().full());
    auto sid = qstring(session->threadID());
    qDebug() << __func__ << "from" << from << "sid:" << sid;

    // 联系人ID（朋友和群聊）
    auto contactId = qstring(session->target().bare());
    emit receiveMessageSession(contactId, sid);

    // 聊天状态过滤器，获取：正在中等输入状态
    auto csf = m_chatStateFilters.value(contactId);
    if (!csf) {
        csf = new ChatStateFilter(session);
        csf->registerChatStateHandler(this);
        m_chatStateFilters.insert(contactId, csf);
    }
}

void IM::handleMessage(const gloox::Message& msg, MessageSession* session) {
    auto from = msg.from();
    auto peerId = qstring(from.full());
    auto friendId = qstring(from.bare());
    auto body = qstring(msg.body());

    qDebug() << __func__ << "from:" << peerId << "subtype:" << (int)msg.subtype();

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
        for (const auto& item : address->addresses()) {
            qDebug() << "address type:" << qstring(item.type) << "jid:" << qstring(item.jid.full());
        }
    }

    auto xml = qstring(msg.tag()->xml());
    emit incoming(xml);
}

void IM::doMessageHeadline(const Message& msg, QString& friendId, const QString& body) {
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
void IM::doMessageChat(const Message& msg, QString& friendId, const QString& body) {
    qDebug() << "doMessageChat from:" << friendId;
    if (!body.isEmpty()) {
        if (!msg.encrypted()) {
            qDebug() << "Is plain message:" << body;
            IMMessage imMsg = fromXMsg(MsgType::Chat, msg);
            emit receiveFriendMessage(friendId, imMsg);
        } else {
            QString xml = qstring(msg.tag()->xml());
            qDebug() << "Is encrypted message:" << xml;
            emit exportEncryptedMessage(xml);
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
        Message* eMsg = static_cast<Message*>(pCarbons->embeddedStanza());
        if (eMsg) {
            auto ebody = eMsg->body();
            if (!ebody.empty()) {
                auto msg = fromXMsg(MsgType::Chat, *eMsg);
                msg.to = qstring(eMsg->to().full());
                emit receiveFriendMessage(msg.to, msg);
            }
        }
    }
}

/**
 * 处理常规消息
 * @param msg
 * @param friendId
 */
void IM::doMessageNormal(const Message& msg, QString& friendId) {
    auto conf = msg.findExtension<Conference>(ExtConference);
    if (conf) {
        auto jid = conf->jid().bare();
        qDebug() << "New conference is arrival" << qstring(jid);
        cacheJoinRoom(jid);
    }
}

void IM::handleMessageEvent(const JID& from, const MessageEvent* et) {
    qDebug() << ("JID:") << qstring(from.full()) << " MessageEvent:%2" << et;
}

void IM::doPubSubEvent(const gloox::PubSub::Event* pse,  //
                       const Message& msg,               //
                       QString& friendId) {              //

    const QString& selfId = getSelfId().toString();
    auto isSelf = friendId == selfId;

    for (auto& item : pse->items()) {
        auto nickTag = item->payload->findChild("nick");
        if (nickTag) {
            Nickname nickname(nickTag);
            auto newNick = qstring(nickname.nick());
            qDebug() << "nick:" << newNick;
            emit receiveNicknameChange(friendId, newNick);
            if (isSelf) {
                if (_nick != newNick) {
                    _nick = newNick;
                    emit selfNicknameChanged(newNick);
                }
            }
        }
        auto avatarData = item->payload->findChild("data", XMLNS, XMLNS_AVATAR);
        if (avatarData) {
            auto base64 = avatarData->cdata();
            if (!base64.empty()) {
                std::string::size_type pos = 0;
                while ((pos = base64.find('\n')) != std::string::npos) base64.erase(pos, 1);
                while ((pos = base64.find('\r')) != std::string::npos) base64.erase(pos, 1);

                auto avt = Base64::decode64(base64);
                if (isSelf) {
                    qDebug() << "Receive self avatar size" << avt.size();
                    emit selfAvatarChanged(avt);
                } else {
                    qDebug() << "Receive friend avatar" << friendId << "size" << avt.size();
                    emit receiveFriendAvatarChanged(friendId, avt);
                }
            }
        }

        auto avatarMetaData = item->payload->findChild("metadata",  //
                                                       XMLNS,       //
                                                       XMLNS_META_AVATAR);
        if (avatarMetaData) {
            auto itemId = avatarMetaData->findChild("info")->findAttribute("id");

            ItemList items;
            Item* item0 = new Item();
            item0->setID(itemId);
            items.emplace_back(item0);

            if (pubSubManager) {
                pubSubManager->requestItems(msg.from(), XMLNS_AVATAR, "", items, this);
            }
        }

        // list xmlns='eu.siacs.conversations.axolotl'
        //    auto devices = item->payload->findChild("list", //
        //                                            XMLNS,  //
        //                                            "eu.siacs.conversations.axolotl");
        //    if (devices) {
        //      for (auto dev : devices->findChildren("device")) {
        //        qDebug() << "device id:" << qstring(dev->findAttribute("id"));
        //      }
        //    }
        //
        //    //bundle xmlns="eu.siacs.conversations.axolotl"
        //    auto bundle = item->payload->findChild("bundle",
        //    XMLNS,"eu.siacs.conversations.axolotl" ); if(bundle){
        //
        //      auto deviceId=qstring(pse->node()).split(":")[1];
        //
        //      qDebug() <<"deviceId: " << deviceId;
        //
        //      auto spkp = bundle->findChild("signedPreKeyPublic")->cdata();
        //      qDebug() <<"signedPreKeyPublic: "<< qstring(spkp);
        //
        //      auto spks = bundle->findChild("signedPreKeySignature")->cdata();
        //      qDebug() <<"signedPreKeySignature: "<< qstring(spks);
        //
        //      auto ik = bundle->findChild("identityKey")->cdata();
        //      qDebug() <<"identityKey: "<< qstring(ik);
        //
        //      //
        //      auto prekeys = bundle->findChild("prekeys")->children();
        //      for (auto pk : prekeys) {
        //        //preKeyId
        //        qDebug() <<"preKeyId:" << qstring( pk->findAttribute("preKeyId"))
        //                 <<"identityKey: "<< qstring(pk->cdata());
        //
        //      }
        //    }
    }
}

/**
 * 接收来自朋友聊天状态
 * @breif http://jabber.org/protocol/chatstates
 * @param from
 * @param state
 */
void IM::handleChatState(const JID& from, ChatStateType state) {
    qDebug() << "from:" << qstring(from.full()) << "state:" << static_cast<int>(state);

    auto friendId = qstring(from.bare());
    emit receiveFriendChatState(friendId, state);
}

/**
 * 发送给朋友当前聊天状态
 * @param to
 * @param state 聊天状态
 */
void IM::sendChatState(const QString& to, ChatStateType state) {
    qDebug() << "sendChatState" << "to:" << to << "state:" << (static_cast<int>(state));
    auto csf = m_chatStateFilters[to];
    if (!csf) {
        qWarning() << "Chat state filter is no existing!";
        return;
    }
    csf->setChatState(state);
}

// MUC handler -- handleMUCParticipantPresence
void IM::handleMUCParticipantPresence(gloox::MUCRoom* room,                  //
                                      const MUCRoomParticipant participant,  //
                                      const Presence& presence) {
    auto groupId = qstring(room->jid().full());
    auto nick = qstring(participant.nick->resource());

    qDebug() << __func__ << groupId;
    auto tag = presence.tag();

    auto nickTag = tag->findChild("nick");
    if (nickTag) nick = qstring(nickTag->cdata());

    auto x = tag->findChild("x", XMLNS, XMLNS_MUC_USER);
    if (x) {
        IMGroupOccupant occ = {.nick = nick};

        auto item = x->findChild("item");
        if (item) {
            auto affiliation = qstring(item->findAttribute("affiliation"));
            auto role = qstring(item->findAttribute("role"));
            auto jid = item->findAttribute("jid");
            occ.jid = qstring(jid);
            occ.affiliation = affiliation;
            occ.role = role;
            occ.status = presence.presence();
        }

        for (auto t : x->findChildren("status")) {
            occ.codes.push_back(qstring(t->findAttribute("code")).toInt());
        }

        emit groupOccupantStatus(groupId, occ);
    }
}

void IM::handleMUCMessage(MUCRoom* room, const gloox::Message& msg, bool priv) {
    auto msgId = qstring(msg.id());
    qDebug() << "msgId:" << (msgId);

    auto roomId = qstring(room->jid().full());
    qDebug() << "roomId:" << roomId;

    auto from = qstring(msg.from().full());
    qDebug() << "from:" << from;

    auto body = qstring(msg.body());
    qDebug() << "body:" << body;

    IMPeerId peerId(msg.from());
    for (const auto& id : sendIds) {
        if (id == msg.id()) {
            qDebug() << "自己发出消息，忽略！";
            sendIds.erase(id);
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
            emit groupInvite(qstring(JID(*mu->jid()).bare()), roomId, qstring(msg.body()));
        }

        return;
    }

    if (body.isEmpty()) {
        return;
    }

    IMMessage imMsg = fromXMsg(MsgType::Groupchat, msg);
    const DelayedDelivery* dd = msg.when();
    if (dd) {
        // yyyy-MM-dd HH:mm:ss 20230614T12:11:43Z
        imMsg.timestamp = QDateTime::fromString(qstring(dd->stamp()), "yyyy-MM-ddTHH:mm:ssZ");
    }

    auto addresses = msg.address();
    if (addresses) {
        for (auto& item : addresses->addresses()) {
            // 群消息来源辨别多终端
            if (item.type == "ofrom") {
                imMsg.from = qstring(item.jid.full());
            }
        }
    }

    emit receiveRoomMessage(roomId, peerId, imMsg);
}

bool IM::handleMUCRoomCreation(MUCRoom* room) {
    qDebug() << "handleMUCRoomCreation" << room->jid().full().c_str();

    room->requestRoomConfig();

    // 添加到缓存
    auto roomId = qstring(room->jid().bare());
    m_roomMap.insert(roomId, IMRoomInfo{room, {}});

    // 群聊增加
    emit groupReceived(roomId, qstring(room->name()));

    joinRoom(roomId);

    //  bookmarkStorage->requestBookmarks();
    return true;
}

void IM::handleMUCSubject(MUCRoom* room,            //
                          const std::string& nick,  //
                          const std::string& subject) {
    qDebug() << __func__ << room->name().c_str() << "subject" << qstring(subject);
    emit groupSubjectChanged(room->jid(), subject);

    // 可能存在其它更新，执行信息拉取
    room->getRoomInfo();
}

void IM::handleMUCInviteDecline(MUCRoom* room, const JID& invitee, const std::string& reason) {
    qDebug() << QString("MUCRoom:%1").arg(qstring(room->name()));
    qDebug() << QString("invitee:%1 reason:%2").arg(qstring(invitee.full())).arg(qstring(reason));
}

void IM::handleMUCError(MUCRoom* room, StanzaError error) {
    qDebug() << __func__ << "MUCRoom:" << qstring(room->name()) << error;
}

void IM::handleMUCInfo(MUCRoom* room,               //
                       int features,                //
                       const std::string& name,     //
                       const DataForm* infoForm) {  //

    auto roomId = qstring(room->jid().full());
    auto roomName = qstring(name);

    qDebug() << __func__ << roomId << roomName;

    IMGroup groupInfo;
    groupInfo.name = roomName;

    if (infoForm) {
        auto info = const_cast<IMRoomInfo*>(findRoom(roomId));

        for (auto field : infoForm->fields()) {
            qDebug() << "field name:" << qstring(field->name()) << qstring(field->value());

            if (field->name() == "muc#roominfo_occupants") {
                groupInfo.occupants = std::stoi(field->value());
            } else if (field->name() == "muc#roominfo_roomname" && !field->value().empty()) {
                groupInfo.name = qstring(field->value());
            } else if (field->name() == "muc#roominfo_subject") {
                groupInfo.subject = qstring(field->value());
            } else if (field->name() == "muc#roominfo_description") {
                groupInfo.description = qstring(field->value());
            } else if (field->name() == "muc#roominfo_creationdate") {
                groupInfo.creationdate = qstring(field->value());
            }
        }
        info->info = groupInfo;
        emit groupRoomInfo(roomId, groupInfo);
    }
}

void IM::handleMUCItems(MUCRoom* room, const Disco::ItemList& items) {
    qDebug() << QString("MUCRoom:%1").arg(qstring(room->name()));
    qDebug() << QString("items:%1").arg(items.size());
}

// config
void IM::handleMUCConfigList(MUCRoom* room,                 //
                             const MUCListItemList& items,  //
                             MUCOperation operation) {
    qDebug() << QString("IM::handleMUCConfigList");
}

void IM::handleMUCConfigForm(MUCRoom* room, const DataForm& form) {
    qDebug() << __func__ << "room:" << room->jid().full().c_str();

    auto roomId = qstring(room->jid().bare());

    auto find = m_roomMap.find(roomId);
    if (find == m_roomMap.end()) {
        qWarning() << "Unable to find room from cache:" << roomId;
        return;
    }

    /**
     * 将缓存的房间配置放入`DataForm`
     */
    int changed = 0;
    auto& roomInfo = find.value();
    for (auto item : form.fields()) {
        for (const auto& change : roomInfo.changes) {
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

        auto* mform = new DataForm(form);
        mform->setType(FormType::TypeSubmit);
        room->setRoomConfig(mform);
        qDebug() << "Update room config items" << changed;
    }
};

void IM::handleMUCConfigResult(MUCRoom* room, bool success, MUCOperation operation) {
    qDebug() << __func__ << "room" << qstring(room->jid().full()) << "operation:" << operation
             << "success:" << success;
    if (success) {
        // 成功则忽略
        return;
    }

    switch (operation) {
        case RequestRoomConfig: {
            qWarning() << "无操作群配置权限";
            break;
        }
        default:
            // other
            break;
    }
};

void IM::handleMUCRequest(MUCRoom* room, const DataForm& form) {
    qDebug() << "handleMUCRequest room" << qstring(room->jid().full());
    for (const auto& item : form.items()) {
        qDebug() << "item" << item;
        for (const auto& item_j : item->fields()) {
            qDebug() << "field:" << item_j->name().c_str() << "=>" << item_j->value().c_str();
        }
    }
};

/**
 * 初始化房间并加入
 */
void IM::createRoom(const JID& jid, const std::string& password) {
    qDebug() << __func__ << "jid:" << qstring(jid.full());

    auto room = new MUCRoom(_client.get(), jid, this, this);
    if (!password.empty()) {
        room->setPassword(password);
    }

    room->instantRoom(MUCOperation::CreateInstantRoom);
    cacheJoinRoom(jid.bare(), jid.resource());

    setRoomName(qstring(jid.bare()), jid.resource());

    ConferenceListItem item;
    item.name = jid.resource();
    item.jid = room->jid().full();
    item.autojoin = true;
    item.nick = stdstring(getNickname());

    // 添加到书签列表
    mConferenceList.emplace_back(item);

    // 存储书签列表
    bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);
    qDebug() << "Store bookmarks is successful for room";
}

/**
 * 要求成员加入房间
 * @param roomJid
 * @param peerId
 */
bool IM::inviteToRoom(const JID& roomJid, const JID& peerId) {
    auto roomInfo = findRoom(qstring(roomJid.bare()));
    if (!roomInfo) {
        return false;
    }

    roomInfo->room->invite(peerId, "");
    return true;
}

const IMRoomInfo* IM::findRoom(const QString& groupId) const {
    auto it = m_roomMap.find(groupId);
    if (it == m_roomMap.end()) {
        qWarning() << "Unable find room" << groupId;
        return nullptr;
    }
    return &(it.value());
}

void IM::joinRoom(MUCRoom* room) {
    room->setNick(stdstring(getNickname()));
    room->join();
    room->getRoomInfo();
    _client->disco()->getDiscoItems(room->jid(), XMLNS_BOOKMARKS, this, DISCO_CTX_BOOKMARKS);
}

void IM::cacheJoinRoom(const std::string& jid, const std::string& name) {
    qDebug() << __func__ << qstring(jid) << qstring(name);

    JID roomJid(jid);
    auto roomId = qstring(roomJid.bare());
    auto room = new MUCRoom(_client.get(), roomJid, this, this);
    m_roomMap.insert(roomId, IMRoomInfo{room, {}});

    qDebug() << "emit room:" << roomId;
    emit groupReceived(roomId, name.empty() ? qstring(roomJid.username()) : qstring(name));

    // 查询成员列表
    // XMLNS_DISCO_ITEMS
    getClient()->disco()->getDiscoItems(jid, XMLNS_DISCO_ITEMS, this, DISCO_CTX_CONF_MEMBERS);

    joinRoom(room);
}

void IM::joinRoom(const QString& roomJid) {
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
    for (const auto& item : m_roomMap) {
        joinRoom(item.room);
    }
}

bool IM::sendToRoom(const QString& to, const QString& msg, const QString& id) {
    qDebug() << __func__ << "=>" << to;
    qDebug() << "msgId:" << id << "content:" << msg;

    if (msg.isEmpty()) {
        qWarning() << "empty message!";
        return false;
    }

    if (id.isEmpty()) {
        qWarning() << "id is empty!";
        return false;
    }

    auto pRoomInfo = findRoom((to));
    if (!pRoomInfo) {
        qWarning() << "The room is not exist!";
        return false;
    }

    auto msgId = stdstring(id);
    sendIds.insert(msgId);
    return pRoomInfo->room->send(msg.toStdString(), msgId);
}

void IM::setRoomSubject(const QString& groupId, const std::string& subject) {
    auto room = findRoom(groupId);
    if (room) {
        room->room->setSubject(subject);
    }
}

void IM::setRoomName(const QString& groupId, const std::string& roomName) {
    qDebug() << __func__ << groupId << roomName.c_str();
    const IMRoomInfo* pRoomInfo = findRoom(groupId);
    if (!pRoomInfo) {
        qDebug() << "room is not exist." << groupId;
        return;
    }

    auto info = const_cast<IMRoomInfo*>(pRoomInfo);
    info->changes.insert(std::make_pair("muc#roomconfig_roomname", roomName));
    // 获取新配置（handleMUCConfigForm处理）
    info->room->requestRoomConfig();
}

void IM::setRoomAlias(const QString& groupId, const std::string& alias) {
    qDebug() << __func__ << groupId << alias.c_str();
    // 修改书签列表
    bool update = false;
    for (auto& item : mConferenceList) {
        if (item.jid == groupId.toStdString()) {
            item.name = alias;
            update = true;
            break;
        }
    }
    // 存储书签列表
    if (update) {
        bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);
        qDebug() << "Store the bookmarks：" << groupId;
    }
}

void IM::setRoomDesc(const QString& groupId, const std::string& desc) {
    qDebug() << __func__ << groupId << desc.c_str();
    const IMRoomInfo* pRoomInfo = findRoom(groupId);
    if (!pRoomInfo) {
        qDebug() << "room is not exist." << groupId;
        return;
    }

    auto info = const_cast<IMRoomInfo*>(pRoomInfo);
    info->changes.insert(std::make_pair("muc#roomconfig_roomdesc", desc));
    // 获取新配置（handleMUCConfigForm处理）
    info->room->requestRoomConfig();
}

#ifdef WANT_PING
void IM::handlePing(const gloox::PingHandler::PingType type, const std::string& body) {
    qDebug() << "ping" << type << qstring(body);
    if (type != websocketPong) {
        IQ iq(IQ::IqType::Result, JID(_host.toStdString()));
        _client->send(iq);
    }
}
#endif

/**
 * 处理个人信息（VCard）
 * @param jid
 * @param vcard
 */
void IM::handleVCard(const JID& jid, const VCard* vcard) {
    qDebug() << __func__ << QString("jid：%1").arg(qstring(jid.full()));

    auto& photo = vcard->photo();
    if (!photo.binval.empty()) {
        qDebug() << QString("photo binval size:%1").arg(photo.binval.size());
        emit receiveFriendAvatarChanged(qstring(jid.bare()), photo.binval);
    }

    //  auto &nickname = vcard->nickname();
    //  if (!nickname.empty()) {
    //    qDebug()<<QString("nickname:%1").arg(qstring(nickname));
    //    emit receiveNicknameChange(qstring(jid.bare()), qstring(nickname));
    //  }
}

void IM::handleVCardResult(VCardContext context, const JID& jid, StanzaError error) {
    qDebug() << QString("context:%1 jid:%2").arg(context).arg(qstring(jid.full()));
    if (error) {
        return;
    }
}

void IM::fetchFriendVCard(const QString& friendId) {
    // 获取联系人个人信息
    qDebug() << "fetchVCard" << friendId;
    JID jid(stdstring(friendId));
    vCardManager->fetchVCard(jid, this);
    //  pubSubManager->subscribe(jid, "", this);
    //  _client->rosterManager()->subscribe(jid.bareJID());
}

void IM::handleTag(Tag* tag) { qDebug() << QString("tag：%1").arg(qstring(tag->xml())); }

bool IM::handleIq(const IQ& iq) {
    emit incoming(qstring(iq.tag()->xml()));
    return true;
}

void IM::handleIqID(const IQ& iq, int context) {}

void IM::requestBookmarks() {
    /**
     * bookmark
     */
    bookmarkStorage->registerBookmarkHandler(this);
    bookmarkStorage->requestBookmarks();
}

/**
 * @brief 处理群聊列表
 * @param bList
 * @param cList
 */
void IM::handleBookmarks(const BookmarkList& bList,    //
                         const ConferenceList& cList)  //
{
    qDebug() << __func__;
    qDebug() << "ConferenceList:" << cList.size();
    qDebug() << "BookmarkList:" << bList.size();

    //  缓存群聊书签列表（新增加群聊加入该书签一起保存）
    mConferenceList = cList;

    for (auto& c : cList) {
        auto name = (!c.name.empty() ? c.name : JID(c.jid).username());
        qDebug() << "room:" << qstring(name) << "jid:" << qstring(c.jid)
                 << "nick:" << qstring(c.nick) << "autojoin:" << c.autojoin;
        // 缓存到本地
        cacheJoinRoom(c.jid, name);
    }

    mBookmarkList = bList;
    for (auto& c : mBookmarkList) {
        qDebug() << "Bookmark name:" << qstring(c.name) << "url:" << qstring(c.url);
    }
}

void IM::handleBookmarks(const BMConferenceList& cList) {
    qDebug() << __func__;
    for (const auto& conf : cList) {
        qDebug() << "conference:" << qstring(conf.jid);
    }
}

// Disco handler
void IM::handleDiscoInfo(const JID& from,          //
                         const Disco::Info& info,  //
                         int context) {
    QString _from = QString::fromStdString(from.full());
    qDebug() << __func__ << _from << "context:" << (context);

    const StringList features = info.features();
    for (auto feature : features) {
        qDebug() << QString("feature=%1").arg(QString::fromStdString(feature));
    }

    const Disco::IdentityList& identities = info.identities();
    for (auto identity : identities) {
        qDebug() << QString("identity=%1").arg(qstring(identity->name()));
    }
}

/**
 * 获取服务发现
 * @param from
 * @param items
 * @param context
 */
void IM::handleDiscoItems(const JID& from,            //
                          const Disco::Items& items,  //
                          int context) {
    QString _from = QString::fromStdString(from.full());
    qDebug() << __func__ << "from" << _from << "context" << context;

    const Disco::ItemList& localItems = items.items();
    for (auto item : localItems) {
        if (context == DISCO_CTX_ROSTER) {
            pubSubManager->subscribe(item->jid(), XMLNS_NICKNAME, this);
#ifndef Q_OS_WIN
            pubSubManager->subscribe(item->jid(), XMLNS_AVATAR, this);
#endif  // !Q_OS_WIN
        }

        else if (context == DISCO_CTX_CONF) {
            /**
             * 处理服务发现的群聊列表
             */
            auto roomId = qstring(item->jid().bare());
            auto name = qstring(item->name());
            qDebug() << "room:" << name << roomId;
        }

        else if (context == DISCO_CTX_CONF_MEMBERS) {
            // 群聊成员
            auto memberId = qstring(item->jid().bare());
            qDebug() << "member:" << memberId;
        }
    }
}

void IM::handleDiscoError(const JID& from,            //
                          const gloox::Error* error,  //
                          int context) {
    QString _from = qstring(from.full());
    qDebug() << QString("from=%1 context=%2 error=%3")
                        .arg(_from)
                        .arg(context)
                        .arg(qstring(error->text()));
}

// Presence Handler
void IM::handlePresence(const Presence& presence) {
    qDebug() << __func__ << "from" << qstring(presence.from().full()) << "presence"
             << presence.presence();

    updateOnlineStatus(presence.from().bare(), presence.from().resource(), presence.presence());
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
void IM::handleItemAdded(const gloox::JID& jid) {
    qDebug() << __func__ << qstring(jid.full());

    auto m = _client->rosterManager();
    // 订阅对方
    m->subscribe(jid);

    auto item = m->getRosterItem(jid);
    if (!item) {
        qWarning() << "Unable to find roster.";
        return;
    }
    emit receiveFriend(IMFriend{item});
}

/**
 * 好友被删除事件
 * @param jid
 */
void IM::handleItemRemoved(const JID& jid) {
    qDebug() << __func__ << qstring(jid.full());
    emit receiveFriendRemoved(qstring(jid.bare()));
}

// 好友更新
void IM::handleItemUpdated(const JID& jid) {
    qDebug() << __func__ << qstring(jid.full());

    auto contactId = qstring(jid.bare());

    auto item = _client->rosterManager()->getRosterItem(jid);
    auto subType = item->subscription();
    qDebug() << __func__ << "subscription:" << (subType);
    auto data = item->data();
    qDebug() << "ask" << qstring(data->ask()) << "sub" << qstring(data->sub());

    if (data->sub() == "both") {
        qDebug() << "建立订阅双向关联";
        emit receiveFriendAliasChanged(jid, data->name());
    }
}

/**
 * 订阅好友
 * @param jid
 */
void IM::handleItemSubscribed(const JID& jid) { qDebug() << __func__ << qstring(jid.full()); }

/**
 * 取消订阅好友
 * @param jid
 */
void IM::handleItemUnsubscribed(const JID& jid) { qDebug() << __func__ << qstring(jid.full()); }

bool IM::removeFriend(const JID& jid) {
    qDebug() << __func__ << jid.full().c_str();

    /**
     * 参考：https://datatracker.ietf.org/doc/html/rfc3921#section-8.4.2
     */
    auto m = _client->rosterManager();
    // 取消订阅
    m->unsubscribe(jid);
    // 从联系人列表移除
    m->remove(jid);
    return true;
}

void IM::addFriend(const JID& jid, const QString& nick, const QString& msg) {
    qDebug() << __func__ << jid.full().c_str() << nick << msg;

    auto m = _client->rosterManager();
    auto f = m->getRosterItem(jid);
    if (f) {
        qWarning() << "IMFriend is existing!";
        return;
    }
    // 订阅对方(同时加入到联系人列表)
    m->subscribe(jid, stdstring(nick), {});
    m->add(jid, stdstring(nick), {});
    m->synchronize();
}

void IM::acceptFriendRequest(const QString& friendId) {
    qDebug() << __func__ << friendId;
    auto jid = JID(stdstring(friendId)).bareJID();

    auto m = _client->rosterManager();
    // 答复同意订阅
    m->ackSubscriptionRequest(jid, true);
    // 同时订阅对方
    m->subscribe(jid);
    // 添加到联系人列表
    m->add(jid, {}, {});
    m->synchronize();
}

void IM::rejectFriendRequest(const QString& friendId) {
    qDebug() << __func__ << friendId;
    _client->rosterManager()->ackSubscriptionRequest(JID(stdstring(friendId)).bareJID(), false);
}

size_t IM::getRosterCount() { return _client->rosterManager()->roster()->size(); }

void IM::getRosterList(std::list<IMFriend>& list) {
    auto rosterManager = _client->rosterManager();
    if (!rosterManager) {
        QMutexLocker locker(&m_mutex);
        rosterManager = enableRosterManager();
    }

    gloox::Roster* rosterMap = rosterManager->roster();
    for (const auto& itr : *rosterMap) {
        auto pItem = itr.second;
        list.push_back(IMFriend{pItem});
    }
}

void IM::setFriendAlias(const JID& jid, const std::string& alias) {
    qDebug() << __func__ << jid.bare().c_str() << alias.c_str();

    auto m = _client->rosterManager();

    // 保存联系人
    m->add(jid.bareJID(), alias, {});
    m->synchronize();
}

void IM::handleRoster(const Roster& roster) {
    qDebug() << __func__ << "size:" << roster.size();

    for (auto& it : roster) {
        auto& key = it.first;
        auto& item = it.second;

        qDebug() << "roster" << item->jid().full().c_str() << "subscription"
                 << item->subscription();
        if (item->jid().server().empty()) {
            qWarning() << "Ignore roster whithout server.";
            continue;
        }

        auto frnd = IMFriend{item};
        emit receiveFriend(frnd);

        //    Subscription sub(gloox::Subscription::Subscribe, jid);
        //    _client->send(sub);
    }
    //  enableDiscoManager();
    //  loadGroupList();
};

/**
 * 接收联系人状态信息（授予订阅请求）
 * @param item 联系人
 * @param resource 终端
 * @param presenceType
 * @param msg
 * @return
 */
void IM::handleRosterPresence(const RosterItem& item,               //
                              const std::string& resource,          //
                              Presence::PresenceType presenceType,  //
                              const std::string& msg) {
    qDebug() << QString("item:%1 resource:%2 presenceType:%3 msg:%4")
                        .arg(qstring(item.jid().full()))
                        .arg(qstring(resource))
                        .arg(presenceType)
                        .arg(qstring(msg));

    updateOnlineStatus(item.jid().bare(), resource, presenceType);

    //  if (presenceType == gloox::Presence::Available) {
    for (auto& it : item.resources()) {
        auto sk = it.first;
        auto sr = it.second;
        for (auto& ext : sr->extensions()) {
            switch (ext->extensionType()) {
                    //        case ExtCaps: {
                    //          auto caps = const_cast<Capabilities *>(
                    //              static_cast<const Capabilities *>(ext));
                    //          qDebug()<<QString("caps:%1").arg(qstring(caps->node())))
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
                            const_cast<VCardUpdate*>(static_cast<const VCardUpdate*>(ext));
                    if (vCardUpdate && vCardUpdate->hasPhoto()) {
                        ItemList items;
                        Item* item0 = new Item();
                        item0->setID(vCardUpdate->hash());
                        items.emplace_back(item0);

                        pubSubManager->requestItems(item.jid(), XMLNS_AVATAR, "", items, this);
                    }
                    break;
                }
            }
        }
    }
    //  }
}

void IM::handleSelfPresence(const RosterItem& item,               //
                            const std::string& resource,          //
                            Presence::PresenceType presenceType,  //
                            const std::string& msg) {
    qDebug() << QString("item:%1 resource:%2 presenceType:%3 msg:%4")  //
                        .arg(qstring(item.jid().full()))               //
                        .arg(qstring(resource))                        //
                        .arg(presenceType)                             //
                        .arg(qstring(msg));

    auto st = _client->resource() == resource ? presenceType : (int)gloox::Presence::Available;

    emit selfStatusChanged(st, msg);

    //  if (presenceType == gloox::Presence::Available) {
    for (auto& it : item.resources()) {
        auto sk = it.first;
        auto sr = it.second;
        for (auto& ext : sr->extensions()) {
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
                            const_cast<VCardUpdate*>(static_cast<const VCardUpdate*>(ext));
                    if (vCardUpdate && vCardUpdate->hasPhoto()) {
                        ItemList items;
                        Item* item0 = new Item();
                        item0->setID(vCardUpdate->hash());
                        items.emplace_back(item0);

                        pubSubManager->requestItems(item.jid().bareJID(), XMLNS_AVATAR, "", items,
                                                    this);
                    }
                    break;
                }
            }
        }
    }
};

/**
 * 好友订阅（加好友）请求
 * @param jid
 * @param msg
 * @return
 */
bool IM::handleSubscriptionRequest(const JID& jid, const std::string& msg) {
    qDebug() << __func__ << qstring(jid.full()) << qstring(msg);
    //  emit receiveFriendRequest(qstring(jid.bare()), qstring(msg));
    auto m = _client->rosterManager();
    // 订阅对方
    //  m->add(jid.bareJID(), jid.username(), {});
    m->subscribe(jid.bareJID(), jid.username(), {}, {});
    return true;
};

/**
 * 好友取消订阅（被删除）请求
 * @param jid
 * @param msg
 * @return
 */
bool IM::handleUnsubscriptionRequest(const JID& jid, const std::string& msg) {
    qDebug() << QString("jid:%1 msg:%2").arg(qstring(jid.full())).arg(qstring(msg));
    return true;
};

void IM::handleNonrosterPresence(const Presence& presence) {
    qDebug() << QString("presence:%1").arg(qstring(presence.from().full()));
};

void IM::handleRosterError(const IQ& iq) {
    qDebug() << QString("text:%1").arg(qstring(iq.error()->text()));
};

void IM::handleRosterItemExchange(const gloox::JID& from, const gloox::RosterX* items) {}

void IM::handleRegistrationFields(const JID& from, int fields, std::string instructions) {};

/**
 * This function is called if @ref Registration::createAccount() was called on
 * an authenticated stream and the server lets us know about this.
 */
void IM::handleAlreadyRegistered(const JID& from) {};

/**
 * This funtion is called to notify about the result of an operation.
 * @param from The server or service the result came from.
 * @param regResult The result of the last operation.
 */
void IM::handleRegistrationResult(const JID& from, RegistrationResult regResult,
                                  const Error* error) {};

/**
 * This function is called additionally to @ref handleRegistrationFields() if
 * the server supplied a data form together with legacy registration fields.
 * @param from The server or service the data form came from.
 * @param form The DataForm containing registration information.
 */
void IM::handleDataForm(const JID& from, const DataForm& form) {};

/**
 * This function is called if the server does not offer in-band registration
 * but wants to refer the user to an external URL.
 * @param from The server or service the referal came from.
 * @param oob The OOB object describing the external URL.
 */
void IM::handleOOB(const JID& from, const OOB& oob) {};

IMContactId IM::getSelfId() {
    IMContactId fId(qstring(_client->jid().bare()));
    return fId;
}

IMPeerId IM::getSelfPeerId() {
    return IMPeerId(qstring(_client->jid().full()));
    ;
}

QString IM::getSelfUsername() { return qstring(self().username()); }

void IM::setNickname(const QString& nickname) {
    qDebug() << QString("nickname:%1").arg(nickname);
    if (_nick == nickname) {
        return;
    }
    _nick = nickname;

    gloox::Nickname nick(nickname.toStdString());

    ItemList items;
    Item* item = new Item();
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

void IM::setAvatar(const QByteArray& avatar) {
    if (avatar.isEmpty()) return;

    QString sha1 = ok::base::Hashs::sha1(avatar);
    qDebug() << QString("avatar size:%1 sha1:%2").arg(avatar.size()).arg(sha1);

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
    Item* item = new Item();
    item->setID(sha1.toStdString());
    item->setPayload(avt.tag());
    items.emplace_back(item);

    pubSubManager->publishItem(JID(), XMLNS_AVATAR, items, nullptr, this);
}

void IM::changePassword(const QString& password) {
    qDebug() << QString("password:%1").arg(password);
    if (password.isEmpty()) return;

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

void IM::sendPresence() {
    //
    Presence pres(Presence::PresenceType::Available, JID());
    pres.addExtension(new Capabilities);
    _client->setPresence();
}

void IM::sendPresence(const JID& to, Presence::PresenceType type) {
    _client->setPresence(to, type, 0);
}

void IM::sendReceiptReceived(const QString& id, QString receiptNum) {
    // <received xmlns='urn:xmpp:receipts' id='richard2-4.1.247'/>

    Message m(gloox::Message::MessageType::Chat, JID(stdstring(id)));
    m.setFrom(_client->jid());

    m.addExtension(new Receipt(Receipt::ReceiptType::Received, receiptNum.toStdString()));

    _client->send(m);
}

void IM::sendServiceDiscoveryItems() {
    auto tag = new Tag("query");
    tag->setXmlns(XMLNS_DISCO_ITEMS);

    IQ* iq = new IQ(gloox::IQ::Get, self().server(), _client->getID());
    iq->setFrom(self());

    auto iqt = iq->tag();
    iqt->addChild(tag);
    _client->send(iqt);
}

void IM::sendServiceDiscoveryInfo(const JID& item) {
    auto tag = new Tag("query");
    tag->setXmlns(XMLNS_DISCO_INFO);

    IQ* iq = new IQ(gloox::IQ::Get, item, _client->getID());
    iq->setFrom(self());

    auto iqt = iq->tag();
    iqt->addChild(tag);
    _client->send(iqt);
}

void IM::handleItem(const JID& service, const std::string& node, const Tag* entry) {
    qDebug() << QString("service:%1 node:%2").arg(qstring(service.full())).arg(qstring(node));
}

void IM::handleItems(const std::string& id,                    //
                     const JID& service,                       //
                     const std::string& node,                  //
                     const gloox::PubSub::ItemList& itemList,  //
                     const gloox::Error* error) {
    qDebug() << __func__ << qstring(service.full()) << (qstring(node));

    if (error) {
        qWarning() << "error:" << error->tag()->xml().c_str();
        return;
    }

    auto friendId = qstring(service.bare());
    auto isSelf = friendId == getSelfId().toString();

    for (auto& item : itemList) {
        auto data = item->payload();
        auto tagName = data->name();

        qDebug() << "handleItem:" << qstring(tagName) << qstring(node);

        if (node == XMLNS_NICKNAME) {
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
            auto base64 = data->cdata();
            if (!base64.empty()) {
                std::string::size_type pos = 0;
                while ((pos = base64.find('\n')) != std::string::npos) base64.erase(pos, 1);
                while ((pos = base64.find('\r')) != std::string::npos) base64.erase(pos, 1);
                auto avt = Base64::decode64(base64);
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

void IM::handleItemPublication(const std::string& id,     //
                               const JID& service,        //
                               const std::string& node,   //
                               const ItemList& itemList,  //
                               const gloox::Error* error) {
    qDebug() << QString("node:%1").arg(qstring(node));
    if (node == XMLNS_AVATAR) {
        // 更新头像元信息
        //  https://xmpp.org/extensions/xep-0084.html#process-pubmeta
        for (auto& item : itemList) {
            qDebug() << QString("itemId:%1").arg(qstring(item->id()));
        }
    }
}

void IM::handleItemDeletion(const std::string& id, const JID& service, const std::string& node,
                            const ItemList& itemList, const gloox::Error* error) {
    qDebug() << QString("id:%1 service:%2").arg(qstring(id)).arg(qstring(service.full()));
}

void IM::handleSubscriptionResult(const std::string& id, const JID& service,
                                  const std::string& node, const std::string& sid, const JID& jid,
                                  const gloox::PubSub::SubscriptionType subType,
                                  const gloox::Error* error) {
    qDebug() << QString("id:%1 jid:%2").arg(qstring(id)).arg(qstring(jid.full()));
    pubSubManager->requestItems(service, node, sid, 100, this);
}

void IM::handleUnsubscriptionResult(const std::string& id, const JID& service,
                                    const gloox::Error* error) {}
void IM::handleSubscriptionOptions(const std::string& id, const JID& service, const JID& jid,
                                   const std::string& node, const DataForm* options,
                                   const std::string& sid, const gloox::Error* error) {}
void IM::handleSubscriptionOptionsResult(const std::string& id, const JID& service, const JID& jid,
                                         const std::string& node, const std::string& sid,
                                         const gloox::Error* error) {}
void IM::handleSubscribers(const std::string& id, const JID& service, const std::string& node,
                           const SubscriptionList& list, const gloox::Error* error) {}
void IM::handleSubscribersResult(const std::string& id, const JID& service, const std::string& node,
                                 const SubscriberList* list, const gloox::Error* error) {}

void IM::handleAffiliates(const std::string& id, const JID& service, const std::string& node,
                          const AffiliateList* list, const gloox::Error* error) {}

void IM::handleAffiliatesResult(const std::string& id, const JID& service, const std::string& node,
                                const AffiliateList* list, const gloox::Error* error) {}
void IM::handleNodeConfig(const std::string& id, const JID& service, const std::string& node,
                          const DataForm* config, const gloox::Error* error) {}
void IM::handleNodeConfigResult(const std::string& id, const JID& service, const std::string& node,
                                const gloox::Error* error) {}
void IM::handleNodeCreation(const std::string& id, const JID& service, const std::string& node,
                            const gloox::Error* error) {}
void IM::handleNodeDeletion(const std::string& id, const JID& service, const std::string& node,
                            const gloox::Error* error) {}
void IM::handleNodePurge(const std::string& id, const JID& service, const std::string& node,
                         const gloox::Error* error) {}
void IM::handleSubscriptions(const std::string& id, const JID& service,
                             const SubscriptionMap& subMap, const gloox::Error* error) {}
void IM::handleAffiliations(const std::string& id, const JID& service, const AffiliationMap& affMap,
                            const gloox::Error* error) {}

void IM::handleDefaultNodeConfig(const std::string& id, const JID& service, const DataForm* config,
                                 const gloox::Error* error) {
    Q_UNUSED(config);
    qDebug() << QString("id:%1 service:%2 error:%3")
                        .arg(qstring(id))
                        .arg(qstring(service.full()))
                        .arg(qstring(error->text()));
}

std::string IM::getOnlineResource(const std::string& bare) {
    auto it = onlineMap.find(bare);
    if (it == onlineMap.end()) {
        return std::string{};
    }
    for (auto r : it->second) {
        return r;
    }
    return std::string{};
}

std::set<std::string> IM::getOnlineResources(const std::string& bare) {
    auto it = onlineMap.find(bare);
    if (it != onlineMap.end()) {
        return it->second;
    }
    return {};
}

void IM::updateOnlineStatus(const std::string& bare, const std::string& resource,
                            Presence::PresenceType presenceType) {
    if (presenceType == Presence::Error) {
        qWarning() << "Ignore error presence.";
        return;
    }
    if (resource.empty()) {
        qWarning() << "Ignore resource is empty.";
        return;
    }

    auto friendId = qstring(bare);
    int status = -1;

    auto it = onlineMap.find(bare);
    if (it == onlineMap.end()) {  // 第一次
        if (presenceType != gloox::Presence::Unavailable) {
            std::set<std::string> resources;
            resources.insert(resource);
            onlineMap.emplace(bare, resources);
            status = gloox::Presence::Available;
        }
    } else {  // 第二次+
        std::set<std::string>& resources = it->second;
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

void IM::retry() { doConnect(); }

bool IM::leaveGroup(const QString& groupId) {
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
    mConferenceList.remove_if([&](ConferenceListItem& a) { return a.jid == stdstring(groupId); });
    bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);

    return true;
}

bool IM::destroyGroup(const QString& groupId) {
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
    mConferenceList.remove_if([&](ConferenceListItem& a) { return a.jid == stdstring(groupId); });
    bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);

    return true;
}

Disco::ItemList IM::handleDiscoNodeItems(const JID& from, const JID& to, const std::string& node) {
    qDebug() << QString("from:%1").arg(from.full().c_str());

    return gloox::Disco::ItemList();
}

Disco::IdentityList IM::handleDiscoNodeIdentities(const JID& from, const std::string& node) {
    return gloox::Disco::IdentityList();
}

StringList IM::handleDiscoNodeFeatures(const JID& from, const std::string& node) {
    return gloox::StringList();
}


void IM::handleIncoming(gloox::Tag* tag) {
    //  auto services = tag->findChild("services", XMLNS, XMLNS_EXTERNAL_SERVICE_DISCOVERY);
    //  if (services) {
    //    mExtDisco = ExtDisco(services);
    //  }
    //  emit incoming(::base::Xmls::parse(qstring(tag->xml())));
}

bool IM::onTLSConnect(const CertInfo& info) {
    qDebug() << QString("CertInfo:");

    time_t from(info.date_from);
    time_t to(info.date_to);

    qDebug() << QString("status: %1\n"    //
                        "issuer: %2\n"    //
                        "peer: %3\n"      //
                        "protocol: %4\n"  //
                        "mac: %5\n"       //
                        "cipher: %6\n"    //
                        "compression: %7\n")
                        .arg((info.status))                       //
                        .arg(qstring(info.issuer.c_str()))        //
                        .arg(qstring(info.server.c_str()))        //
                        .arg((info.protocol.c_str()))             //
                        .arg(qstring(info.mac.c_str()))           //
                        .arg(qstring(info.cipher.c_str()))        //
                        .arg(qstring(info.compression.c_str()));  //

    qDebug() << QString("from:%1").arg(ctime(&from));
    qDebug() << QString("to:%1").arg(ctime(&to));

    return true;
}

void IM::handleLog(LogLevel level, LogArea area, const std::string& message) {
    //   qDebug()<<QString("%1").arg(message.c_str()));
    auto line = QString::fromStdString(message);
    switch (area) {
        case LogAreaXmlIncoming:
            qDebug() << "Received XML:" << line;
            break;
        case LogAreaXmlOutgoing:
            qDebug() << "Sent XML:" << line;
            break;
        case LogAreaClassConnectionBOSH:
            qDebug() << "BOSH:" << line;
            break;
        case LogAreaClassClient:
            qDebug() << "Client:" << line;
            break;
        case LogAreaClassDns:
            qDebug() << "dns:" << line;
            break;
        default:
            qDebug() << QString("level: %1, area: %2 msg: %3").arg(level).arg(area).arg(line);
    }
}

IMStatus IM::getFriendStatus(const QString& qString) {
    return onlineMap.find(qString.toStdString()) != onlineMap.end() ? IMStatus::Available
                                                                    : IMStatus::Unavailable;
}

void IM::requestFriendNickname(const JID& friendId) {
    qDebug() << __func__ << friendId.full().c_str();
    pubSubManager->subscribe(friendId, XMLNS_NICKNAME, this);
}

}  // namespace lib::messenger
