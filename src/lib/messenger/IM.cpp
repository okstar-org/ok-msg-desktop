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

#include "IMFromHostHandler.h"
#include "base/hashs.h"
#include "base/system/sys_info.h"
#include "base/times.h"
#include "base/xmls.h"

#include <jinglegroup.h>
#include <list>
#include <string>
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
#include <QDebug>
#include <range/v3/all.hpp>

namespace lib::messenger {

#define DISCO_CTX_CONF 0
#define DISCO_CTX_ROSTER 1
#define DISCO_CTX_BOOKMARKS 2
#define DISCO_CTX_CONF_MEMBERS 3

gloox::ConferenceList mConferenceList;
gloox::BookmarkList mBookmarkList;

/**
 * 聊天通讯核心类
 * @param user
 * @param pwd
 * @param features_
 */
IM::IM(std::string host,
       std::string user,
       std::string pwd,
       std::vector<std::string>
               features_)  //
        : features(std::move(features_))
        , _host(std::move(host))
        , _username(std::move(user))
        , _password(std::move(pwd)) {
    qDebug() << __func__;
    auto osInfo = ok::base::SystemInfo::instance()->osInfo();
    discoVal = stdstring(osInfo.prettyName);
    // 生成本机resource. 格式:OkEDU.<HOST>.[VER].[UNIQUE]
    _resource = APPLICATION_ALIAS "." + stdstring(osInfo.hostName) + "." + GIT_DESCRIBE + "." +
                stdstring(osInfo.uniqueId.mid(0, 6));  //
    qDebug() << "resource:" << _resource.c_str();
}

IM::~IM() {}

std::unique_ptr<gloox::Client> IM::makeClient() {
    //    qDebug() << __func__;
    loginJid = gloox::JID(_username + "@" + _host + "/" + _resource);
    //    qDebug() << __func__ << "Using Jid:" << qstring(loginJid.full());

    /**
     * Client
     */
    auto client = std::make_unique<gloox::Client>(loginJid, _password);
    auto disco = client->disco();

    disco->setVersion("disco", GIT_DESCRIBE, discoVal);
    disco->setIdentity("client", "pc", GIT_VERSION);
    disco->addIdentity("pubsub", "pep");
    disco->addFeature(gloox::XMLNS_PUBSUB);
    disco->addFeature(gloox::XMLNS_PUBSUB_EVENT);
    disco->addFeature(gloox::XMLNS_PUBSUB_OWNER);
    disco->addFeature(gloox::XMLNS_PUBSUB_PUBLISH_OPTIONS);
    disco->addFeature(gloox::XMLNS_PUBSUB_AUTO_SUBSCRIBE);
    disco->addFeature(gloox::XMLNS_PUBSUB_AUTO_CREATE);
    client->registerStanzaExtension(new gloox::Disco::Items);
    client->registerStanzaExtension(new gloox::PubSub::Event());

    client->registerStanzaExtension(new gloox::VCardUpdate);
    client->registerStanzaExtension(new gloox::Capabilities);
    client->registerStanzaExtension(new gloox::InBandBytestream::IBB);
    client->registerStanzaExtension(new gloox::ChatState(nullptr));
    client->registerStanzaExtension(new gloox::Receipt(nullptr));
    client->registerStanzaExtension(new gloox::Forward());
    client->registerStanzaExtension(new gloox::Carbons());
    client->registerStanzaExtension(new gloox::Attention());
    client->registerStanzaExtension(new gloox::DelayedDelivery());

    // XEP-0215: External Service Discovery
    client->registerStanzaExtension(new gloox::ExtDisco());
    client->registerIqHandler(this, gloox::ExtSrvDisco);

    client->registerStanzaExtension(new gloox::Addresses());
    client->registerStanzaExtension(new gloox::Nickname(nullptr));

    /**
     *
     */
    disco->addFeature(gloox::XMLNS_X_CONFERENCE);
    client->registerStanzaExtension(new gloox::Conference);

    /**
     * XEP-0402: PEP Native Bookmarks
     * urn:xmpp:bookmarks:1
     */
    disco->addFeature(gloox::XMLNS_BOOKMARKS2);
    disco->addFeature(gloox::XMLNS_BOOKMARKS2_NOTIFY);
    disco->addFeature(gloox::XMLNS_BOOKMARKS2_COMPAT);
    disco->addFeature("urn:xmpp:bookmarks-conversion:0");
    m_nativeBookmark = std::make_unique<gloox::NativeBookmarkStorage>(client.get());
    m_nativeBookmark->registerBookmarkHandler(this);

    for (const auto& feat : features) {
        disco->addFeature((feat));
    }

    /**
     * 聊天相关
     */
    disco->addFeature(gloox::XMLNS_CHAT_STATES);

    disco->addFeature(gloox::XMLNS_MUC);
    disco->addFeature(gloox::XMLNS_MUC_ADMIN);
    disco->addFeature(gloox::XMLNS_MUC_OWNER);
    disco->addFeature(gloox::XMLNS_MUC_ROOMS);
    disco->addFeature(gloox::XMLNS_MUC_ROOMINFO);
    disco->addFeature(gloox::XMLNS_MUC_USER);
    disco->addFeature(gloox::XMLNS_MUC_UNIQUE);
    disco->addFeature(gloox::XMLNS_MUC_REQUEST);

    disco->addFeature(gloox::XMLNS_DISCO_INFO);
    disco->addFeature(gloox::XMLNS_DISCO_ITEMS);
    disco->addFeature(gloox::XMLNS_DISCO_PUBLISH);
    disco->addFeature(gloox::XMLNS_CAPS);

    disco->addFeature(gloox::XMLNS_STANZA_FORWARDING);

    disco->addFeature(gloox::XMLNS_BOOKMARKS);
    disco->addFeature(gloox::XMLNS_PRIVATE_XML);
    // XMLNS_RECEIPTS
    disco->addFeature(gloox::XMLNS_RECEIPTS);
    disco->addFeature(gloox::XMLNS_MESSAGE_CARBONS);
    disco->addFeature("urn:xmpp:carbons:rules:0");
    disco->addFeature(gloox::XMLNS_ADDRESSES);

    // NICK
    disco->addFeature(gloox::XMLNS_NICKNAME);
    disco->addFeature(gloox::XMLNS_NICKNAME + "+notify");

    // FILE
    disco->addFeature(gloox::XMLNS_IBB);
    // jingle av
    disco->addFeature(gloox::XMLNS_JINGLE);
    disco->addFeature(gloox::XMLNS_JINGLE_MESSAGE);
    disco->addFeature(gloox::XMLNS_JINGLE_ERRORS);
    disco->addFeature(gloox::XMLNS_JINGLE_ICE_UDP);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_DTLS);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_DTLS_SCTP);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP);
    disco->addFeature(gloox::XMLNS_JINGLE_FEATURE_AUDIO);
    disco->addFeature(gloox::XMLNS_JINGLE_FEATURE_VIDEO);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_FB);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_HDREXT);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_GROUP);
    client->registerStanzaExtension(new gloox::Jingle::JingleMessage());
    // jingle file

    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER4);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER5);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER_MULTI);
    disco->addFeature(gloox::XMLNS_JINGLE_IBB);

    client->setTls(gloox::TLSPolicy::TLSDisabled);
    client->setCompression(false);
    client->registerIqHandler(this, gloox::ExtPubSub);

    /**
     * listeners
     */
    client->registerConnectionListener(this);
    client->registerPresenceHandler(this);
    client->registerMessageSessionHandler(this);
    client->registerMessageHandler(this);
    //  client->setStreamManagement(true, true);

#ifdef LOG_XMPP
    client->logInstance().registerLogHandler(gloox::LogLevelDebug, gloox::LogAreaAll, this);
#endif

    vCardManager = std::make_unique<gloox::VCardManager>(client.get());
    pubSubManager = std::make_unique<gloox::PubSub::Manager>(client.get());
    bookmarkStorage = std::make_unique<gloox::BookmarkStorage>(client.get());
    // session manager
    _sessionManager = std::make_unique<gloox::Jingle::SessionManager>(client.get(), this);
    _sessionManager->registerPlugin(new gloox::Jingle::Content());
    _sessionManager->registerPlugin(new gloox::Jingle::ICEUDP());
    _sessionManager->registerPlugin(new gloox::Jingle::Group());
    _sessionManager->registerPlugin(new gloox::Jingle::RTP());

    _sessionManager->registerPlugin(new gloox::Jingle::Content());
    _sessionManager->registerPlugin(new gloox::Jingle::FileTransfer());
    _sessionManager->registerPlugin(new gloox::Jingle::IBB());

    return std::move(client);
}

void IM::start() {
    qDebug() << __func__;
    mutex.lock();

    for (auto h : imHandlers) {
        h->onConnecting();
    }

    _client = makeClient();
    // block the current thread
    _client->connect(true);
}

bool IM::isStarted() const {
    return _client.get();
}

void IM::stop() {
    std::lock_guard<std::mutex> locker(mutex);

    doDisconnect();

    for (auto h : imHandlers) {
        h->onStopped();
    }
}

void IM::onDisconnect(gloox::ConnectionError e) {
    qDebug() << __func__ << "error:" << e;

    isConnected = false;
    for (auto h : imHandlers) {
        h->onDisconnected((int)e);
    }
}

void IM::onConnect() {
    qDebug() << __func__ << "connected";
    if (isConnected) {
        return;
    }

    isConnected = true;

    auto res = _client->resource();
    qDebug() << __func__ << "resource:" << res.c_str();

    pubSubManager->subscribe(_client->jid(), gloox::XMLNS_NICKNAME, this);
    pubSubManager->subscribe(_client->jid(), gloox::XMLNS_AVATAR, this);

    requestVCards();
    // emit selfIdChanged(qstring(_client->username()));

    //   enable carbons（多终端支持）
    gloox::IQ iq(gloox::IQ::IqType::Set, gloox::JID(), "server");
    iq.addExtension(new gloox::Carbons(gloox::Carbons::Enable));
    _client->send(iq);

    // request ext server disco
    gloox::IQ iq2(gloox::IQ::Get, gloox::JID(_host));
    auto t = iq2.tag();
    t->addChild(gloox::ExtDisco::newRequest());
    _client->send(t);

    auto rosterManager = _client->rosterManager();
    if (!rosterManager) {
        rosterManager = enableRosterManager();
    }

    mutex.unlock();

    for (auto h : imHandlers) {
        h->onConnected();
        h->onStarted();
    }
}

void IM::send(const std::string& xml) {
    if (xml.empty()) {
        qWarning() << "send xml content is empty";
        return;
    }
    _client->send(xml);
    _client.reset();
}

IMMessage IM::fromXMsg(MsgType type, const gloox::Message& msg) {
    IMMessage imMsg = {.from = (msg.from().full()),
                       .to = (msg.to().full()),
                       .body = (msg.body()),
                       .timestamp = ok::base::Times::now().toSecsSinceEpoch()};
    if (!msg.id().empty()) {
        imMsg.id = ((msg.id()));
    }
    return imMsg;
}

void IM::enableDiscoManager() {
    qDebug() << "enableDiscoManager";
    auto client = _client.get();
    auto disco = client->disco();
    for (const auto& feat : features) {
        disco->addFeature((feat));
    }

    /**
     * 头像相关
     */
    client->registerStanzaExtension(new gloox::Avatar);
    // urn:xmpp:avatar:data
    disco->addFeature(gloox::XMLNS_AVATAR);
    // urn:xmpp:avatar:metadata
    disco->addFeature(gloox::XMLNS_META_AVATAR);
    // urn:xmpp:avatar:metadata+notify
    disco->addFeature(gloox::XMLNS_META_AVATAR + "+notify");

    disco->addFeature(gloox::XMLNS_CHAT_STATES);
    disco->addFeature(gloox::XMLNS_MUC);
    disco->addFeature(gloox::XMLNS_MUC_ADMIN);
    disco->addFeature(gloox::XMLNS_MUC_OWNER);
    disco->addFeature(gloox::XMLNS_MUC_ROOMS);
    disco->addFeature(gloox::XMLNS_MUC_ROOMINFO);
    disco->addFeature(gloox::XMLNS_MUC_USER);
    disco->addFeature(gloox::XMLNS_MUC_UNIQUE);
    disco->addFeature(gloox::XMLNS_MUC_REQUEST);
    disco->addFeature(gloox::XMLNS_DISCO_INFO);
    disco->addFeature(gloox::XMLNS_DISCO_ITEMS);
    disco->addFeature(gloox::XMLNS_DISCO_PUBLISH);
    disco->addFeature(gloox::XMLNS_CAPS);
    disco->addFeature(gloox::XMLNS_STANZA_FORWARDING);

    disco->addFeature(gloox::XMLNS_BOOKMARKS);
    disco->addFeature(gloox::XMLNS_PRIVATE_XML);
    // XMLNS_RECEIPTS
    disco->addFeature(gloox::XMLNS_RECEIPTS);
    disco->addFeature(gloox::XMLNS_MESSAGE_CARBONS);
    disco->addFeature("urn:xmpp:carbons:rules:0");
    disco->addFeature(gloox::XMLNS_ADDRESSES);

    // 基本Jingle功能
    disco->addFeature(gloox::XMLNS_IBB);
    disco->addFeature(gloox::XMLNS_JINGLE);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER4);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER5);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER_MULTI);

    disco->addFeature(gloox::XMLNS_JINGLE_IBB);
    disco->addFeature(gloox::XMLNS_JINGLE_ERRORS);
    disco->addFeature(gloox::XMLNS_JINGLE_ICE_UDP);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_DTLS);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_DTLS_SCTP);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP);
    disco->addFeature(gloox::XMLNS_JINGLE_FEATURE_AUDIO);
    disco->addFeature(gloox::XMLNS_JINGLE_FEATURE_VIDEO);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_FB);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_HDREXT);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_GROUP);
    disco->addFeature(gloox::XMLNS_JINGLE_MESSAGE);
    // JIT_MEET
    disco->addFeature(gloox::XMLNS_JIT_MEET);

    /**
     * 昵称 NICK
     */
    disco->addFeature(gloox::XMLNS_NICKNAME);
    disco->addFeature(gloox::XMLNS_NICKNAME + "+notify");
    client->registerStanzaExtension(new gloox::Nickname(nullptr));

    client->registerStanzaExtension(new gloox::VCardUpdate);
    client->registerStanzaExtension(new gloox::Capabilities);
    client->registerStanzaExtension(new gloox::Disco::Items);

    client->registerStanzaExtension(new gloox::Jingle::JingleMessage());
    client->registerStanzaExtension(new gloox::InBandBytestream::IBB);
    client->registerStanzaExtension(new gloox::ChatState(nullptr));
    client->registerStanzaExtension(new gloox::Receipt(nullptr));
    client->registerStanzaExtension(new gloox::Forward());
    client->registerStanzaExtension(new gloox::Carbons());
    client->registerStanzaExtension(new gloox::Attention());
    client->registerStanzaExtension(new gloox::DelayedDelivery());
    client->registerStanzaExtension(new gloox::ExtDisco());
    client->registerStanzaExtension(new gloox::Addresses());

    disco->registerDiscoHandler(this);
    disco->registerNodeHandler(this, gloox::EmptyString);
}

gloox::RosterManager* IM::enableRosterManager() {
    qDebug() << __func__;
    /**
     * roster
     */
    auto pRosterManager = _client->enableRoster();
    pRosterManager->registerRosterListener(this);
    return pRosterManager;

    //
    //  /**
    //   * Registration
    //   */
    //  mRegistration = std::make_unique<Registration>(_client.get());
    //  mRegistration->registerRegistrationHandler(this);
}

void IM::doConnect() {
    qDebug() << __func__;
    _client->connect(true);
}

void IM::doDisconnect() {
    assert(_client.get());
    _client->disconnect();
}

QDomDocument IM::buildMessage(const std::string& to,   //
                              const std::string& msg,  //
                              const std::string& id) {
    gloox::Message m(gloox::Message::MessageType::Chat, gloox::JID(to), msg);
    m.setFrom(_client->jid());
    m.setID((id));
    m.addExtension(new gloox::Receipt(gloox::Receipt::Request));
    return ok::base::Xmls::parse(qstring(m.tag()->xml()));
}

bool IM::sendTo(const std::string& friendId, const std::string& msg, const std::string& id) {
    qDebug() << __func__ << friendId.c_str() << "msgId:" << id.c_str() << "msg:" << msg.c_str();

    gloox::Message m(gloox::Message::MessageType::Chat, gloox::JID(friendId), msg);
    m.setFrom(_client->jid());
    m.setID(id);
    m.addExtension(new gloox::Receipt(gloox::Receipt::Request));

    _client->send(m);
    return true;
}

// Handle Message session
void IM::handleMessageSession(gloox::MessageSession* session) {
    auto from = session->target();
    auto sid = (session->threadID());
    qDebug() << __func__ << "from:" << from.full().c_str() << "sid:" << sid.c_str();

    // 联系人ID（朋友和群聊）
    auto iterator = fromHostHandlers.find(from.server());
    if (iterator != fromHostHandlers.end()) {
        iterator->second->handleHostMessageSession(from, sid);
        return;
    }

    auto contactId = (session->target().bare());
    for (auto handler : friendHandlers) {
        handler->onMessageReceipt(contactId, (sid));
    }

    // 聊天状态过滤器，获取：正在中等输入状态
    auto it = m_chatStateFilters.find(contactId);

    if (it != m_chatStateFilters.end()) {
        auto csf = it->second;
        csf = new gloox::ChatStateFilter(session);
        csf->registerChatStateHandler(this);
        m_chatStateFilters.insert(std::make_pair(contactId, csf));
    }
}

void IM::handleMessage(const gloox::Message& msg, gloox::MessageSession* session) {
    auto from = msg.from();
    auto peerId = from.full();
    auto friendId = from.bare();
    auto body = msg.body();

    qDebug() << __func__ << "from:" << peerId.c_str() << "subtype:" << (int)msg.subtype();

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

    auto conf = msg.findExtension<gloox::Conference>(gloox::ExtConference);
    if (conf) {
        requestBookmarks();
    }

    auto chatState = msg.findExtension<gloox::ChatState>(gloox::ExtChatState);
    if (chatState) {
        handleChatState(msg.from(), chatState->state());
    }

    auto pEvent = msg.findExtension<gloox::PubSub::Event>(gloox::ExtPubSubEvent);
    if (pEvent) {
        doPubSubEvent(pEvent, msg, friendId);
        //        emit doPubSubEventDone();
    }

    auto delay = msg.when();
    if (delay) {
        qDebug() << "delay:" << delay->stamp().c_str();
    }

    auto address = msg.address();
    if (address) {
        for (const auto& item : address->addresses()) {
            qDebug() << "address type:" << (item.type.c_str())
                     << "jid:" << (item.jid.full().c_str());
        }
    }

    auto iterator = fromHostHandlers.find(from.server());
    if (iterator != fromHostHandlers.end()) {
        iterator->second->handleHostMessage(from, msg);
        return;
    }

    // TODO 暂时注释
    //     auto xml = qstring(msg.tag()->xml());
    //     emit incoming(xml);
}

void IM::doMessageHeadline(const gloox::Message& msg, std::string& friendId,
                           const std::string& body) {
    auto mu = msg.findExtension<gloox::MUCRoom::MUCUser>(gloox::ExtMUCUser);
    if (mu) {
        // 群聊邀请
        if (gloox::MUCRoom::OpInviteFrom == mu->operation()) {
            for (auto handler : groupHandlers) {
                handler->onGroupInvite(gloox::JID(*mu->jid()).username(), friendId, body);
            }
        }
    }
}

/**
 * 处理聊天消息
 *
 */
void IM::doMessageChat(const gloox::Message& msg, std::string& friendId, const std::string& body) {
    qDebug() << __func__ << "from:" << friendId.c_str() << body.c_str();
    if (!body.empty()) {
        if (!msg.encrypted()) {
            for (auto handler : friendHandlers) {
                handler->onFriendMessage(friendId, fromXMsg(MsgType::Chat, msg));
            }
        } else {
            qDebug() << "Is encrypted message:" << msg.tag()->xml().c_str();
            //            emit exportEncryptedMessage(xml);
        }
    }
    // 从 message.receipt 提取接收确认ID
    auto pReceipt = msg.findExtension<gloox::Receipt>(gloox::ExtReceipt);
    if (pReceipt && !pReceipt->id().empty()) {
        for (auto handler : friendHandlers) {
            handler->onFriendMessageReceipt(friendId, pReceipt->id());
        }
    }

    // forwarded/message
    auto pCarbons = msg.findExtension<gloox::Carbons>(gloox::ExtCarbons);
    if (pCarbons && pCarbons->embeddedStanza()) {
        auto* eMsg = static_cast<gloox::Message*>(pCarbons->embeddedStanza());
        if (eMsg) {
            auto ebody = eMsg->body();
            if (!ebody.empty()) {
                auto imMessage = fromXMsg(MsgType::Chat, *eMsg);
                imMessage.to = eMsg->to().full();
                for (auto handler : friendHandlers) {
                    handler->onFriendMessage(imMessage.to, imMessage);
                }
            }
        }
    }
}

/**
 * 处理常规消息
 * @param msg
 * @param friendId
 */
void IM::doMessageNormal(const gloox::Message& msg, std::string& friendId) {
    auto conf = msg.findExtension<gloox::Conference>(gloox::ExtConference);
    if (conf) {
        auto jid = conf->jid().full();
        cacheJoinRoom(jid);
    }
}

void IM::handleMessageEvent(const gloox::JID& from, const gloox::MessageEvent* et) {
    qDebug() << __func__ << "from:" << from.full().c_str() << "MessageEvent:" << et;
}

void IM::doPubSubEvent(const gloox::PubSub::Event* pse,  //
                       const gloox::Message& msg,        //
                       std::string& friendId) {          //
    qDebug() << __func__ << friendId.c_str();

    const std::string& selfId = getSelfId().toString();
    auto isSelf = friendId == selfId;

    for (auto& item : pse->items()) {
        auto nickTag = item->payload->findChild("nick");
        if (nickTag) {
            gloox::Nickname nickname(nickTag);
            auto newNick = nickname.nick();
            if (isSelf) {
                if (_nick != newNick) {
                    _nick = newNick;
                    for (auto handler : selfHandlers) {
                        handler->onSelfNameChanged(newNick);
                    }
                }
            } else {
                for (auto handler : friendHandlers) {
                    handler->onFriendNickChanged(friendId, newNick);
                }
            }
        }
        auto avatarData = item->payload->findChild("data", gloox::XMLNS, gloox::XMLNS_AVATAR);
        if (avatarData) {
            auto base64 = avatarData->cdata();
            if (!base64.empty()) {
                std::string::size_type pos = 0;
                while ((pos = base64.find('\n')) != std::string::npos) base64.erase(pos, 1);
                while ((pos = base64.find('\r')) != std::string::npos) base64.erase(pos, 1);

                auto avt = gloox::Base64::decode64(base64);
                if (isSelf) {
                    qDebug() << "Receive self avatar size" << avt.size();
                    for (auto handler : selfHandlers) {
                        handler->onSelfAvatarChanged(avt);
                    }
                } else {
                    qDebug() << "Receive friend avatar" << friendId.c_str() << "size" << avt.size();
                    for (auto handler : friendHandlers) {
                        handler->onFriendAvatarChanged(friendId, avt);
                    }
                }
            }
        }

        auto avatarMetaData = item->payload->findChild("metadata",    //
                                                       gloox::XMLNS,  //
                                                       gloox::XMLNS_META_AVATAR);
        if (avatarMetaData) {
            auto itemId = avatarMetaData->findChild("info")->findAttribute("id");
            gloox::PubSub::ItemList items;
            auto item0 = new gloox::PubSub::Item();
            item0->setID(itemId);
            items.emplace_back(item0);

            if (pubSubManager) {
                pubSubManager->requestItems(msg.from(), gloox::XMLNS_AVATAR, "", items, this);
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
void IM::handleChatState(const gloox::JID& from, gloox::ChatStateType state) {
    qDebug() << __func__ << qstring(from.full()) << "state:" << static_cast<int>(state);
    for (auto handler : friendHandlers) {
        handler->onFriendChatState(from.full(), state);
    }
}

/**
 * 发送给朋友当前聊天状态
 * @param to
 * @param state 聊天状态
 */
void IM::sendChatState(const std::string& to, gloox::ChatStateType state) {
    qDebug() << __func__ << "to:" << to.c_str() << "state:" << (static_cast<int>(state));
    auto csf = m_chatStateFilters[to];
    if (!csf) {
        qWarning() << "Chat state filter is no existing!";
        return;
    }
    csf->setChatState(state);
}

// MUC handler -- handleMUCParticipantPresence
void IM::handleMUCParticipantPresence(gloox::MUCRoom* room,                         //
                                      const gloox::MUCRoomParticipant participant,  //
                                      const gloox::Presence& presence) {
    qDebug() << __func__;

    auto groupId = room->jid().full();
    auto nick = participant.nick->resource();
    auto tag = presence.tag();

    auto nickTag = tag->findChild("nick");
    if (nickTag) nick = (nickTag->cdata());

    auto x = tag->findChild("x", gloox::XMLNS, gloox::XMLNS_MUC_USER);
    if (x) {
        IMGroupOccupant occ = {.nick = nick};

        auto item = x->findChild("item");
        if (item) {
            occ.jid = item->findAttribute("jid");
            occ.affiliation = item->findAttribute("affiliation");
            occ.role = item->findAttribute("role");
            occ.status = presence.presence();
        }

        for (auto t : x->findChildren("status")) {
            occ.codes.push_back(qstring(t->findAttribute("code")).toInt());
        }

        for (auto handler : groupHandlers) {
            handler->onGroupOccupantStatus(groupId, occ);
        }
    }
}

void IM::handleMUCMessage(gloox::MUCRoom* room, const gloox::Message& msg, bool priv) {
    auto msgId = msg.id();
    auto roomId = room->jid().full();
    auto from = msg.from().full();
    auto body = msg.body();
    qDebug() << __func__ << "roomId:" << roomId.c_str() << "msg:" << body.c_str();

    IMPeerId peerId(from);
    for (const auto& id : sendIds) {
        if (id == msg.id()) {
            qWarning() << "Ignore messages from oneself";
            sendIds.erase(id);
            return;
        }
    }

    auto mu = msg.findExtension<gloox::MUCRoom::MUCUser>(gloox::ExtMUCUser);
    if (mu) {
        if (mu->flags() & gloox::UserRoomConfigurationChanged) {
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
        if (gloox::MUCRoom::OpInviteFrom == mu->operation()) {
            for (auto h : groupHandlers) {
                h->onGroupInvite((gloox::JID(*mu->jid()).bare()), roomId, (msg.body()));
            }
        }

        return;
    }

    if (body.empty()) {
        return;
    }

    IMMessage imMsg = fromXMsg(MsgType::Groupchat, msg);
    const gloox::DelayedDelivery* dd = msg.when();
    if (dd) {
        // yyyy-MM-dd HH:mm:ss 20230614T12:11:43Z
        imMsg.timestamp = QDateTime::fromString(qstring(dd->stamp()), "yyyy-MM-ddTHH:mm:ssZ")
                                  .toSecsSinceEpoch();
    }

    auto addresses = msg.address();
    if (addresses) {
        for (auto& item : addresses->addresses()) {
            // 群消息来源辨别多终端
            if (item.type == "ofrom") {
                imMsg.from = item.jid.full();
            }
        }
    }

    for (auto handler : groupHandlers) {
        handler->onGroupMessage(roomId, peerId, imMsg);
    }
}

bool IM::handleMUCRoomCreation(gloox::MUCRoom* room) {
    qDebug() << "handleMUCRoomCreation" << room->jid().full().c_str();

    room->requestRoomConfig();

    // 添加到缓存
    auto roomId = (room->jid().bare());
    m_roomMap.insert(std::make_pair(roomId, new IMRoomInfo(room, {}, {})));

    // 群聊增加
    for (auto handler : groupHandlers) {
        handler->onGroup(roomId, (room->name()));
    }

    joinRoom(roomId);

    //  bookmarkStorage->requestBookmarks();
    return true;
}

void IM::handleMUCSubject(gloox::MUCRoom* room,     //
                          const std::string& nick,  //
                          const std::string& subject) {
    qDebug() << __func__ << room->name().c_str() << "subject" << subject.c_str();
    for (auto handler : groupHandlers) {
        handler->onGroupSubjectChanged((room->jid().bare()), subject);
    }

    // 可能存在其它更新，执行信息拉取
    room->getRoomInfo();
}

void IM::handleMUCInviteDecline(gloox::MUCRoom* room, const gloox::JID& invitee,
                                const std::string& reason) {}

void IM::handleMUCError(gloox::MUCRoom* room, gloox::StanzaError error) {
    qWarning() << __func__ << "MUCRoom:" << qstring(room->name()) << error;
}

void IM::handleMUCInfo(gloox::MUCRoom* room,               //
                       int features,                       //
                       const std::string& name,            //
                       const gloox::DataForm* infoForm) {  //

    auto roomId = room->jid().full();
    auto roomName = name;

    qDebug() << __func__ << roomId.c_str() << roomName.c_str();

    IMGroup groupInfo;
    groupInfo.name = roomName;

    if (infoForm) {
        auto info = const_cast<IMRoomInfo*>(findRoom(roomId));

        for (auto field : infoForm->fields()) {
            qDebug() << "field name:" << field->name().c_str() << field->value().c_str();

            if (field->name() == "muc#roominfo_occupants") {
                groupInfo.occupants = std::stoi(field->value());
            } else if (field->name() == "muc#roominfo_roomname" && !field->value().empty()) {
                groupInfo.name = (field->value());
            } else if (field->name() == "muc#roominfo_subject") {
                groupInfo.subject = (field->value());
            } else if (field->name() == "muc#roominfo_description") {
                groupInfo.description = (field->value());
            } else if (field->name() == "muc#roominfo_creationdate") {
                groupInfo.creationdate = (field->value());
            }
        }
        info->info = groupInfo;

        for (auto handler : groupHandlers) {
            handler->onGroupInfo(roomId, groupInfo);
        }
    }
}

void IM::handleMUCItems(gloox::MUCRoom* room, const gloox::Disco::ItemList& items) {}

// config
void IM::handleMUCConfigList(gloox::MUCRoom* room,                 //
                             const gloox::MUCListItemList& items,  //
                             gloox::MUCOperation operation) {
    qDebug() << __func__;
}

void IM::handleMUCConfigForm(gloox::MUCRoom* room, const gloox::DataForm& form) {
    qDebug() << __func__ << "room:" << room->jid().full().c_str();

    auto roomId = (room->jid().bare());

    auto find = m_roomMap.find(roomId);
    if (find == m_roomMap.end()) {
        qWarning() << "Unable to find room from cache:" << roomId.c_str();
        return;
    }

    /**
     * 将缓存的房间配置放入`DataForm`
     */
    int changed = 0;
    auto& roomInfo = find->second;
    for (auto item : form.fields()) {
        for (const auto& change : roomInfo->changes) {
            if (item->name() == change.first) {
                item->setValue(change.second);
                changed += 1;
                break;
            }
        }
    }

    if (changed) {
        // 清空修改信息
        roomInfo->changes.clear();
        auto* mform = new gloox::DataForm(form);
        mform->setType(gloox::FormType::TypeSubmit);
        room->setRoomConfig(mform);
        qDebug() << "Update room config items" << changed;
    }
};

void IM::handleMUCConfigResult(gloox::MUCRoom* room, bool success, gloox::MUCOperation operation) {
    qDebug() << __func__ << "room" << room->jid().full().c_str() << "operation:" << operation
             << "success:" << success;
};

void IM::handleMUCRequest(gloox::MUCRoom* room, const gloox::DataForm& form) {
    qDebug() << __func__ << "room" << room->jid().full().c_str();
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
void IM::createRoom(const gloox::JID& jid, const std::string& password) {
    qDebug() << __func__ << "jid:" << jid.full().c_str();

    auto room = new gloox::MUCRoom(_client.get(), jid, this, this);
    if (!password.empty()) {
        room->setPassword(password);
    }

    room->instantRoom(gloox::MUCOperation::CreateInstantRoom);
    cacheJoinRoom(jid.bare(), jid.resource());

    setRoomName((jid.bare()), jid.resource());

    gloox::ConferenceListItem item;
    item.name = jid.resource();
    item.jid = room->jid().full();
    item.autojoin = true;
    item.nick = (getNickname());

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
bool IM::inviteToRoom(const gloox::JID& roomJid, const gloox::JID& peerId) {
    auto roomInfo = findRoom(roomJid.bare());
    if (!roomInfo) {
        return false;
    }

    roomInfo->room->invite(peerId, "");
    return true;
}

const IMRoomInfo* IM::findRoom(const std::string& groupId) const {
    auto it = m_roomMap.find(groupId);
    if (it == m_roomMap.end()) {
        qWarning() << "Unable to find room!";
        return {};
    }
    return it->second;
}

void IM::joinRoom(gloox::MUCRoom* room) {
    room->setNick((getNickname()));
    room->join();
    room->getRoomInfo();
    _client->disco()->getDiscoItems(room->jid(), gloox::XMLNS_BOOKMARKS, this, DISCO_CTX_BOOKMARKS);
}

void IM::cacheJoinRoom(const std::string& jid, const std::string& name) {
    qDebug() << __func__ << jid.c_str() << name.c_str();

    gloox::JID roomJid(jid);
    auto roomId = (roomJid.bare());
    auto room = new gloox::MUCRoom(_client.get(), roomJid, this, this);
    m_roomMap.insert(std::make_pair(roomId, new IMRoomInfo(room, {})));

    for (auto handler : groupHandlers) {
        handler->onGroup(roomId, name.empty() ? (roomJid.username()) : (name));
    }

    // 查询成员列表
    // XMLNS_DISCO_ITEMS
    getClient()->disco()->getDiscoItems(jid, gloox::XMLNS_DISCO_ITEMS, this,
                                        DISCO_CTX_CONF_MEMBERS);

    joinRoom(room);
}

void IM::joinRoom(const std::string& roomJid) {
    qDebug() << __func__ << roomJid.c_str();

    auto it = m_roomMap.find(roomJid);
    if (it == m_roomMap.end()) {
        qWarning() << "Unable find room from cache" << roomJid.c_str();
        return;
    }

    joinRoom(it->second->room);
}

bool IM::sendToRoom(const std::string& to, const std::string& msg, const std::string& id) {
    qDebug() << __func__ << "content:" << msg.c_str() << " => " << to.c_str();

    if (msg.empty()) {
        qWarning() << "empty message!";
        return false;
    }

    if (id.empty()) {
        qWarning() << "id is empty!";
        return false;
    }

    auto pRoomInfo = findRoom((to));
    if (!pRoomInfo) {
        qWarning() << "The room is not exist!";
        return false;
    }

    auto msgId = (id);
    sendIds.insert(msgId);
    return pRoomInfo->room->send(msg, msgId);
}

void IM::setRoomSubject(const std::string& groupId, const std::string& subject) {
    auto room = findRoom(groupId);
    if (room) {
        room->room->setSubject(subject);
    }
}

void IM::setRoomName(const std::string& groupId, const std::string& roomName) {
    qDebug() << __func__ << groupId.c_str() << roomName.c_str();
    const IMRoomInfo* pRoomInfo = findRoom(groupId);
    if (!pRoomInfo) {
        qDebug() << "room is not exist." << groupId.c_str();
        return;
    }

    auto info = const_cast<IMRoomInfo*>(pRoomInfo);
    info->changes.insert(std::make_pair("muc#roomconfig_roomname", roomName));
    // 获取新配置（handleMUCConfigForm处理）
    info->room->requestRoomConfig();
}

void IM::setRoomAlias(const std::string& groupId, const std::string& alias) {
    qDebug() << __func__ << groupId.c_str() << alias.c_str();
    // 修改书签列表
    bool update = false;
    for (auto& item : mConferenceList) {
        if (item.jid == groupId) {
            item.name = alias;
            update = true;
            break;
        }
    }
    // 存储书签列表
    if (update) {
        qDebug() << "Store the bookmarks:" << groupId.c_str();
        bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);
    }
}

void IM::setRoomDesc(const std::string& groupId, const std::string& desc) {
    qDebug() << __func__ << groupId.c_str() << desc.c_str();
    const IMRoomInfo* pRoomInfo = findRoom(groupId);
    if (!pRoomInfo) {
        return;
    }

    auto info = const_cast<IMRoomInfo*>(pRoomInfo);
    info->changes.insert(std::make_pair("muc#roomconfig_roomdesc", desc));
    // 获取新配置（handleMUCConfigForm处理）
    info->room->requestRoomConfig();
}

#ifdef WANT_PING
void IM::handlePing(const gloox::PingHandler::PingType type, const std::string& body) {
    qDebug() << __func__ << (int)type << qstring(body);
    if (type != websocketPong) {
        gloox::IQ iq(gloox::IQ::IqType::Result, gloox::JID(_host));
        _client->send(iq);
    }
}
#endif

/**
 * 处理个人信息（VCard）
 * @param jid
 * @param vcard
 */
void IM::handleVCard(const gloox::JID& jid, const gloox::VCard* vcard) {
    qDebug() << __func__ << "jid:" << qstring(jid.full());

    IMVCard imvCard = {
            .fullName = vcard->formattedname(),  // vCard:FN
            .nickname = vcard->nickname(),
            .title = vcard->title(),
    };

    for (const auto& item : vcard->addresses()) {
        if (item.work) {
            IMVCard::Adr adr = {.street = (item.street),
                                .locality = (item.locality),
                                .region = (item.region),
                                .country = (item.ctry)};
            imvCard.adrs.push_back(adr);
            break;
        }
    }

    auto& emails = vcard->emailAddresses();
    for (auto& item : emails) {
        if (!item.userid.empty()) {
            IMVCard::Email email = {.type = 0, .number = (item.userid)};
            email.type = 1;
            imvCard.emails.push_back(email);
            break;
        };
    }

    for (auto& item : vcard->telephone()) {
        if (!item.number.empty() && item.work) {
            IMVCard::Tel tel = {.type = 0, .mobile = item.cell, .number = (item.number)};
            imvCard.tels.push_back(tel);
        };
    }

    auto& photo = vcard->photo();
    if (!photo.binval.empty() || !photo.extval.empty()) {
        imvCard.photo = {.type = (photo.type),  //
                         .bin = photo.binval,   //
                         .url = (photo.extval)};
    }

    if (jid.bare() == getClient()->jid().bare()) {
        for (auto handler : selfHandlers) {
            handler->onSelfVCardChanged(imvCard);
        }
    } else {
        for (auto handler : friendHandlers) {
            handler->onFriendVCard(IMPeerId(jid), imvCard);
        }
    }
}

void IM::handleVCardResult(VCardContext context, const gloox::JID& jid, gloox::StanzaError error) {
    qDebug() << __func__;
    if (error) {
        return;
    }
}

void IM::fetchFriendVCard(const std::string& friendId) {
    // 获取联系人个人信息
    qDebug() << __func__ << friendId.c_str();
    gloox::JID jid(friendId);
    vCardManager->fetchVCard(jid, this);
    //  pubSubManager->subscribe(jid, "", this);
    //  _client->rosterManager()->subscribe(jid.bareJID());
}

void IM::requestVCards() {
    qDebug() << __func__;
    vCardManager->fetchVCard(self().bareJID(), this);
}

void IM::handleTag(gloox::Tag* tag) {
    qDebug() << __func__ << tag->name().c_str();
}

bool IM::handleIq(const gloox::IQ& iq) {
    qDebug() << __func__ << iq.id().c_str();
    const auto* ext = iq.findExtension<gloox::ExtDisco>(gloox::ExtSrvDisco);
    if (ext) {
        std::vector<gloox::ExtDisco::Service> ss;
        for (const auto& item : ext->services()) {
            ss.push_back(item);
        }
        //
        mExtSrvDiscos = ranges::views::all(ss) | ranges::views::transform([&](const auto& item) {
                            ortc::IceServer ice;
                            ice.uri = item.type + ":" + item.host + ":" + std::to_string(item.port);
                            //        +"?transport=" + item.transport;
                            ice.username = item.username;
                            ice.password = item.password;
                            return ice;
                        }) |
                        ranges::to_vector;
    }
    //    emit incoming(qstring(iq.tag()->xml()));
    return true;
}

void IM::handleIqID(const gloox::IQ& iq, int context) {}

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
void IM::handleBookmarks(const gloox::BookmarkList& bList,    //
                         const gloox::ConferenceList& cList)  //
{
    qDebug() << __func__;
    qDebug() << "ConferenceList:" << cList.size();
    qDebug() << "BookmarkList:" << bList.size();

    //  缓存群聊书签列表（新增加群聊加入该书签一起保存）
    mConferenceList = cList;

    for (auto& c : cList) {
        auto name = (!c.name.empty() ? c.name : gloox::JID(c.jid).username());
        // 缓存到本地
        cacheJoinRoom(c.jid, name);
    }

    mBookmarkList = bList;
    for (auto& c : mBookmarkList) {
        qDebug() << "Bookmark name:" << c.name.c_str() << "url:" << c.url.c_str();
    }
}

void IM::handleBookmarks(const gloox::BMConferenceList& cList) {
    qDebug() << __func__;
    for (const auto& conf : cList) {
        qDebug() << "conference:" << conf.jid.c_str();
    }
}

// Disco handler
void IM::handleDiscoInfo(const gloox::JID& from,          //
                         const gloox::Disco::Info& info,  //
                         int context) {
    std::string _from = from.full();
    qDebug() << __func__ << _from.c_str() << "context:" << context;

    for (auto& feature : info.features()) {
        qDebug() << "feature:" << feature.c_str();
    }

    const gloox::Disco::IdentityList& identities = info.identities();
    for (auto identity : identities) {
        qDebug() << "identity:" << identity->name().c_str();
    }
}

/**
 * 获取服务发现
 * @param from
 * @param items
 * @param context
 */
void IM::handleDiscoItems(const gloox::JID& from,            //
                          const gloox::Disco::Items& items,  //
                          int context) {
    std::string _from = (from.full());
    qDebug() << __func__ << "from" << _from.c_str() << "context" << context;

    const gloox::Disco::ItemList& localItems = items.items();
    for (auto item : localItems) {
        if (context == DISCO_CTX_ROSTER) {
            pubSubManager->subscribe(item->jid(), gloox::XMLNS_NICKNAME, this);
#ifndef Q_OS_WIN
            pubSubManager->subscribe(item->jid(), gloox::XMLNS_AVATAR, this);
#endif  // !Q_OS_WIN
        }

        else if (context == DISCO_CTX_CONF) {
            /**
             * 处理服务发现的群聊列表
             */
            qDebug() << "room:" << item->name().c_str() << item->jid().bare().c_str();
        }

        else if (context == DISCO_CTX_CONF_MEMBERS) {
            // 群聊成员
            qDebug() << "member:" << item->jid().bare().c_str();
        }
    }
}

void IM::handleDiscoError(const gloox::JID& from,     //
                          const gloox::Error* error,  //
                          int context) {
    std::string _from = from.full();
    qDebug() << __func__ << "from:" << _from.c_str();
}

// Presence Handler
void IM::handlePresence(const gloox::Presence& presence) {
    auto& from = presence.from();
    qDebug() << __func__ << "from" << from.full().c_str() << "presence" << presence.presence();

    auto iterator = fromHostHandlers.find(from.server());
    if (iterator != fromHostHandlers.end()) {
        iterator->second->handleHostPresence(from, presence);
    }
    updateOnlineStatus(from.bare(), from.resource(), presence.presence());
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
    qDebug() << __func__ << jid.full().c_str();

    auto m = _client->rosterManager();
    // 订阅对方
    m->subscribe(jid);

    auto item = m->getRosterItem(jid);
    if (!item) {
        qWarning() << "Unable to find roster.";
        return;
    }

    for (auto handler : friendHandlers) {
        handler->onFriend(IMFriend{item});
    }
}

/**
 * 好友被删除事件
 * @param jid
 */
void IM::handleItemRemoved(const gloox::JID& jid) {
    qDebug() << __func__ << jid.full().c_str();
    for (auto handler : friendHandlers) {
        handler->onFriendRemoved((jid.bare()));
    }
}

// 好友更新
void IM::handleItemUpdated(const gloox::JID& jid) {
    qDebug() << __func__ << jid.full().c_str();

    auto contactId = jid.bare();

    auto item = _client->rosterManager()->getRosterItem(jid);
    auto subType = item->subscription();
    qDebug() << __func__ << "subscription:" << (subType);
    auto data = item->data();
    qDebug() << "ask" << data->ask().c_str() << "sub" << data->sub().c_str();

    for (auto handler : friendHandlers) {
        handler->onFriendAliasChanged(IMContactId(jid.bareJID()), (data->name()));
    }
}

/**
 * 订阅好友
 * @param jid
 */
void IM::handleItemSubscribed(const gloox::JID& jid) {
    qDebug() << __func__ << qstring(jid.full());
}

/**
 * 取消订阅好友
 * @param jid
 */
void IM::handleItemUnsubscribed(const gloox::JID& jid) {
    qDebug() << __func__ << qstring(jid.full());
}

bool IM::removeFriend(const gloox::JID& jid) {
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

void IM::addFriend(const gloox::JID& jid, const std::string& nick, const std::string& msg) {
    qDebug() << __func__ << jid.full().c_str() << nick.c_str() << msg.c_str();

    auto m = _client->rosterManager();
    auto f = m->getRosterItem(jid);
    if (f) {
        qWarning() << "IMFriend is existing!";
        return;
    }
    // 订阅对方(同时加入到联系人列表)
    m->subscribe(jid, nick, {});
    m->add(jid, nick, {});
    m->synchronize();
}

void IM::acceptFriendRequest(const std::string& friendId) {
    qDebug() << __func__ << friendId.c_str();
    auto jid = gloox::JID(friendId).bareJID();

    auto m = _client->rosterManager();
    // 答复同意订阅
    m->ackSubscriptionRequest(jid, true);
    // 同时订阅对方
    m->subscribe(jid);
    // 添加到联系人列表
    m->add(jid, {}, {});
    m->synchronize();
}

void IM::rejectFriendRequest(const std::string& friendId) {
    qDebug() << __func__ << friendId.c_str();
    _client->rosterManager()->ackSubscriptionRequest(gloox::JID(friendId).bareJID(), false);
}

size_t IM::getRosterCount() {
    return _client->rosterManager()->roster()->size();
}

void IM::getRosterList(std::list<IMFriend>& list) {
    auto rosterManager = _client->rosterManager();
    if (!rosterManager) {
        std::lock_guard<std::mutex> locker(mutex);
        rosterManager = enableRosterManager();
    }

    gloox::Roster* rosterMap = rosterManager->roster();
    for (const auto& itr : *rosterMap) {
        auto pItem = itr.second;
        list.push_back(IMFriend{pItem});
    }
}

void IM::setFriendAlias(const gloox::JID& jid, const std::string& alias) {
    qDebug() << __func__ << jid.bare().c_str() << alias.c_str();

    auto m = _client->rosterManager();

    // 保存联系人
    m->add(jid.bareJID(), alias, {});
    m->synchronize();
}

void IM::handleRoster(const gloox::Roster& roster) {
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
        for (auto handler : friendHandlers) {
            handler->onFriend(frnd);
        }

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
void IM::handleRosterPresence(const gloox::RosterItem& item,               //
                              const std::string& resource,                 //
                              gloox::Presence::PresenceType presenceType,  //
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
                    //          qDebug()<<std::string("caps:%1").arg(qstring(caps->node())))
                    //          break;
                    //        }
                case gloox::ExtVCardUpdate: {
                    // VCard个人信息更新
                    /**
                     * <pubsub xmlns='http://jabber.org/protocol/pubsub'>
                    <items node='urn:xmpp:avatar:data'>
                      <item id='111f4b3c50d7b0df729d299bc6f8e9ef9066971f'/>
                    </items>
                    </pubsub>
                     */
                    auto vCardUpdate = const_cast<gloox::VCardUpdate*>(
                            static_cast<const gloox::VCardUpdate*>(ext));
                    if (vCardUpdate && vCardUpdate->hasPhoto()) {
                        gloox::PubSub::ItemList items;
                        auto item0 = new gloox::PubSub::Item();
                        item0->setID(vCardUpdate->hash());
                        items.emplace_back(item0);
                        pubSubManager->requestItems(item.jid(), gloox::XMLNS_AVATAR, "", items,
                                                    this);
                    }
                    break;
                }
            }
        }
    }
    //  }
}

void IM::handleSelfPresence(const gloox::RosterItem& item,               //
                            const std::string& resource,                 //
                            gloox::Presence::PresenceType presenceType,  //
                            const std::string& msg) {
    qDebug() << QString("item:%1 resource:%2 presenceType:%3 msg:%4")  //
                        .arg(qstring(item.jid().full()))               //
                        .arg(qstring(resource))                        //
                        .arg(presenceType)                             //
                        .arg(qstring(msg));

    auto st = _client->resource() == resource ? presenceType : (int)gloox::Presence::Available;

    for (auto handler : selfHandlers) {
        handler->onSelfStatusChanged(static_cast<IMStatus>(st), msg);
    }

    //  if (presenceType == gloox::Presence::Available) {
    for (auto& it : item.resources()) {
        auto sk = it.first;
        auto sr = it.second;
        for (auto& ext : sr->extensions()) {
            switch (ext->extensionType()) {
                case gloox::ExtVCardUpdate: {
                    // VCard个人信息更新
                    /**
                     * <pubsub xmlns='http://jabber.org/protocol/pubsub'>
                    <items node='urn:xmpp:avatar:data'>
                      <item id='111f4b3c50d7b0df729d299bc6f8e9ef9066971f'/>
                    </items>
                    </pubsub>
                     */
                    auto vCardUpdate = const_cast<gloox::VCardUpdate*>(
                            static_cast<const gloox::VCardUpdate*>(ext));
                    if (vCardUpdate && vCardUpdate->hasPhoto()) {
                        gloox::PubSub::ItemList items;
                        auto item0 = new gloox::PubSub::Item();
                        item0->setID(vCardUpdate->hash());
                        items.emplace_back(item0);

                        pubSubManager->requestItems(item.jid().bareJID(), gloox::XMLNS_AVATAR, "",
                                                    items, this);
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
bool IM::handleSubscriptionRequest(const gloox::JID& jid, const std::string& msg) {
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
bool IM::handleUnsubscriptionRequest(const gloox::JID& jid, const std::string& msg) {
    qDebug() << __func__;
    return true;
};

void IM::handleNonrosterPresence(const gloox::Presence& presence) {
    qDebug() << __func__;
};

void IM::handleRosterError(const gloox::IQ& iq) {
    qDebug() << __func__;
};

void IM::handleRosterItemExchange(const gloox::JID& from, const gloox::RosterX* items) {
    qDebug() << __func__;
}

void IM::handleRegistrationFields(const gloox::JID& from, int fields, std::string instructions) {
    qDebug() << __func__;
};

void IM::handleAlreadyRegistered(const gloox::JID& from) {
    qDebug() << __func__;
};

void IM::handleRegistrationResult(const gloox::JID& from, gloox::RegistrationResult regResult,
                                  const gloox::Error* error) {
    qDebug() << __func__;
};

void IM::handleDataForm(const gloox::JID& from, const gloox::DataForm& form) {
    qDebug() << __func__;
};

void IM::handleOOB(const gloox::JID& from, const gloox::OOB& oob) {
    qDebug() << __func__;
};

IMContactId IM::getSelfId() {
    return IMContactId(loginJid.bare());
}

IMPeerId IM::getSelfPeerId() {
    std::lock_guard<std::mutex> locker(mutex);
    return IMPeerId(_client->jid().full());
}

std::string IM::getSelfUsername() {
    std::lock_guard<std::mutex> locker(mutex);
    return self().username();
}

void IM::setNickname(const std::string& nickname) {
    qDebug() << __func__ << nickname.c_str();

    if (_nick == nickname) {
        return;
    }
    _nick = nickname;

    gloox::Nickname nick(nickname);

    gloox::PubSub::ItemList items;
    auto item = new gloox::PubSub::Item();
    item->setID("current");
    item->setPayload(nick.tag());
    items.emplace_back(item);
    pubSubManager->publishItem(gloox::JID(), gloox::XMLNS_NICKNAME, items, nullptr, this);
    qDebug() << __func__ << "completed.";
}

std::string IM::getNickname() {
    if (!_nick.empty()) {
        return _nick;
    }
    return getSelfUsername();
}

void IM::setAvatar(const std::string& avatar) {
    qDebug() << __func__;
    if (avatar.empty()) {
        qWarning() << "Empty avatar!";
        return;
    }

    qDebug() << __func__ << "size:" << avatar.size();

    const QByteArray& byteArray = QByteArray::fromStdString(avatar);
    auto sha1 = ok::base::Hashs::sha1String(byteArray);
    qDebug() << __func__ << "sha1:" << sha1;

    auto base64 = byteArray.toBase64().toStdString();

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

    gloox::AvatarData avt(payload);

    gloox::PubSub::ItemList items;
    auto item = new gloox::PubSub::Item();
    item->setID(sha1.toStdString());
    item->setPayload(avt.tag());
    items.emplace_back(item);

    pubSubManager->publishItem(gloox::JID(), gloox::XMLNS_AVATAR, items, nullptr, this);

    qDebug() << __func__ << "completed.";
}

void IM::changePassword(const std::string& password) {
    qDebug() << __func__ << password.c_str();
    if (password.empty()) return;

    // changing password
    mRegistration->changePassword(_client->username(), password);
}

void IM::loadGroupList() {
    auto disco = _client->disco();
    disco->getDiscoItems(gloox::JID("conference." + _host), "", this, DISCO_CTX_CONF);
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
        _client->disco()->getDiscoItems(it->first, "", this, DISCO_CTX_ROSTER);
    }
}

void IM::sendPresence() {
    gloox::Presence pres(gloox::Presence::PresenceType::Available, gloox::JID());
    pres.addExtension(new gloox::Capabilities);
    _client->setPresence();
}

void IM::sendPresence(const gloox::JID& to, gloox::Presence::PresenceType type) {
    _client->setPresence(to, type, 0);
}

void IM::sendReceiptReceived(const std::string& id, std::string receiptNum) {
    // <received xmlns='urn:xmpp:receipts' id='richard2-4.1.247'/>

    gloox::Message m(gloox::Message::MessageType::Chat, gloox::JID(id));
    m.setFrom(_client->jid());

    m.addExtension(new gloox::Receipt(gloox::Receipt::ReceiptType::Received, receiptNum));

    _client->send(m);
}

void IM::sendServiceDiscoveryItems() {
    auto tag = new gloox::Tag("query");
    tag->setXmlns(gloox::XMLNS_DISCO_ITEMS);

    auto* iq = new gloox::IQ(gloox::IQ::Get, self().server(), _client->getID());
    iq->setFrom(self());

    auto iqt = iq->tag();
    iqt->addChild(tag);
    _client->send(iqt);
}

void IM::sendServiceDiscoveryInfo(const gloox::JID& item) {
    auto tag = new gloox::Tag("query");
    tag->setXmlns(gloox::XMLNS_DISCO_INFO);

    auto* iq = new gloox::IQ(gloox::IQ::Get, item, _client->getID());
    iq->setFrom(self());

    auto iqt = iq->tag();
    iqt->addChild(tag);
    _client->send(iqt);
}

void IM::handleItem(const gloox::JID& service, const std::string& node, const gloox::Tag* entry) {}

void IM::handleItems(const std::string& id,                    //
                     const gloox::JID& service,                //
                     const std::string& node,                  //
                     const gloox::PubSub::ItemList& itemList,  //
                     const gloox::Error* error) {
    qDebug() << __func__ << qstring(service.full()) << (qstring(node));

    if (error) {
        qWarning() << "error:" << error->tag()->xml().c_str();
        return;
    }

    auto friendId = (service.bare());
    auto isSelf = friendId == getSelfId().toString();

    for (auto& item : itemList) {
        auto data = item->payload();
        auto tagName = data->name();

        qDebug() << "handleItem:" << qstring(tagName) << qstring(node);

        if (node == gloox::XMLNS_NICKNAME) {
            gloox::Nickname nickname(data);
            auto nick = (nickname.nick());
            if (isSelf) {
                for (auto handler : selfHandlers) {
                    handler->onSelfNameChanged(nick);
                }
            } else {
                for (auto handler : friendHandlers) {
                    handler->onFriendNickChanged(friendId, nick);
                }
            }
        }

        if (node == gloox::XMLNS_AVATAR) {
            auto base64 = data->cdata();
            if (!base64.empty()) {
                std::string::size_type pos = 0;
                while ((pos = base64.find('\n')) != std::string::npos) base64.erase(pos, 1);
                while ((pos = base64.find('\r')) != std::string::npos) base64.erase(pos, 1);
                auto avt = gloox::Base64::decode64(base64);
                if (isSelf) {
                    for (auto h : selfHandlers) {
                        h->onSelfAvatarChanged(avt);
                    }
                } else {
                    // emit receiveFriendAvatarChanged(friendId, avt);
                    for (auto handler : friendHandlers) {
                        handler->onFriendAvatarChanged(friendId, avt);
                    }
                }
            }
        }
    }
}

void IM::handleItemPublication(const std::string& id,                    //
                               const gloox::JID& service,                //
                               const std::string& node,                  //
                               const gloox::PubSub::ItemList& itemList,  //
                               const gloox::Error* error) {
    qDebug() << __func__ << "node:" << qstring(node);
    if (node == gloox::XMLNS_AVATAR) {
        // 更新头像元信息
        //  https://xmpp.org/extensions/xep-0084.html#process-pubmeta
        for (auto& item : itemList) {
            qDebug() << QString("itemId:%1").arg(qstring(item->id()));
        }
    }
}

void IM::handleItemDeletion(const std::string& id, const gloox::JID& service,
                            const std::string& node, const gloox::PubSub::ItemList& itemList,
                            const gloox::Error* error) {}

void IM::handleSubscriptionResult(const std::string& id, const gloox::JID& service,
                                  const std::string& node, const std::string& sid,
                                  const gloox::JID& jid,
                                  const gloox::PubSub::SubscriptionType subType,
                                  const gloox::Error* error) {
    qDebug() << __func__ << "id" << qstring(id) << "service:" << qstring(service.full())
             << "node:" << qstring(node) << "jid:" << qstring(jid.full());
    pubSubManager->requestItems(service, node, sid, 100, this);
}

void IM::handleUnsubscriptionResult(const std::string& id, const gloox::JID& service,
                                    const gloox::Error* error) {}
void IM::handleSubscriptionOptions(const std::string& id, const gloox::JID& service,
                                   const gloox::JID& jid, const std::string& node,
                                   const gloox::DataForm* options, const std::string& sid,
                                   const gloox::Error* error) {}
void IM::handleSubscriptionOptionsResult(const std::string& id, const gloox::JID& service,
                                         const gloox::JID& jid, const std::string& node,
                                         const std::string& sid, const gloox::Error* error) {}
void IM::handleSubscribers(const std::string& id, const gloox::JID& service,
                           const std::string& node, const gloox::PubSub::SubscriptionList& list,
                           const gloox::Error* error) {}
void IM::handleSubscribersResult(const std::string& id, const gloox::JID& service,
                                 const std::string& node, const gloox::PubSub::SubscriberList* list,
                                 const gloox::Error* error) {}

void IM::handleAffiliates(const std::string& id, const gloox::JID& service, const std::string& node,
                          const gloox::PubSub::AffiliateList* list, const gloox::Error* error) {}

void IM::handleAffiliatesResult(const std::string& id, const gloox::JID& service,
                                const std::string& node, const gloox::PubSub::AffiliateList* list,
                                const gloox::Error* error) {}
void IM::handleNodeConfig(const std::string& id, const gloox::JID& service, const std::string& node,
                          const gloox::DataForm* config, const gloox::Error* error) {}
void IM::handleNodeConfigResult(const std::string& id, const gloox::JID& service,
                                const std::string& node, const gloox::Error* error) {}
void IM::handleNodeCreation(const std::string& id, const gloox::JID& service,
                            const std::string& node, const gloox::Error* error) {}
void IM::handleNodeDeletion(const std::string& id, const gloox::JID& service,
                            const std::string& node, const gloox::Error* error) {}
void IM::handleNodePurge(const std::string& id, const gloox::JID& service, const std::string& node,
                         const gloox::Error* error) {}
void IM::handleSubscriptions(const std::string& id, const gloox::JID& service,
                             const gloox::PubSub::SubscriptionMap& subMap,
                             const gloox::Error* error) {}
void IM::handleAffiliations(const std::string& id, const gloox::JID& service,
                            const gloox::PubSub::AffiliationMap& affMap,
                            const gloox::Error* error) {}

void IM::handleDefaultNodeConfig(const std::string& id, const gloox::JID& service,
                                 const gloox::DataForm* config, const gloox::Error* error) {}

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
                            gloox::Presence::PresenceType presenceType) {
    if (presenceType == gloox::Presence::Error) {
        qWarning() << "Ignore error presence.";
        return;
    }
    if (resource.empty()) {
        qWarning() << "Ignore resource is empty.";
        return;
    }

    auto friendId = bare;
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

    for (auto handler : friendHandlers) {
        handler->onFriendStatus(friendId, static_cast<IMStatus>(status));
    }
}

bool IM::leaveGroup(const std::string& groupId) {
    qDebug() << "leaveGroup" << groupId.c_str();
    auto r = findRoom(groupId);
    if (!r) {
        qWarning() << "Unable to find room" << groupId.c_str();
        return false;
    }
    r->room->leave();

    // 删除缓存
    m_roomMap.erase(groupId);

    // 从书签删除，再保存书签
    mConferenceList.remove_if([&](gloox::ConferenceListItem& a) { return a.jid == groupId; });
    bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);

    return true;
}

bool IM::destroyGroup(const std::string& groupId) {
    qDebug() << "destroyGroup" << groupId.c_str();

    auto r = findRoom(groupId);
    if (!r) {
        qWarning() << "Unable to find room" << groupId.c_str();
        return false;
    }
    // 删除
    r->room->destroy();

    // 删除缓存
    m_roomMap.erase(groupId);

    // 从书签删除，再保存书签
    mConferenceList.remove_if([&](gloox::ConferenceListItem& a) { return a.jid == (groupId); });
    bookmarkStorage->storeBookmarks(mBookmarkList, mConferenceList);

    return true;
}

gloox::Disco::ItemList IM::handleDiscoNodeItems(const gloox::JID& from, const gloox::JID& to,
                                                const std::string& node) {
    qDebug() << __func__ << QString("from:%1").arg(from.full().c_str());
    return gloox::Disco::ItemList();
}

gloox::Disco::IdentityList IM::handleDiscoNodeIdentities(const gloox::JID& from,
                                                         const std::string& node) {
    return gloox::Disco::IdentityList();
}

gloox::StringList IM::handleDiscoNodeFeatures(const gloox::JID& from, const std::string& node) {
    return gloox::StringList();
}

void IM::handleIncoming(gloox::Tag* tag) {
    //  auto services = tag->findChild("services", XMLNS, XMLNS_EXTERNAL_SERVICE_DISCOVERY);
    //  if (services) {
    //    mExtDisco = ExtDisco(services);
    //  }
    //  emit incoming(::base::Xmls::parse(qstring(tag->xml())));
}

bool IM::onTLSConnect(const gloox::CertInfo& info) {
    qDebug() << "CertInfo:";

    time_t from(info.date_from);
    time_t to(info.date_to);

    qDebug() << "status: " << info.status << "issuer: " << info.issuer.c_str()
             << "peer: " << info.server.c_str() << "protocol: " << info.protocol.c_str()
             << "mac: " << info.mac.c_str() << "cipher: " << info.cipher.c_str()
             << "compression: " << info.compression.c_str();

    qDebug() << QString("from:%1").arg(ctime(&from));
    qDebug() << QString("to:%1").arg(ctime(&to));

    return true;
}

void IM::handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string& message) {
    //   qDebug()<<std::string("%1").arg(message.c_str()));
    auto line = message;
    switch (area) {
        case gloox::LogAreaXmlIncoming:
            qDebug() << "Received XML:" << line.c_str();
            break;
        case gloox::LogAreaXmlOutgoing:
            qDebug() << "Sent XML:" << line.c_str();
            break;
        case gloox::LogAreaClassConnectionBOSH:
            qDebug() << "BOSH:" << line.c_str();
            break;
        case gloox::LogAreaClassClient:
            qDebug() << "Client:" << line.c_str();
            break;
        case gloox::LogAreaClassDns:
            qDebug() << "dns:" << line.c_str();
            break;
        default:
            qDebug() << "msg:" << line.c_str();
    }
}

IMStatus IM::getFriendStatus(const std::string& qString) {
    return onlineMap.find(qString) != onlineMap.end() ? IMStatus::Available : IMStatus::Unavailable;
}

void IM::requestFriendNickname(const gloox::JID& friendId) {
    qDebug() << __func__ << friendId.full().c_str();
    pubSubManager->subscribe(friendId, gloox::XMLNS_NICKNAME, this);
}

/**
 * Jingle sessions
 */

void IM::handleSessionActionError(gloox::Jingle::Action action, gloox::Jingle::Session* session,
                                  const gloox::Error* error) {
    qDebug() << __func__ << "sid:" << qstring(session->sid())
             << "action:" << static_cast<int>(action)
             << "remote:" << qstring(session->remote().full())
             << "error:" << qstring(error->text());
}

void IM::handleIncomingSession(gloox::Jingle::Session* session) {
    auto sid = qstring(session->sid());
    qDebug() << __func__ << "sId" << sid;
}

// Session
void IM::handleSessionAction(gloox::Jingle::Action action,     //
                             gloox::Jingle::Session* session,  //
                             const gloox::Jingle::Session::Jingle* jingle) {
    auto from = session->remote();
    auto peerId = IMPeerId(from);
    auto sid = qstring(jingle->sid());

    qDebug() << __func__ << static_cast<int>(action) << qstring(from.full()) << "sid:" << sid;

    switch (action) {
        case gloox::Jingle::Action::SessionInitiate: {
            for (auto h : m_sessionHandlers) {
                if (h->doSessionInitiate(session, jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::SessionInfo: {
            for (auto h : m_sessionHandlers) {
                if (h->doSessionInfo(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::SessionTerminate: {
            for (auto h : m_sessionHandlers) {
                if (h->doSessionTerminate(session, jingle, peerId)) break;
            }
            removeSession(session);
            break;
        }
        case gloox::Jingle::Action::SessionAccept: {
            for (auto h : m_sessionHandlers) {
                if (h->doSessionAccept(session, jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::ContentAccept: {
            for (auto h : m_sessionHandlers) {
                if (h->doContentAccept(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::ContentAdd: {
            // content-add
            for (auto h : m_sessionHandlers) {
                if (h->doContentAdd(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::ContentRemove: {
            for (auto h : m_sessionHandlers) {
                if (h->doContentRemove(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::ContentModify: {
            for (auto h : m_sessionHandlers) {
                if (h->doContentModify(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::ContentReject: {
            for (auto h : m_sessionHandlers) {
                if (h->doContentReject(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::TransportAccept: {
            for (auto h : m_sessionHandlers) {
                if (h->doTransportAccept(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::TransportInfo: {
            for (auto h : m_sessionHandlers) {
                if (h->doTransportInfo(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::TransportReject: {
            for (auto h : m_sessionHandlers) {
                if (h->doTransportReject(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::TransportReplace: {
            for (auto h : m_sessionHandlers) {
                if (h->doTransportReplace(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::SecurityInfo: {
            for (auto h : m_sessionHandlers) {
                if (h->doSecurityInfo(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::DescriptionInfo: {
            for (auto h : m_sessionHandlers) {
                if (h->doDescriptionInfo(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::SourceAdd: {
            for (auto h : m_sessionHandlers) {
                if (h->doSourceAdd(jingle, peerId)) break;
            }
            break;
        }
        case gloox::Jingle::Action::InvalidAction:
            for (auto h : m_sessionHandlers) {
                if (h->doInvalidAction(jingle, peerId)) break;
            }
            break;
    }
}

gloox::Jingle::Session* IM::createSession(const gloox::JID& jid, const std::string& sId,
                                          IMSessionHandler* h) {
    if (!_sessionManager) {
        return nullptr;
    }
    if (!h) {
        return nullptr;
    }
    return _sessionManager->createSession(jid, this, (sId));
}

void IM::removeSession(gloox::Jingle::Session* s) {
    if (!_sessionManager) return;
    if (!s) return;
    _sessionManager->discardSession(s);
}

void IM::addSessionHandler(IMSessionHandler* h) {
    qDebug() << __func__ << "handler:" << h;
    assert(h);

    m_sessionHandlers.push_back(h);
}

void IM::removeSessionHandler(IMSessionHandler* h) {
    qDebug() << __func__ << "handler:" << h;
    assert(h);
    m_sessionHandlers.erase(std::find_if(m_sessionHandlers.begin(),
                                         m_sessionHandlers.end(),
                                         [h](IMSessionHandler* e) { return e == h; }),
                            m_sessionHandlers.end());
}

void IM::removeSelfHandler(SelfHandler* h) {
    qDebug() << __func__ << "handler:" << h;
    assert(h);

    selfHandlers.erase(std::find_if(selfHandlers.begin(),
                                    selfHandlers.end(),
                                    [h](SelfHandler* e) { return e == h; }),
                       selfHandlers.end());
}

void IM::addFromHostHandler(const std::string& from, IMFromHostHandler* h) {
    qDebug() << __func__ << "from: " << qstring(from) << " handler:" << h;
    assert(h);
    fromHostHandlers[from] = h;
}

void IM::clearFromHostHandler() {
    fromHostHandlers.clear();
}

void IM::addFriendHandler(FriendHandler* h) {
    friendHandlers.push_back(h);
}

void IM::addSelfHandler(SelfHandler* h) {
    selfHandlers.push_back(h);
}

void IM::addGroupHandler(GroupHandler* h) {
    groupHandlers.push_back(h);
}

void IM::addIMHandler(IMHandler* h) {
    imHandlers.push_back(h);
}

}  // namespace lib::messenger
