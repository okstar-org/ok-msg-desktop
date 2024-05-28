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

#pragma once

#include "IMMessage.h"
#include "IMFriend.h"
#include "IMGroup.h"

#include "base/task.h"
#include "base/timer.h"
#include "tox/tox.h"
#include "tox/toxav.h"

#include <QDateTime>
#include <QString>
#include <array>
#include <cstddef>
#include <memory>
#include <utility>

class QDomElement;
class QDomDocument;

namespace lib {
namespace messenger {

class IMJingle;
class IMConference;
enum class IMStatus;

} // namespace messenger
} // namespace lib

namespace ok {
namespace session {
class AuthSession;
}
} // namespace ok

namespace lib {
namespace messenger {

class File;

class SelfHandler {
public:
  virtual void onSelfIdChanged(QString id) = 0;
  virtual void onSelfNameChanged(QString name) = 0;
  virtual void onSelfAvatarChanged(const std::string avatar) = 0;
  virtual void onSelfStatusChanged(Tox_User_Status status,
                                   const std::string &msg) = 0;
};

class FriendHandler {
public:
  virtual void onFriend(const IMFriend & frnd) = 0;
  virtual void onFriendRequest(QString friendId, QString msg) = 0;
  virtual void onFriendRemoved(QString friendId) = 0;
  virtual void onFriendStatus(QString friendId, Tox_User_Status status) = 0;
  virtual void onFriendMessage(QString friendId, IMMessage message) = 0;
  virtual void onFriendMessageSession(QString friendId, QString sid) = 0;
  virtual void onFriendNameChanged(QString friendId, QString name) = 0;
  virtual void onFriendAvatarChanged(const QString friendId,
                                     const std::string avatar) = 0;

  virtual void onFriendAliasChanged(const IMContactId & fId, const QString& alias)=0;

  virtual void onFriendChatState(QString friendId, int state) = 0;
  virtual void onMessageReceipt(QString friendId, QString receipt) = 0;
};

class GroupHandler {
public:
  virtual void onGroup(const QString groupId,
                       const QString name) = 0;

  virtual void onGroupInvite(const QString groupId, //
                             const QString peerId,  //
                             const QString message) = 0;

  virtual void onGroupSubjectChanged(const QString &groupId, const QString &subject)=0;

  virtual void onGroupMessage(const QString groupId, //
                              const IMPeerId peerId,   //
                              const IMMessage message) = 0;

  virtual void onGroupOccupants(const QString groupId, uint size) = 0;

  virtual void onGroupInfo(QString groupId, IMGroup groupInfo) = 0;

  virtual void onGroupOccupantStatus(const QString groupId, //
                                     IMGroupOccupant) = 0;
};

class CallHandler {
public:
  virtual void onCall(const QString &friendId, //
                      const QString &callId,   //
                      bool audio, bool video) = 0;

  virtual void onCallRetract(const QString &friendId, //
                      int state) = 0;

  virtual void onCallAcceptByOther(const QString& callId, const IMPeerId & peerId) = 0;

  virtual void receiveCallStateAccepted(IMPeerId friendId, //
                                        QString callId,  //
                                        bool video) = 0;

  virtual void receiveCallStateRejected(IMPeerId friendId, //
                                        QString callId,  //
                                        bool video) = 0;

  virtual void onHangup(const QString &friendId, //
                        TOXAV_FRIEND_CALL_STATE state) = 0;

  virtual void onSelfVideoFrame(uint16_t w, uint16_t h, //
                                const uint8_t *y,       //
                                const uint8_t *u,       //
                                const uint8_t *v,       //
                                int32_t ystride,        //
                                int32_t ustride,        //
                                int32_t vstride) = 0;

  virtual void onFriendVideoFrame(const QString &friendId, //
                                  uint16_t w, uint16_t h,  //
                                  const uint8_t *y,        //
                                  const uint8_t *u,        //
                                  const uint8_t *v,        //
                                  int32_t ystride,         //
                                  int32_t ustride,         //
                                  int32_t vstride) = 0;
};

class FileHandler {
public:

  virtual void onFileRequest(const QString &friendId, const File &file) = 0;
  virtual void onFileRecvChunk(const QString &friendId, const QString &fileId,
                               int seq, const std::string &chunk) = 0;
  virtual void onFileRecvFinished(const QString &friendId,
                                  const QString &fileId) = 0;
  virtual void onFileSendInfo(const QString &friendId, const File &file,
                              int m_seq, int m_sentBytes, bool end) = 0;
  virtual void onFileSendAbort(const QString &friendId, const File &file,
                               int m_sentBytes) = 0;
  virtual void onFileSendError(const QString &friendId, const File &file,
                               int m_sentBytes) = 0;
};

class Messenger : public QObject {
  Q_OBJECT
public:
//  using Ptr = std::shared_ptr<Messenger>;
  explicit Messenger(QObject *parent = nullptr);
  ~Messenger() override;

//  static Messenger *getInstance();

  void start();
  void stop();

  void send(const QString &xml);

  IMPeerId getSelfId() const;
  QString getSelfUsername() const;
  QString getSelfNick() const;
  Tox_User_Status getSelfStatus() const;

  void addSelfHandler(SelfHandler *);
  void addGroupHandler(GroupHandler *);
  void addCallHandler(CallHandler *);
  void addFileHandler(FileHandler *);


  bool sendToGroup(const QString &g, const QString &msg, QString &receiptNum);



  void receiptReceived(const QString &f, QString receipt);

  bool sendFileToFriend(const QString &f, const File &file);


  bool connectJingle();

  QString genUniqueId();

  /** self */
  void setSelfNickname(const QString &nickname);
  void changePassword(const QString &password);
  void setSelfAvatar(const QByteArray &avatar);
  // void setMute(bool mute);

  /**
   * IMFriend (audio/video)
   */
  void addFriendHandler(FriendHandler *);

  size_t getFriendCount();

  void getFriendList(std::list<lib::messenger::IMFriend> &);

  void setFriendAlias(const QString &f, const QString &alias);

  // 添加好友
  void sendFriendRequest(const QString &username, const QString &nick, const QString &message);
  // 接受朋友邀请
  void acceptFriendRequest(const QString &f);
  // 拒绝朋友邀请
  void rejectFriendRequest(const QString &f);
  
  void getFriendVCard(const QString &f);

  Tox_User_Status getFriendStatus(const QString &f);

  bool sendToFriend(const QString &f, const QString &msg, QString &receiptNum,
                    bool encrypt = false);

  // 发起呼叫邀请
  bool callToFriend(const QString &f, const QString &sId, bool video);

  // 创建呼叫
  bool createCallToPeerId(const IMPeerId &to, const QString &sId,
                          bool video);

  bool answerToFriend(const QString &f, const QString &callId, bool video);
  bool cancelToFriend(const QString &f, const QString &sId);
  bool removeFriend(const QString &f);
  // 静音功能
  void setMute(bool mute);
  void setRemoteMute(bool mute);
  void sendChatState(const QString &friendId, int state);
  /**
   * Group
   */
  void loadGroupList();
  bool initRoom();
  bool callToGroup(const QString &g);
  bool createGroup(const QString &group);
  void joinGroup(const QString &group);
  void setRoomName(const QString &group, const QString &nick);
  void setRoomDesc(const QString &group, const QString &desc);
  void setRoomSubject(const QString &group, const QString &subject);
  void setRoomAlias(const QString &group, const QString &alias);
  bool inviteGroup(const QString &group, const QString &f);
  bool leaveGroup(const QString &group);
  bool destroyGroup(const QString &group);

  /**
   * File
   */
  void rejectFileRequest(QString friendId, const File &file);
  void acceptFileRequest(QString friendId, const File &file);
  void finishFileRequest(QString friendId, const QString &sId);
  void finishFileTransfer(QString friendId, const QString &sId);
  void cancelFile(QString fileId);


  void requestBookmarks();
  void setUIStarted();

private:


  bool connectIM();

  std::unique_ptr<lib::messenger::IMJingle> _jingle;
  std::unique_ptr<lib::messenger::IMConference> _conference;

  std::vector<FriendHandler *> friendHandlers;
  std::vector<SelfHandler *> selfHandlers;
  std::vector<GroupHandler *> groupHandlers;
  std::vector<CallHandler *> callHandlers;
  std::vector<FileHandler *> fileHandlers;

  size_t sentCount = 0;
  std::unique_ptr<base::DelayedCallTimer> _delayer;

signals:
  void started();
  void stopped();
  void connected();
  void disconnect();
  void incoming(const QString dom);

  void receivedGroupMessage(lib::messenger::IMMessage imMsg); //
  void messageSent(const IMMessage &message);                 //

  void receiveSelfVideoFrame(uint16_t w, uint16_t h, //
                             const uint8_t *y,       //
                             const uint8_t *u,       //
                             const uint8_t *v,       //
                             int32_t ystride,        //
                             int32_t ustride,        //
                             int32_t vstride);

  void receiveFriendVideoFrame(const QString &friendId, //
                               uint16_t w, uint16_t h,  //
                               const uint8_t *y,        //
                               const uint8_t *u,        //
                               const uint8_t *v,        //
                               int32_t ystride,         //
                               int32_t ustride,         //
                               int32_t vstride);

private slots:
  void onConnectResult(lib::messenger::IMStatus);
  void onStarted();
  void onStopped();
  void onReceiveGroupMessage(lib::messenger::IMMessage imMsg);
  void onDisconnect();
  void onEncryptedMessage(QString dom);

  void onGroupReceived(QString groupId, QString name);

};

} // namespace messenger
} // namespace lib
