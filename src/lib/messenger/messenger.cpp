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
#include "IMFile.h"
#include <range/v3/all.hpp>
#include "application.h"
#include "base/xmls.h"
#include "IMMeet.h"
#include "lib/messenger/IM.h"
#include "lib/messenger/IMCall.h"
#include "lib/plugin/pluginmanager.h"

namespace lib::messenger {

Messenger::Messenger(const QString& host,
                     const QString& name,
                     const QString& password,
                     QObject* parent)
        : QObject(parent), _im{nullptr} {
    qDebug() << __func__;

    auto _session = ok::Application::Instance()->getSession();

    QStringList features;
#ifdef OK_PLUGIN
    auto pm = ok::plugin::PluginManager::instance();
    features << pm->pluginFeatures();
    int acc = pm->addAccount(_session->account(), this);
    qDebug() << "PluginManager account id=>" << acc;
#endif

    _im = new ::lib::messenger::IM(host, name, password, features);
    qDebug() << "Create im =>" << _im;
    connectIM();
}

Messenger::~Messenger() {
    qDebug() << __func__;
}

void Messenger::start() {
    qDebug() << __func__;
    _im->start();
    //connect(_im, &IM::started, [&]() { emit started(); });
}

bool Messenger::isStarted() const
{
    return _im->isStarted();
}

void Messenger::sendChatState(const QString& friendId, int state) {
    _im->sendChatState(friendId, static_cast<gloox::ChatStateType>(state));
}

// void Messenger::onStarted() {
//     qDebug() << __func__;
// #ifdef OK_PLUGIN
//     qDebug() << "Initialize plugin manager...";
//     qRegisterMetaType<QDomDocument>("QDomDocument");
//     connect(_im, &IM::exportEncryptedMessage, this, &Messenger::onEncryptedMessage);
//     qDebug() << "Initialized plugin manager successfully";
// #endif
//     qDebug() << "connected completed";
// }


bool Messenger::connectIM() {
    // connect(_im, &IM::incoming, this, [=, this](QString xml) { emit incoming(xml); });
    return true;
}

bool Messenger::initRoom() {
    _im->loadGroupList();
    return true;
}


QString Messenger::genUniqueId() {
    return qstring(_im->getClient()->getID());
}

bool Messenger::sendToGroup(const QString& g, const QString& msg, const QString& id) {
    qDebug() << QString("sendToGroup=>%1 id:%2 msg:%2").arg(g).arg(id).arg(msg);
    sentCount++;
    return _im->sendToRoom(g, msg, id);
}

bool Messenger::sendToFriend(const QString& f,
                             const QString& msg,
                             const QString& id,
                             bool encrypt) {
    qDebug() << __func__ << msg << "=>" << f;
    sentCount++;
    bool y = false;
    if (encrypt) {
#ifdef OK_PLUGIN
        auto _session = ok::Application::Instance()->getSession();

        ok::base::Jid ownJid(qstring(_im->self().full()));
        _session->account()->setJid(ownJid);

        auto pm = ok::plugin::PluginManager::instance();
        pm->addAccount(_session->account(), this);

        auto dom = _im->buildMessage(f, msg, id);
        auto ele = dom.documentElement();

        if (pm->encryptMessageElement(_session->account(), ele)) {
            qDebug() << "encryptMessageElement=>" << ele.ownerDocument().toString();
            auto xml = ok::base::Xmls::format(ele);
            _im->send(xml);
            y = true;
        }
#endif
    }
    if (!y) {
        y = _im->sendTo(f, msg, id);
    }
    return y;
}

void Messenger::receiptReceived(const QString& f, QString receipt) {
    return _im->sendReceiptReceived(f, receipt);
}

void Messenger::sendFriendRequest(const QString& f, const QString& nick, const QString& message) {
    qDebug() << __func__ << f << nick << message;
    _im->addFriend(gloox::JID(stdstring(f)), nick, message);
}

void Messenger::acceptFriendRequest(const QString& f) {
    _im->acceptFriendRequest(f);
}

void Messenger::rejectFriendRequest(const QString& f) {
    _im->rejectFriendRequest(f);
}

void Messenger::getFriendVCard(const QString& f) {
    _im->fetchFriendVCard(f);
}

bool Messenger::removeFriend(const QString& f) {
    return _im->removeFriend(gloox::JID(f.toStdString()));
}

size_t Messenger::getFriendCount() {
    return _im->getRosterCount();
}

void Messenger::getFriendList(std::list<IMFriend>& list) {
    _im->getRosterList(list);
}

void Messenger::setFriendAlias(const QString& f, const QString& alias) {
    _im->setFriendAlias(gloox::JID(stdstring(f)), stdstring(alias));
}

IMStatus Messenger::getFriendStatus(const QString& f) {
    return _im->getFriendStatus(f);
}



void Messenger::addFriendHandler(FriendHandler* handler) {
    _im->addFriendHandler(handler);
}

void Messenger::addGroupHandler(GroupHandler* handler) {
    _im->addGroupHandler(handler);
}

void Messenger::stop() {
    _im->stop();
}

void Messenger::doConnect() {
    _im->doConnect();
}

void Messenger::send(const QString& xml) {
    _im->send(xml);
}

IMContactId Messenger::getSelfId() const {
    return _im->getSelfId();
}

IMPeerId Messenger::getSelfPeerId() const
{
    return _im->getSelfPeerId();
}

IMStatus Messenger::getSelfStatus() const {
    auto pt = gloox::Presence::PresenceType::Available;
    return static_cast<IMStatus>(pt);
}

void Messenger::addIMHandler(IMHandler *h)
{
    _im->addIMHandler(h);
}

void Messenger::addSelfHandler(SelfHandler *h)
{
    _im->addSelfHandler(h);
}

void Messenger::setSelfNickname(const QString& nickname) {
    _im->setNickname(nickname);
}

QString Messenger::getSelfUsername() const {
    return _im->getSelfUsername();
}

QString Messenger::getSelfNick() const {
    return _im->getNickname();
}

void Messenger::changePassword(const QString& password) {
    _im->changePassword(password);
}

void Messenger::onEncryptedMessage(QString xml) {
#ifdef OK_PLUGIN
    if (xml.isNull()) {
        qWarning() << "Empty encryptedMessage!";
        return;
    }
    auto dom = ok::base::Xmls::parse(xml);

    qDebug() << "onEncryptedMessage:" << dom.toString();
    auto _session = ok::Application::Instance()->getSession();
    auto info = _session->getSignInInfo();

    _session->account()->setJid(ok::base::Jid(qstring(_im->self().full())));
    auto pm = ok::plugin::PluginManager::instance();
    pm->addAccount(_session->account(), this);

    auto ele = dom.documentElement();
    bool decrypted = pm->decryptMessageElement(_session->account(), ele);
    qDebug() << "decrypt message=>" << decrypted;

    auto body = ele.firstChildElement("body").text();
    if (body.isEmpty()) {
        qWarning() << "Empty body!";
        return;
    }

    auto from = ele.attribute("from");
    auto to = ele.attribute("to");
    auto id = ele.attribute("id");

    //TODO 加密
    auto msg = IMMessage{MsgType::Chat, id, from, to, body, QDateTime::currentDateTime()};

    // for (auto handler : friendHandlers) {
    //     handler->onFriendMessage(qstring(gloox::JID(stdstring(from)).bareJID().bare()), msg);
    // }
#endif
}

void Messenger::loadGroupList() {
    _im->loadGroupList();
}

QString Messenger::createGroup(const QString& groupId, const QString& groupName) {
    gloox::JID self = _im->self();
    self.setUsername(stdstring(groupId));
    self.setResource(stdstring(groupName));
    self.setServer("conference." + self.server());

    _im->createRoom(self);
    return qstring(self.bare());
}

bool Messenger::inviteGroup(const IMContactId& group, const IMContactId& f) {
    return _im->inviteToRoom(gloox::JID(stdstring(group.toString())),
                             gloox::JID(stdstring(f.toString())));
}

bool Messenger::leaveGroup(const QString& group) {
    return _im->leaveGroup(group);
}

bool Messenger::destroyGroup(const QString& group) {
    return _im->destroyGroup(group);
}

void Messenger::setRoomName(const QString& group, const QString& nick) {
    _im->setRoomName(group, stdstring(nick));
}

void Messenger::setRoomDesc(const QString& group, const QString& nick) {
    _im->setRoomDesc(group, stdstring(nick));
}

void Messenger::setRoomSubject(const QString& group, const QString& notice) {
    _im->setRoomSubject(group, stdstring(notice));
}

void Messenger::setRoomAlias(const QString& group, const QString& alias) {
    _im->setRoomAlias(group, stdstring(alias));
}

void Messenger::joinGroup(const QString& group) {
    qDebug() << QString("group:%1").arg(group);
    _im->joinRoom(group);
}

void Messenger::setSelfAvatar(const QByteArray& avatar) {
    _im->setAvatar(avatar);
}

void Messenger::requestBookmarks() {
    _im->requestVCards();
    // im->requestBookmarks();
    // im->enablePubSubManager();
}

// Call
MessengerCall::MessengerCall(Messenger* messenger, QObject* parent)
        : QObject(parent), call{nullptr} {
    call = new IMCall(messenger->im(), this);
}

MessengerCall::~MessengerCall() {
    call->deleteLater();
}

void MessengerCall::addCallHandler(CallHandler* h) {
    call->addCallHandler(h);
}
bool MessengerCall::callToFriend(const QString& f, const QString& sId, bool video) {
    return call->callToFriend(f, sId, video);
}
bool MessengerCall::callToPeerId(const IMPeerId& to, const QString& sId, bool video) {
    return call->callToPeerId(to, sId, video);
}
bool MessengerCall::callAnswerToFriend(const IMPeerId& peer, const QString& callId, bool video) {
    return call->callAnswerToFriend(peer, callId, video);
}
void MessengerCall::callRetract(const IMContactId& f, const QString& sId) {
    call->callRetract(f, sId);
}
void MessengerCall::callReject(const IMPeerId& f, const QString& sId) {
    call->callReject(f, sId);
}

void MessengerCall::setMute(ortc::CtrlState state) {
    call->setCtrlState(state);
}

// File
MessengerFile::MessengerFile(Messenger* messenger, QObject* parent)
        : QObject(parent), fileSender{nullptr} {
    fileSender = new IMFile(messenger->im(), this);
}

MessengerFile::~MessengerFile() {
    fileSender->deleteLater();
}

void MessengerFile::fileRejectRequest(QString friendId, const File& file) {
    fileSender->fileRejectRequest(friendId, file);
}
void MessengerFile::fileAcceptRequest(QString friendId, const File& file) {
    fileSender->fileAcceptRequest(friendId, file);
}
void MessengerFile::fileFinishRequest(QString friendId, const QString& sId) {
    fileSender->fileFinishRequest(friendId, sId);
}
void MessengerFile::fileFinishTransfer(QString friendId, const QString& sId) {
    fileSender->fileFinishTransfer(friendId, sId);
}

void MessengerFile::fileCancel(QString fileId) {
    fileSender->fileCancel(fileId);
}

bool MessengerFile::fileSendToFriend(const QString& f, const File& file) {
    return fileSender->fileSendToFriend(f, file);
}

void MessengerFile::addFileHandler(FileHandler* h) {
    fileSender->addFileHandler(h);
}

MessengerMeet::MessengerMeet(lib::messenger::Messenger* messenger, QObject* parent)
        : QObject(parent), meet{nullptr} {
    meet = new IMMeet(messenger->im());
}

MessengerMeet::~MessengerMeet() {
    meet->deleteLater();
}

void MessengerMeet::create(const QString& room) {
    meet->create(room);
}

void MessengerMeet::addHandler(MessengerMeetHandler* hdr) {
    meet->addMeetHandler(hdr);
}

void MessengerMeet::removeHandler(MessengerMeetHandler* hdr) {
    meet->removeMeetHandler(hdr);
}

void MessengerMeet::onCall(const IMPeerId& peerId, const QString& callId, bool audio, bool video) {}

void MessengerMeet::onCallRetract(const QString& friendId, CallState state) {}

void MessengerMeet::onCallAcceptByOther(const QString& callId, const IMPeerId& peerId) {}

void MessengerMeet::onPeerConnectionChange(IMPeerId friendId, QString callId,
                                           ortc::PeerConnectionState state) {}

void MessengerMeet::receiveCallStateAccepted(IMPeerId friendId, QString callId, bool video) {}

void MessengerMeet::receiveCallStateRejected(IMPeerId friendId, QString callId, bool video) {}

void MessengerMeet::onHangup(const QString& friendId, CallState state) {}

void MessengerMeet::onSelfVideoFrame(uint16_t w, uint16_t h, const uint8_t* y, const uint8_t* u,
                                     const uint8_t* v, int32_t ystride, int32_t ustride,
                                     int32_t vstride) {}

void MessengerMeet::onFriendVideoFrame(const QString& friendId, uint16_t w, uint16_t h,
                                       const uint8_t* y, const uint8_t* u, const uint8_t* v,
                                       int32_t ystride, int32_t ustride, int32_t vstride) {}

void MessengerMeet::leave() {
    meet->leave();
}

[[maybe_unused]] std::vector<std::string> MessengerMeet::getVideoDeviceList() {
    return meet->getVideoDeviceList();
}

void MessengerMeet::sendMessage(const QString& msg) {
    meet->sendMessage(msg);
}

void MessengerMeet::setCtrlState(ortc::CtrlState state) {
    meet->setEnable(state);
}

}  // namespace lib::messenger
