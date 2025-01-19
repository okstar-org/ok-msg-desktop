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

#include "Messenger.h"
#include <iostream>
#include <range/v3/all.hpp>
#include "IMFile.h"
#include "IMMeet.h"
#include "base/xmls.h"
#include "lib/messenger/IM.h"
#include "lib/messenger/IMCall.h"
// #include "lib/plugin/pluginmanager.h"

namespace lib::messenger {

Messenger::Messenger(const std::string& host, const std::string& name, const std::string& password)
        : _im{nullptr} {
    std::vector<std::string> features;
    // #ifdef OK_PLUGIN
    //     auto pm = ok::plugin::PluginManager::instance();
    //     for(auto &f: pm->pluginFeatures()){
    //         features.push_back(stdstring(f));
    //     }
    //     int acc = pm->addAccount(_session->account(), this);
    //     qDebug() << "PluginManager account id=>" << acc;
    // #endif
    _im = new IM(host, name, password, features);
}

Messenger::~Messenger() {
    //    qDebug() << __func__;
}

void Messenger::start() {
    _im->start();
}

bool Messenger::isStarted() const {
    return _im->isStarted();
}

void Messenger::sendChatState(const std::string& friendId, int state) {
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

std::string Messenger::genUniqueId() {
    return (_im->getClient()->getID());
}

bool Messenger::sendToGroup(const std::string& g, const std::string& msg, const std::string& id) {
    //    std::cout << std::string("sendToGroup=>%1 id:%2 msg:%2").arg(g).arg(id).arg(msg);
    sentCount++;
    return _im->sendToRoom(g, msg, id);
}

bool Messenger::sendToFriend(const std::string& f,
                             const std::string& msg,
                             const std::string& id,
                             bool encrypt) {
    std::cout << __func__ << msg << "=>" << f;
    sentCount++;
    bool y = false;
    if (encrypt) {
        std::cerr << "Encrypt message!";
        return false;
        // #ifdef OK_PLUGIN
        //         auto _session = ok::Application::Instance()->getSession();
        //
        //         ok::base::Jid ownJid(qstring(_im->self().full()));
        //         _session->account()->setJid(ownJid);
        //
        //         auto pm = ok::plugin::PluginManager::instance();
        //         pm->addAccount(_session->account(), this);
        //
        //         auto dom = _im->buildMessage(f, msg, id);
        //         auto ele = dom.documentElement();
        //
        //         if (pm->encryptMessageElement(_session->account(), ele)) {
        //             std::cout << "encryptMessageElement=>" << ele.ownerDocument().toString();
        //             auto xml = ok::base::Xmls::format(ele);
        //             _im->send(xml);
        //             y = true;
        //         }
        // #endif
    }
    if (!y) {
        y = _im->sendTo(f, msg, id);
    }
    return y;
}

void Messenger::receiptReceived(const std::string& f, std::string receipt) {
    return _im->sendReceiptReceived(f, receipt);
}

void Messenger::sendFriendRequest(const std::string& f, const std::string& nick,
                                  const std::string& message) {
    std::cout << __func__ << f << nick << message;
    _im->addFriend(gloox::JID((f)), nick, message);
}

void Messenger::acceptFriendRequest(const std::string& f) {
    _im->acceptFriendRequest(f);
}

void Messenger::rejectFriendRequest(const std::string& f) {
    _im->rejectFriendRequest(f);
}

void Messenger::getFriendVCard(const std::string& f) {
    _im->fetchFriendVCard(f);
}

bool Messenger::removeFriend(const std::string& f) {
    return _im->removeFriend(gloox::JID(f));
}

size_t Messenger::getFriendCount() {
    return _im->getRosterCount();
}

void Messenger::getFriendList(std::list<IMFriend>& list) {
    _im->getRosterList(list);
}

void Messenger::setFriendAlias(const std::string& f, const std::string& alias) {
    _im->setFriendAlias(gloox::JID((f)), (alias));
}

IMStatus Messenger::getFriendStatus(const std::string& f) {
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

void Messenger::send(const std::string& xml) {
    _im->send(xml);
}

IMContactId Messenger::getSelfId() const {
    return _im->getSelfId();
}

IMPeerId Messenger::getSelfPeerId() const {
    return _im->getSelfPeerId();
}

IMStatus Messenger::getSelfStatus() const {
    auto pt = gloox::Presence::PresenceType::Available;
    return static_cast<IMStatus>(pt);
}

void Messenger::addIMHandler(IMHandler* h) {
    _im->addIMHandler(h);
}

void Messenger::addSelfHandler(SelfHandler* h) {
    _im->addSelfHandler(h);
}

void Messenger::setSelfNickname(const std::string& nickname) {
    _im->setNickname(nickname);
}

std::string Messenger::getSelfUsername() const {
    return _im->getSelfUsername();
}

std::string Messenger::getSelfNick() const {
    return _im->getNickname();
}

void Messenger::changePassword(const std::string& password) {
    _im->changePassword(password);
}
//
// void Messenger::onEncryptedMessage(std::string xml) {
// #ifdef OK_PLUGIN
//    if (xml.isNull()) {
//        qWarning() << "Empty encryptedMessage!";
//        return;
//    }
//    auto dom = ok::base::Xmls::parse(xml);
//
//    std::cout << "onEncryptedMessage:" << dom.toString();
//    auto _session = ok::Application::Instance()->getSession();
//    auto info = _session->getSignInInfo();
//
//    _session->account()->setJid(ok::base::Jid(qstring(_im->self().full())));
//    auto pm = ok::plugin::PluginManager::instance();
//    pm->addAccount(_session->account(), this);
//
//    auto ele = dom.documentElement();
//    bool decrypted = pm->decryptMessageElement(_session->account(), ele);
//    std::cout << "decrypt message=>" << decrypted;
//
//    auto body = ele.firstChildElement("body").text();
//    if (body.isEmpty()) {
//        qWarning() << "Empty body!";
//        return;
//    }
//
//    auto from = ele.attribute("from");
//    auto to = ele.attribute("to");
//    auto id = ele.attribute("id");
//
//    //TODO 加密
//    auto msg = IMMessage{MsgType::Chat, id, from, to, body, QDateTime::currentDateTime()};
//
//    // for (auto handler : friendHandlers) {
//    //     handler->onFriendMessage(qstring(gloox::JID((from)).bareJID().bare()), msg);
//    // }
// #endif
//}

void Messenger::loadGroupList() {
    _im->loadGroupList();
}

std::string Messenger::createGroup(const std::string& groupId, const std::string& groupName) {
    gloox::JID self = _im->self();
    self.setUsername((groupId));
    self.setResource((groupName));
    self.setServer("conference." + self.server());

    _im->createRoom(self);
    return (self.bare());
}

bool Messenger::inviteGroup(const IMContactId& group, const IMContactId& f) {
    return _im->inviteToRoom(gloox::JID((group.toString())), gloox::JID((f.toString())));
}

bool Messenger::leaveGroup(const std::string& group) {
    return _im->leaveGroup(group);
}

bool Messenger::destroyGroup(const std::string& group) {
    return _im->destroyGroup(group);
}

void Messenger::setRoomName(const std::string& group, const std::string& nick) {
    _im->setRoomName(group, (nick));
}

void Messenger::setRoomDesc(const std::string& group, const std::string& nick) {
    _im->setRoomDesc(group, (nick));
}

void Messenger::setRoomSubject(const std::string& group, const std::string& notice) {
    _im->setRoomSubject(group, (notice));
}

void Messenger::setRoomAlias(const std::string& group, const std::string& alias) {
    _im->setRoomAlias(group, (alias));
}

void Messenger::joinGroup(const std::string& group) {
    _im->joinRoom(group);
}

void Messenger::setSelfAvatar(const std::string& avatar) {
    _im->setAvatar(avatar);
}

void Messenger::requestBookmarks() {
    _im->requestVCards();
    // im->requestBookmarks();
    // im->enablePubSubManager();
}

// Call
MessengerCall::MessengerCall(Messenger* messenger) {
    call = new IMCall(messenger->im());
}

MessengerCall::~MessengerCall() {
    delete call;
}

void MessengerCall::addCallHandler(CallHandler* h) {
    call->addCallHandler(h);
}
bool MessengerCall::callToFriend(const std::string& f, const std::string& sId, bool video) {
    return call->callToFriend(f, sId, video);
}
bool MessengerCall::callToPeerId(const IMPeerId& to, const std::string& sId, bool video) {
    return call->callToPeerId(to, sId, video);
}
bool MessengerCall::callAnswerToFriend(const IMPeerId& peer, const std::string& callId,
                                       bool video) {
    return call->callAnswerToFriend(peer, callId, video);
}
void MessengerCall::callCancel(const IMContactId& f, const std::string& sId) {
    call->callCancel(f, sId);
}
void MessengerCall::callReject(const IMPeerId& f, const std::string& sId) {
    call->callReject(f, sId);
}

void MessengerCall::setCtrlState(ortc::CtrlState state) {
    call->setCtrlState(state);
}

void MessengerCall::setSpeakerVolume(uint32_t vol) {
    call->setSpeakerVolume(vol);
}

// File
MessengerFile::MessengerFile(Messenger* messenger) : fileSender{nullptr} {
    fileSender = new IMFile(messenger->im());
}

MessengerFile::~MessengerFile() {
    delete fileSender;
}

void MessengerFile::addHandler(FileHandler* h) {
    fileSender->addHandler(h);
}

void MessengerFile::fileRejectRequest(std::string friendId, const File& file) {
    fileSender->fileRejectRequest(friendId, file);
}
void MessengerFile::fileAcceptRequest(std::string friendId, const File& file) {
    fileSender->fileAcceptRequest(friendId, file);
}
void MessengerFile::fileFinishRequest(std::string friendId, const std::string& sId) {
    fileSender->fileFinishRequest(friendId, sId);
}
void MessengerFile::fileFinishTransfer(std::string friendId, const std::string& sId) {
    fileSender->fileFinishTransfer(friendId, sId);
}

void MessengerFile::fileCancel(std::string fileId) {
    fileSender->fileCancel(fileId);
}

bool MessengerFile::fileSendToFriend(const std::string& friendId, const File& file) {
    return fileSender->fileSendToFriend(friendId, file);
}

MessengerMeet::MessengerMeet(lib::messenger::Messenger* messenger) : meet{nullptr} {
    meet = new IMMeet(messenger->im());
}

MessengerMeet::~MessengerMeet() {
    delete meet;
}

void MessengerMeet::create(const std::string& room) {
    meet->create(room);
}

void MessengerMeet::addHandler(MessengerMeetHandler* hdr) {
    meet->addMeetHandler(hdr);
}

void MessengerMeet::removeHandler(MessengerMeetHandler* hdr) {
    meet->removeMeetHandler(hdr);
}

void MessengerMeet::onCall(const IMPeerId& peerId, const std::string& callId, bool audio,
                           bool video) {}

void MessengerMeet::onCallCreated(const IMPeerId& peerId, const std::string& callId) {}

void MessengerMeet::onCallRetract(const IMPeerId& peerId, CallState state) {}

void MessengerMeet::onCallAcceptByOther(const IMPeerId& peerId, const std::string& callId) {}

void MessengerMeet::onPeerConnectionChange(const IMPeerId& peerId, const std::string& callId,
                                           ortc::PeerConnectionState state) {}

void MessengerMeet::onIceGatheringChange(const IMPeerId& friendId, const std::string& callId,
                                         ortc::IceGatheringState state) {}

void MessengerMeet::onIceConnectionChange(const IMPeerId& peerId, const std::string& callId,
                                          ortc::IceConnectionState state) {}

void MessengerMeet::receiveCallStateAccepted(const IMPeerId& peerId, const std::string& callId,
                                             bool video) {}

void MessengerMeet::receiveCallStateRejected(const IMPeerId& peerId, const std::string& callId,
                                             bool video) {}

void MessengerMeet::onHangup(const IMPeerId& peerId, CallState state) {}

void MessengerMeet::onEnd(const IMPeerId& peerId) {}

void MessengerMeet::onSelfVideoFrame(uint16_t w, uint16_t h, const uint8_t* y, const uint8_t* u,
                                     const uint8_t* v, int32_t ystride, int32_t ustride,
                                     int32_t vstride) {}

void MessengerMeet::onFriendVideoFrame(const std::string& friendId, uint16_t w, uint16_t h,
                                       const uint8_t* y, const uint8_t* u, const uint8_t* v,
                                       int32_t ystride, int32_t ustride, int32_t vstride) {}

void MessengerMeet::leave() {
    meet->leave();
}

[[maybe_unused]] std::vector<std::string> MessengerMeet::getVideoDeviceList() {
    return meet->getVideoDeviceList();
}

void MessengerMeet::sendMessage(const std::string& msg) {
    meet->sendMessage(msg);
}

void MessengerMeet::setCtrlState(ortc::CtrlState state) {
    meet->setEnable(state);
}

}  // namespace lib::messenger
