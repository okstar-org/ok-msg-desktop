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

#include "FriendId.h"
#include "groupid.h"
#include "icorefriendmessagesender.h"
#include "icoregroupmessagesender.h"
#include "icoregroupquery.h"
#include "icoreidhandler.h"
#include "receiptnum.h"
#include "toxfile.h"
#include "toxid.h"

#include "base/compatiblerecursivemutex.h"
#include "lib/messenger/messenger.h"

#include "src/model/status.h"
#include "src/util/strongtype.h"

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>

#include <functional>
#include <memory>

#include <src/model/message.h>

class CoreAV;
class CoreFile;
class IAudioControl;
class ICoreSettings;
class GroupInvite;
class Friend;
class Profile;
class Core;

using ToxCorePtr = std::unique_ptr<Core>;

class Core : public QObject,
             public ICoreIdHandler,
             public ICoreFriendMessageSender,
             public ICoreGroupMessageSender,
             public ICoreGroupQuery,
             public lib::messenger::FriendHandler,
             public lib::messenger::GroupHandler,
             public lib::messenger::SelfHandler {
  Q_OBJECT

public:
  enum class ToxCoreErrors { BAD_PROXY, INVALID_SAVE, FAILED_TO_START, ERROR_ALLOC };

  static ToxCorePtr makeToxCore(const QByteArray &savedata, const ICoreSettings *const settings, ToxCoreErrors *err = nullptr);
  static Core *getInstance();
//  const CoreAV *getAv() const;
//  CoreAV *getAv();
  CoreFile *getCoreFile() const;
  ~Core();

  static const QString TOX_EXT;
  static QStringList splitMessage(const QString &message);
  QString getPeerName(const FriendId &id) const;
  void loadFriendList(std::list<FriendInfo> &) const;

  void loadGroupList() const;
  GroupId getGroupPersistentId(QString groupId) const override;
  uint32_t getGroupNumberPeers(QString groupId) const override;
  QString getGroupPeerName(QString groupId, QString peerId) const override;
  ToxPeer getGroupPeerPk(QString groupId, QString peerId) const override;
  QStringList getGroupPeerNames(QString groupId) const override;
  bool getGroupAvEnabled(QString groupId) const override;
  FriendId getFriendPublicKey(QString friendNumber) const;

  QString getFriendUsername(QString friendNumber) const;
  void setFriendAlias(const QString &friendId, const QString &alias);

  void getFriendInfo(const QString &friendNumber) const;
  Status::Status getFriendStatus(const QString &friendNumber) const;

  bool isFriendOnline(QString friendId) const;
  bool hasFriendWithPublicKey(const FriendId &publicKey) const;
  QString joinGroupchat(const GroupInvite &inviteInfo);
  void joinRoom(const QString &groupId);
  void quitGroupChat(const QString &groupId) const;

  QString getUsername() const override;
  QString getNick() const override;
  Status::Status getStatus() const;
  QString getStatusMessage() const;
  ToxId getSelfPeerId() const override;
  FriendId getSelfId() const override;
  QPair<QByteArray, QByteArray> getKeypair() const;

  void sendFile(QString friendId, QString filename, QString filePath, long long filesize);

  void requestBookmarks();
  void setUIStarted();

  void start();
  void stop();

  QByteArray getToxSaveData();

  void acceptFriendRequest(const FriendId &friendPk);
  void rejectFriendRequest(const FriendId &friendPk);
  void removeFriend(QString friendId);
  void requestFriendship(const FriendId &friendAddress, const QString &nick, const QString &message);
  // FriendSender
  bool sendMessage(QString friendId, const QString &message, ReceiptNum &receipt, bool encrypt = false) override;
  bool sendAction(QString friendId, const QString &action, ReceiptNum &receipt, bool encrypt = false) override;
  void sendTyping(QString friendId, bool typing);

  GroupId createGroup(const QString &name="");
  void inviteToGroup(const ContactId &friendId, const GroupId& groupId);
  void leaveGroup(QString groupId);
  void destroyGroup(QString groupId);

  void setStatus(Status::Status status);
  void setUsername(const QString &username);
  void setPassword(const QString &password);
  void setStatusMessage(const QString &message);
  void setAvatar(const QByteArray &avatar);

 // GroupSender
  QString sendGroupMessage(QString groupId, const QString &message) override;
  QString sendGroupAction(QString groupId, const QString &message) override;

  void setGroupName(const QString &groupId, const QString &name);
  void setGroupSubject(const QString &groupId, const QString &subject);
  void setGroupDesc(const QString &groupId, const QString &desc);
  void setGroupAlias(const QString &groupId, const QString &alias);



  void logout();

signals:
  void connected();
  void disconnected();

  void friendRequestReceived(const FriendId &friendPk, const QString &message);
  void friendAvatarChanged(const FriendId &friendPk, const QByteArray &avatar);
  void friendAliasChanged(const FriendId &fId, const QString &alias);
  void friendAvatarRemoved(const FriendId &friendPk);

  void requestSent(const FriendId &friendPk, const QString &message);
  void failedToAddFriend(const FriendId &friendPk, const QString &errorInfo = QString());

  void usernameSet(const QString &username);
  void avatarSet(QByteArray avatar);
  void statusMessageSet(const QString &message);
  void statusSet(Status::Status status);
  void idSet(const ToxId &id);

  void failedToSetUsername(const QString &username);
  void failedToSetStatusMessage(const QString &message);
  void failedToSetStatus(Status::Status status);
  void failedToSetTyping(bool typing);

  void avReady();

  void saveRequest();

  void fileAvatarOfferReceived(QString friendId, //
                               QString fileId,   //
                               const QByteArray &avatarHash);

  void messageSessionReceived(const ContactId &cId, const QString &sid);

  void friendMessageReceived(const FriendId &friendId,        //
                             const FriendMessage &message, //
                             bool isAction);

  void friendAdded(const FriendInfo frnd);

  void friendStatusChanged(const FriendId &friendId, Status::Status status);
  void friendStatusMessageChanged(const FriendId &friendId, const QString &message);
  void friendUsernameChanged(const FriendId &friendPk, const QString &username);
  void friendTypingChanged(const FriendId &friendId, bool isTyping);

  void friendRemoved(QString friendId);
  void friendLastSeenChanged(QString friendId, const QDateTime &dateTime);

  void emptyGroupCreated(QString groupnumber, const GroupId groupId, const QString &title = QString());
  void groupInviteReceived(const GroupInvite &inviteInfo);

  void groupSubjectChanged(GroupId groupId, QString subject);

  void groupMessageReceived(GroupId groupId, GroupMessage msg);

  void groupNamelistChanged(QString groupnumber, QString peerId, uint8_t change);

  void groupPeerlistChanged(QString groupnumber);

  void groupPeerSizeChanged(QString groupnumber, const uint size);

  void groupPeerStatusChanged(QString groupnumber, GroupOccupant go);

  void groupPeerNameChanged(QString groupnumber, const FriendId &peerPk, const QString &newName);

  void groupInfoReceipt(const GroupId &groupId, const GroupInfo &info);

  void groupPeerAudioPlaying(QString groupnumber, FriendId peerPk);

  void groupSentFailed(QString groupId);

  void groupAdded(const GroupId &groupId, const QString &name);

  void actionSentResult(QString friendId, const QString &action, int success);

  void receiptRecieved(const FriendId &friedId, ReceiptNum receipt);

  void failedToRemoveFriend(QString friendId);

private:
  Core(QThread *coreThread);

  /**
   *    SelfHandler
   */
  virtual void onSelfIdChanged(QString id) override;
  virtual void onSelfNameChanged(QString name) override;
  virtual void onSelfAvatarChanged(const std::string avatar) override;
  virtual void onSelfStatusChanged(lib::messenger::IMStatus status, const std::string &msg) override;

  //
  //  static void onFriendRequest(Tox *tox, const QString &cUserId,
  //                              const uint8_t *cMessage, size_t cMessageSize,
  //                              void *core);
  //
  //
  //  static void onGroupInvite(Tox *tox, QString receiver,
  //                            Tox_Conference_Type type, const uint8_t *cookie,
  //                            size_t length, void *vCore);

  //  static void onGroupPeerListChange(Tox *, QString groupId, void *core);
  //
  //  static void onGroupPeerNameChange(Tox *, QString groupId, QString peerId,
  //                                    const uint8_t *name, size_t length,
  //                                    void *core);
  //  static void onGroupTitleChange(Tox *tox, QString groupId, QString peerId,
  //                                 const uint8_t *cTitle, size_t length,
  //                                 void *vCore);
  //  static void onReadReceiptCallback(Tox *tox, QString receiver,
  //                                    ReceiptNum receipt, void *core);

  QString sendGroupMessageWithType(QString groupId, const QString &message);

  bool sendMessageWithType(QString friendId, const QString &message,
                           ReceiptNum &receipt, bool encrypt = false);

  void sendReceiptReceived(const QString &friendId, QString receipt);

  bool checkConnection();

  void makeTox(QByteArray savedata, ICoreSettings *s);
  void loadFriends();
  void loadGroups();
  void bootstrapDht();

  void checkLastOnline(QString friendId);

  QString getFriendRequestErrorMessage(const ToxId &friendId, const QString &message) const;
  void registerCallbacks(Tox *tox);

  /**
   * FriendHandler
   * @param list
   */

  virtual void onFriend(const lib::messenger::IMFriend &frnd) override;

  virtual void onFriendRequest(const QString friendId, QString name) override;

  virtual void onFriendRemoved(QString friendId) override;

  virtual void onFriendStatus(QString friendId, lib::messenger::IMStatus status) override;

  virtual void onFriendMessage(QString friendId, lib::messenger::IMMessage message) override;

  virtual void onMessageSession(QString cId, QString sid) override;

  virtual void onFriendChatState(QString friendId, int state) override;

  virtual void onFriendNameChanged(QString friendId, QString name) override;

  virtual void onFriendAvatarChanged(const QString friendId, const std::string avatar) override;

  virtual void onFriendAliasChanged(const lib::messenger::IMContactId &fId, const QString &alias) override;
  virtual void onMessageReceipt(QString friendId, ReceiptNum receipt) override;

  /**
   * GroupHandler
   */
  virtual void onGroup(const QString groupId, const QString name) override;

  virtual void onGroupInvite(const QString groupId, //
                             const QString peerId,  //
                             const QString message) override;
  virtual void onGroupSubjectChanged(const QString &groupId, const QString &subject) override;

  virtual void onGroupMessage(const QString groupId,                 //
                              const lib::messenger::IMPeerId peerId, //
                              const lib::messenger::IMMessage message) override;

  virtual void onGroupInfo(QString groupId, lib::messenger::IMGroup groupInfo) override;

  virtual void onGroupOccupants(const QString groupId, uint size) override;

  virtual void onGroupOccupantStatus(const QString groupId, lib::messenger::IMGroupOccupant) override;

private slots:
  void process();
  void onStarted();

private:
  struct ToxDeleter {
    void operator()(Tox *tox) {
      if (tox) {
        tox->stop();
      }
    }
  };

  using ToxPtr = std::unique_ptr<Tox, ToxDeleter>;
  ToxPtr tox;

  std::unique_ptr<CoreFile> file;
//  std::unique_ptr<CoreAV> av;
  ReceiptNum m_receipt;
  QTimer *toxTimer = nullptr;
  // recursive, since we might call our own functions
  mutable CompatibleRecursiveMutex coreLoopLock;

  std::unique_ptr<QThread> coreThread = nullptr;

  Status::Status fromToxStatus(const lib::messenger::IMStatus &status) const;
};

#endif // CORE_HPP
