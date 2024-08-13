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

#include <QThread>
#include <memory>

#include "application.h"
#include "base/logs.h"
#include "base/xmls.h"

#include "lib/messenger/IM.h"
#include "lib/messenger/IMConference.h"
#include "lib/messenger/IMFile.h"
#include "lib/messenger/IMCall.h"
#include "lib/plugin/pluginmanager.h"

namespace lib {
namespace messenger {

Messenger::Messenger(const QString& host,
                     const QString& name,
                     const QString& password,
                     QObject* parent)
        : QObject(parent)
        , _im{nullptr}
        , _delayer(std::make_unique<::base::DelayedCallTimer>())  //
{
    qDebug() << __func__;

    connect(this, &Messenger::disconnect, this, &Messenger::onDisconnect);

    auto _session = ok::Application::Instance()->getSession();

    QStringList features;
#ifdef OK_PLUGIN
    auto pm = ok::plugin::PluginManager::instance();
    features << pm->pluginFeatures();
    int acc = pm->addAccount(_session->account(), this);
    qDebug() << "PluginManager account id=>" << acc;
#endif

    _im = new ::lib::messenger::IM(host, name, password, features);
    connectIM();

    //    connect(_im, &::lib::messenger::IM::connectResult,
    //            this, &AuthSession::onIMConnectStatus);
    //    qRegisterMetaType<ok::session::SignInInfo>("ok::session::SignInInfo");

    //    connect(_im, &::lib::messenger::IM::started, this, &AuthSession::onIMStarted);

    //    connect(_im, &IM::connectResult, this, [pm, &_session](lib::messenger::IMConnectStatus
    //    status) {
    //      if(status == lib::messenger::IMConnectStatus::CONNECTED){
    //        pm->startLogin(_session->account());
    //      }
    //    });
}

Messenger::~Messenger() { qDebug() << __func__; }

// Messenger *Messenger::getInstance() {
//   static Messenger *self = nullptr;
//   if (!self)
//     self = new Messenger();
//   return self;
// }

void Messenger::start() {
    qDebug() << __func__;
    _im->start();
    connect(_im, &IM::started, [&]() {
        emit started();
    });
}

void Messenger::sendChatState(const QString& friendId, int state) {
    _im->sendChatState(friendId, static_cast<ChatStateType>(state));
}

void Messenger::onConnectResult(lib::messenger::IMConnectStatus status) {
    qDebug() << ("status:") << (int)status;
    if (status == lib::messenger::IMConnectStatus::DISCONNECTED) {
        _delayer->call(1000 * 5, [&]() {
            qDebug(("Retry connect..."));
            _im->doConnect();
        });
    }
}

void Messenger::onStarted() {
    qDebug() << __func__;

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

bool Messenger::connectIM() {
    connect(_im, &IM::started, this, &Messenger::onStarted);
    connect(
            _im, &IM::incoming, this, [=, this](QString xml) { emit incoming(xml); },
            Qt::QueuedConnection);

    /**
     * selfHandlers
     */
    connect(_im, &IM::selfNicknameChanged, this, [&](const QString& nickname) {
        for (auto handler : selfHandlers) {
            handler->onSelfNameChanged(nickname);
        }
    });
    connect(_im, &IM::selfAvatarChanged, this, [&](const std::string avatar) {
        for (auto handler : selfHandlers) {
            handler->onSelfAvatarChanged(avatar);
        }
    });
    connect(_im, &IM::selfStatusChanged, this, [&](int type, const std::string& status) {
        for (auto handler : selfHandlers) {
            handler->onSelfStatusChanged(static_cast<IMStatus>(type), status);
        }
    });
    connect(_im, &IM::selfIdChanged, this, [&](QString id) {
        for (auto handler : selfHandlers) {
            handler->onSelfIdChanged(id);
        }
    });

    /**
     * friendHandlers
     */
    connect(_im, &IM::receiveFriend, this, [&](const IMFriend& frnd) {
        for (auto handler : friendHandlers) {
            handler->onFriend(frnd);
        }
    });

    connect(_im, &IM::receiveFriendRequest, this, [&](const QString friendId, QString msg) -> void {
        for (auto handler : friendHandlers) {
            handler->onFriendRequest(friendId, msg);
        }
    });

    connect(_im, &IM::receiveMessageReceipt, this, [&](QString friendId, QString receipt) -> void {
        for (auto handler : friendHandlers) {
            handler->onMessageReceipt(friendId, receipt);
        }
    });

    connect(_im, &IM::receiveFriendStatus, this, [&](QString friendId, int type) -> void {
        for (auto handler : friendHandlers) {
            handler->onFriendStatus(friendId, static_cast<IMStatus>(type));
        }
    });

    connect(_im, &IM::receiveFriendChatState, this, [&](QString friendId, int state) -> void {
        for (auto handler : friendHandlers) {
            handler->onFriendChatState(friendId, state);
        }
    });

    connect(_im, &IM::receiveFriendMessage, this, [&](QString friendId, IMMessage msg) -> void {
        for (auto handler : friendHandlers) {
            handler->onFriendMessage(friendId, msg);
        }
    });

    connect(_im, &IM::receiveMessageSession, this, [&](QString contactId, QString sid) -> void {
        for (auto handler : friendHandlers) {
            handler->onMessageSession(contactId, sid);
        }
    });

    connect(_im, &IM::receiveNicknameChange, this, [&](QString friendId, QString nickname) {
        for (auto handler : friendHandlers) {
            handler->onFriendNickChanged(friendId, nickname);
        }
    });

    connect(_im, &IM::receiveFriendAvatarChanged, this, [&](QString friendId, std::string avatar) {
        for (auto handler : friendHandlers) {
            handler->onFriendAvatarChanged(friendId, avatar);
        }
    });

    connect(_im, &IM::receiveFriendAliasChanged, this,
            [&](const JID& friendId, const std::string& alias) {
                for (auto handler : friendHandlers) {
                    handler->onFriendAliasChanged(IMContactId(friendId.bareJID()), qstring(alias));
                }
            });

    // group
    connect(_im, &IM::groupInvite, this,
            [&](const QString& groupId, const QString& peerId, const QString& message) {
                for (auto handler : groupHandlers) {
                    handler->onGroupInvite(groupId, peerId, message);
                }
            });

    connect(_im, &IM::groupSubjectChanged, this,
            [&](const JID& group, const std::string& subject) -> void {
                for (auto handler : groupHandlers) {
                    handler->onGroupSubjectChanged(qstring(group.bare()), qstring(subject));
                }
            });

    connect(_im, &IM::groupReceived, this, &Messenger::onGroupReceived);

    connect(_im, &IM::receiveRoomMessage, this,
            [&](QString groupId, IMPeerId peerId, IMMessage msg) -> void {
                for (auto handler : groupHandlers) {
                    handler->onGroupMessage(groupId, peerId, msg);
                }
            });

    connect(_im, &IM::groupOccupants, this, [&](const QString& groupId, const uint size) -> void {
        for (auto handler : groupHandlers) {
            handler->onGroupOccupants(groupId, size);
        }
    });

    connect(_im, &IM::groupOccupantStatus, this,
            [&](const QString& groupId, const IMGroupOccupant& go) -> void {
                for (auto handler : groupHandlers) {
                    handler->onGroupOccupantStatus(groupId, go);
                }
            });

    qRegisterMetaType<IMGroup>("IMGroup");
    connect(_im, &IM::groupRoomInfo, this, [&](const QString& groupId, const IMGroup info) -> void {
        for (auto handler : groupHandlers) {
            handler->onGroupInfo(groupId, info);
        }
    });

    return true;
}

bool Messenger::initRoom() {
    _im->loadGroupList();
    return true;
}

void Messenger::onReceiveGroupMessage(IMMessage msg) { emit receivedGroupMessage(msg); }

QString Messenger::genUniqueId() { return qstring(_im->getClient()->getID()); }

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
    _im->addFriend(JID(stdstring(f)), nick, message);
}

void Messenger::acceptFriendRequest(const QString& f) { _im->acceptFriendRequest(f); }

void Messenger::rejectFriendRequest(const QString& f) { _im->rejectFriendRequest(f); }

void Messenger::getFriendVCard(const QString& f) { _im->fetchFriendVCard(f); }

bool Messenger::removeFriend(const QString& f) { return _im->removeFriend(JID(f.toStdString())); }

size_t Messenger::getFriendCount() { return _im->getRosterCount(); }

void Messenger::getFriendList(std::list<IMFriend>& list) { _im->getRosterList(list); }

void Messenger::setFriendAlias(const QString& f, const QString& alias) {
    _im->setFriendAlias(JID(stdstring(f)), stdstring(alias));
}

IMStatus Messenger::getFriendStatus(const QString& f) { return _im->getFriendStatus(f); }

void Messenger::addSelfHandler(SelfHandler* handler) { selfHandlers.push_back(handler); }

void Messenger::addFriendHandler(FriendHandler* handler) { friendHandlers.push_back(handler); }

void Messenger::addGroupHandler(GroupHandler* handler) { groupHandlers.push_back(handler); }

void Messenger::stop() { _im->stop(); }

void Messenger::send(const QString& xml) { _im->send(xml); }

IMPeerId Messenger::getSelfId() const { return _im->getSelfPeerId(); }

IMStatus Messenger::getSelfStatus() const {
    auto pt = gloox::Presence::PresenceType::Available;
    return static_cast<IMStatus>(pt);
}

void Messenger::setSelfNickname(const QString& nickname) { _im->setNickname(nickname); }

QString Messenger::getSelfUsername() const { return _im->getSelfUsername(); }

QString Messenger::getSelfNick() const { return _im->getNickname(); }

void Messenger::changePassword(const QString& password) { _im->changePassword(password); }

void Messenger::onDisconnect() {
    _delayer->call(1000 * 5, [&]() {
        qDebug(("retry connect..."));
        _im->start();
    });
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

    _session->account()->setJid(qstring(_im->self().full()));
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

    auto msg = IMMessage{MsgType::Chat, id, from, to, body, QDateTime::currentDateTime()};

    for (auto handler : friendHandlers) {
        handler->onFriendMessage(qstring(JID(stdstring(from)).bareJID().bare()), msg);
    }
#endif
}

void Messenger::loadGroupList() { _im->loadGroupList(); }

QString Messenger::createGroup(const QString& groupId, const QString& groupName) {
    JID self = _im->self();
    self.setUsername(stdstring(groupId));
    self.setResource(stdstring(groupName));
    self.setServer("conference." + self.server());

    _im->createRoom(self);
    return qstring(self.bare());
}

bool Messenger::inviteGroup(const IMContactId& group, const IMContactId& f) {
    return _im->inviteToRoom(JID(stdstring(group.toString())), JID(stdstring(f.toString())));
}

bool Messenger::leaveGroup(const QString& group) { return _im->leaveGroup(group); }

bool Messenger::destroyGroup(const QString& group) { return _im->destroyGroup(group); }

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

void Messenger::onGroupReceived(QString groupId, QString name) {
    for (auto handler : groupHandlers) {
        handler->onGroup(groupId, name);
    }
}

//Call
MessengerCall::MessengerCall(Messenger* messenger, QObject* parent) {
    call = messenger->imCall();
}

void MessengerCall::addCallHandler(CallHandler* h) {
    call->addCallHandler(h);
}
bool MessengerCall::callToFriend(const QString& f, const QString& sId, bool video) {
    return call->callToFriend(f, sId, video);
}
bool MessengerCall::callToPeerId(const IMPeerId& to, const QString& sId, bool video) {
    return call->callToPeerId(to, sId,video);
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
void MessengerCall::setMute(bool mute) {
    call->setMute(mute);
}
void MessengerCall::setRemoteMute(bool mute) {
    call->setRemoteMute(mute);
}

//File

MessengerFile::MessengerFile(Messenger* messenger, QObject* parent) : QObject(parent) {
    fileSender = new IMFile(messenger->im(), this);
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
void MessengerFile::addFileHandler(FileHandler* h) { fileSender->addFileHandler(h); }

}  // namespace messenger
}  // namespace lib
