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

#ifndef CORE_HPP
#define CORE_HPP

#include "icorefriendmessagesender.h"
#include "icoregroupmessagesender.h"
#include "icoregroupquery.h"
#include "icoreidhandler.h"
#include "lib/session/AuthSession.h"
#include "src/model/FriendId.h"
#include "src/model/MsgId.h"
#include "src/model/groupid.h"
#include "toxfile.h"
#include "toxid.h"

#include "base/compatiblerecursivemutex.h"
#include "lib/messenger/Messenger.h"

#include "base/strongtype.h"
#include "src/model/VCard.h"
#include "src/model/friendlist.h"
#include "src/model/status.h"

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>

#include <functional>
#include <memory>

#include <src/model/message.h>

namespace module::im {

class IAudioControl;
class ICoreSettings;
class GroupInvite;
class Friend;
class Profile;
class Core;

using ToxCorePtr = std::unique_ptr<Core>;

/**
 * IM Core 聊天核心
 * 维护av(音视频)、file(文件传输)
 */
class Core : public QObject,
             public ICoreIdHandler,
             public ICoreFriendMessageSender,
             public ICoreGroupMessageSender,
             public ICoreGroupQuery,
             public lib::messenger::FriendHandler,
             public lib::messenger::GroupHandler,
             public lib::messenger::SelfHandler,
             public lib::messenger::IMHandler {
    Q_OBJECT
public:
    enum class ToxCoreErrors { BAD_PROXY, INVALID_SAVE, FAILED_TO_START, ERROR_ALLOC };

    static ToxCorePtr makeToxCore(lib::messenger::Messenger* messenger,
                                  const ICoreSettings* const settings,
                                  ToxCoreErrors* err = nullptr);
    static Core* getInstance();
    //  const CoreAV *getAv() const;
    //  CoreAV *getAv();
    //    CoreFile* getCoreFile() const;
    ~Core() override;

    lib::messenger::Messenger* getMessenger() {
        return messenger;
    }
    static const QString TOX_EXT;
    static QStringList splitMessage(const QString& message);
    QString getPeerName(const FriendId& id) const;
    void loadFriendList(std::list<FriendInfo>&) const;
    FriendList& getFriendList() {
        return friendList;
    }
    void loadGroupList() const;
    GroupId getGroupPersistentId(QString groupId) const override;
    uint32_t getGroupNumberPeers(QString groupId) const override;
    QString getGroupPeerName(QString groupId, QString peerId) const override;
    ToxPeer getGroupPeerPk(QString groupId, QString peerId) const override;
    QStringList getGroupPeerNames(QString groupId) const override;
    bool getGroupAvEnabled(QString groupId) const override;
    FriendId getFriendPublicKey(QString friendNumber) const;

    QString getFriendUsername(QString friendNumber) const;
    void setFriendAlias(const QString& friendId, const QString& alias);

    void getFriendInfo(const QString& friendNumber) const;
    Status getFriendStatus(const QString& friendNumber) const;

    bool isFriendOnline(QString friendId) const;
    bool hasFriendWithPublicKey(const FriendId& publicKey) const;
    QString joinGroupchat(const GroupInvite& inviteInfo);
    void joinRoom(const QString& groupId);
    void quitGroupChat(const QString& groupId) const;

    QString getUsername() const override;
    QString getNick() const override;
    Status getStatus() const;
    QString getStatusMessage() const;
    ToxId getSelfPeerId() const override;
    FriendId getSelfId() const override;
    QPair<QByteArray, QByteArray> getKeypair() const;

    void sendFile(QString friendId, QString filename, QString filePath, long long filesize);

    void requestBookmarks();

    void start();
    void stop();

    QByteArray getToxSaveData();

    void acceptFriendRequest(const FriendId& friendPk);
    void rejectFriendRequest(const FriendId& friendPk);
    bool removeFriend(QString friendId);
    void requestFriendship(const FriendId& friendAddress, const QString& nick,
                           const QString& message);
    // FriendSender
    bool sendMessage(QString friendId, const QString& message, const MsgId& msgId,
                     bool encrypt = false) override;
    bool sendAction(QString friendId, const QString& action, const MsgId& msgId,
                    bool encrypt = false) override;
    void sendTyping(QString friendId, bool typing);

    GroupId createGroup(const QString& name = "");
    void inviteToGroup(const ContactId& friendId, const GroupId& groupId);
    void leaveGroup(QString groupId);
    void destroyGroup(QString groupId);

    void setStatus(Status status);
    void setNick(const QString& nick);
    void setPassword(const QString& password);
    void setStatusMessage(const QString& message);
    void setAvatar(const QByteArray& avatar);

    // GroupSender
    bool sendGroupMessage(QString groupId, const QString& message, const MsgId& id) override;
    bool sendGroupAction(QString groupId, const QString& message, const MsgId& id) override;

    void setGroupName(const QString& groupId, const QString& name);
    void setGroupSubject(const QString& groupId, const QString& subject);
    void setGroupDesc(const QString& groupId, const QString& desc);
    void setGroupAlias(const QString& groupId, const QString& alias);

    void logout();

private:
    Core(QThread* coreThread);

    /**
     *    SelfHandler
     */
    void onSelfIdChanged(const std::string& id) override;
    void onSelfNameChanged(const std::string& name) override;
    void onSelfAvatarChanged(const std::string& avatar) override;
    void onSelfStatusChanged(lib::messenger::IMStatus status, const std::string& msg) override;
    void onSelfVCardChanged(lib::messenger::IMVCard& imvCard) override;

    bool sendGroupMessageWithType(QString groupId, const QString& message, const MsgId& msgId);

    bool sendMessageWithType(QString friendId, const QString& message, const MsgId& msgId,
                             bool encrypt = false);

    void sendReceiptReceived(const QString& friendId, const QString& receipt);

    void makeTox(QByteArray savedata, ICoreSettings* s);
    void loadFriends();
    void loadGroups();
    void bootstrapDht();

    void checkLastOnline(QString friendId);

    QString getFriendRequestErrorMessage(const ToxId& friendId, const QString& message) const;
    void registerCallbacks(lib::messenger::Messenger* messenger);

    /**
     * IMHandler
     */
    void onConnecting() override;
    void onConnected() override;
    void onDisconnected(int) override;
    void onStarted() override;
    void onStopped() override;

    /**
     * FriendHandler
     * @param list
     */

    void onFriend(const lib::messenger::IMFriend& frnd) override;

    void onFriendRequest(const std::string& friendId, const std::string& name) override;

    void onFriendRemoved(const std::string& friendId) override;

    void onFriendStatus(const std::string& friendId, lib::messenger::IMStatus status) override;

    void onFriendMessage(const std::string& friendId,
                         const lib::messenger::IMMessage& message) override;

    void onFriendMessageReceipt(const std::string& friendId, const std::string& msgId) override;

    void onMessageSession(const std::string& cId, const std::string& sid) override;

    void onFriendChatState(const std::string& friendId, int state) override;

    void onFriendNickChanged(const std::string& friendId, const std::string& nick) override;

    void onFriendAvatarChanged(const std::string& friendId, const std::string& avatar) override;

    void onFriendAliasChanged(const lib::messenger::IMContactId& fId,
                              const std::string& alias) override;

    void onFriendVCard(const lib::messenger::IMContactId& fId,
                       const lib::messenger::IMVCard& imvCard) override;

    void onMessageReceipt(const std::string& friendId, const std::string& receipt) override;

    /**
     * GroupHandler
     */
    void onGroup(const std::string& groupId, const std::string& name) override;

    void onGroupInvite(const std::string& groupId,  //
                       const std::string& peerId,   //
                       const std::string& message) override;

    void onGroupSubjectChanged(const std::string& groupId, const std::string& subject) override;

    void onGroupMessage(const std::string& groupId,              //
                        const lib::messenger::IMPeerId& peerId,  //
                        const lib::messenger::IMMessage& message) override;

    void onGroupInfo(const std::string& groupId, const lib::messenger::IMGroup& groupInfo) override;

    void onGroupOccupants(const std::string& groupId, uint size) override;

    void onGroupOccupantStatus(const std::string& groupId,
                               const lib::messenger::IMGroupOccupant&) override;

private:
    //  struct ToxDeleter {
    //    void operator()(lib::messenger::Messenger *tox) {
    //      if (tox) {
    //        tox->stop();
    //      }
    //    }
    //  };
    FriendList friendList;

    lib::messenger::Messenger* messenger;
    MsgId m_receipt;
    QTimer* toxTimer = nullptr;

    mutable CompatibleRecursiveMutex mutex;

    std::unique_ptr<QThread> coreThread = nullptr;

    Status fromToxStatus(const lib::messenger::IMStatus& status) const;

signals:
    void started();
    void connecting();
    void connected();
    void disconnected(int err);

    void friendRequestReceived(const FriendId& friendPk, const QString& message);
    void friendAvatarChanged(const FriendId& friendPk, const QByteArray& avatar);
    void friendAliasChanged(const FriendId& fId, const QString& alias);
    void friendAvatarRemoved(const FriendId& fId);
    void friendVCardSet(const FriendId& fId, const VCard& imvCard);

    void requestSent(const FriendId& friendPk, const QString& message);
    void failedToAddFriend(const FriendId& friendPk, const QString& errorInfo = QString());

    void usernameSet(const QString& username);
    void avatarSet(QByteArray avatar);
    void statusMessageSet(const QString& message);
    void statusSet(Status status);
    void idSet(const ToxId& id);
    void vCardSet(const VCard& imvCard);

    void failedToSetUsername(const QString& username);
    void failedToSetStatusMessage(const QString& message);
    void failedToSetStatus(Status status);
    void failedToSetTyping(bool typing);

    void avReady();

    void saveRequest();

    void fileAvatarOfferReceived(QString friendId,  //
                                 QString fileId,    //
                                 const QByteArray& avatarHash);

    void messageSessionReceived(const ContactId& cId, const QString& sid);

    void friendMessageReceived(const FriendId friendId,      //
                               const FriendMessage message,  //
                               bool isAction);

    void friendAdded(const FriendInfo frnd);

    void friendStatusChanged(const FriendId& friendId, Status status);
    void friendStatusMessageChanged(const FriendId& friendId, const QString& message);
    void friendUsernameChanged(const FriendId& friendPk, const QString& username);
    void friendNicknameChanged(const FriendId& friendPk, const QString& nickname);
    void friendTypingChanged(const FriendId& friendId, bool isTyping);

    void friendRemoved(QString friendId);
    void friendLastSeenChanged(QString friendId, const QDateTime& dateTime);

    void emptyGroupCreated(QString groupnumber, const GroupId groupId,
                           const QString& title = QString());
    void groupInviteReceived(const GroupInvite& inviteInfo);

    void groupSubjectChanged(GroupId groupId, QString subject);

    void groupMessageReceived(GroupId groupId, GroupMessage msg);

    void groupNamelistChanged(QString groupnumber, QString peerId, uint8_t change);

    void groupPeerlistChanged(QString groupnumber);

    void groupPeerSizeChanged(QString groupnumber, const uint size);

    void groupPeerStatusChanged(QString groupnumber, GroupOccupant go);

    void groupPeerNameChanged(QString groupnumber, const FriendId& peerPk, const QString& newName);

    void groupInfoReceipt(const GroupId& groupId, const GroupInfo& info);

    void groupPeerAudioPlaying(QString groupnumber, FriendId peerPk);

    void groupSentFailed(QString groupId);

    void groupAdded(const GroupId& groupId, const QString& name);

    void actionSentResult(QString friendId, const QString& action, int success);

    void receiptRecieved(const FriendId& friedId, MsgId receipt);

    void failedToRemoveFriend(QString friendId);
private slots:
    void process();
};
}  // namespace module::im
#endif  // CORE_HPP
