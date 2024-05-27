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

//不要修改顺序和值
enum class FileStatus {
  INITIALIZING = 0,
  PAUSED = 1,
  TRANSMITTING = 2,
  BROKEN = 3,
  CANCELED = 4,
  FINISHED = 5,
};

//不要修改顺序和值
enum class FileDirection {
  SENDING = 0,
  RECEIVING = 1,
};

struct File {
  //id(file id = ibb id) 和 sId(session id)
  QString id;
  QString sId;
  QString name;
  QString path;
  quint64 size;
  FileStatus status;
  FileDirection direction;
  [[__nodiscard__ ]] QString toString() const;
  friend QDebug &operator<<(QDebug &debug, const File &f);
};


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
  virtual void onFriend(const Friend& frnd) = 0;
  virtual void onFriendRequest(QString friendId, QString msg) = 0;
  virtual void onFriendRemoved(QString friendId) = 0;
  virtual void onFriendStatus(QString friendId, Tox_User_Status status) = 0;
  virtual void onFriendMessage(QString friendId, IMMessage message) = 0;
  virtual void onFriendMessageSession(QString friendId, QString sid) = 0;
  virtual void onFriendNameChanged(QString friendId, QString name) = 0;
  virtual void onFriendAvatarChanged(const QString friendId,
                                     const std::string avatar) = 0;
  virtual void onFriendChatState(QString friendId, int state) = 0;
  virtual void onMessageReceipt(QString friendId, QString receipt) = 0;
};

struct GroupInfo {
  QString name;
  QString description;
  QString subject;
  QString creationdate;
  uint64_t occupants=0;
} ;

struct GroupOccupant{
    QString jid;
    QString nick;
    QString affiliation;
    QString role;
    int status;
    //https://xmpp.org/registrar/mucstatus.html
    QList<int> codes;
};

class GroupHandler {
public:
  virtual void onGroup(const QString groupId,
                       const QString name) = 0;

  virtual void onGroupInvite(const QString groupId, //
                             const QString peerId,  //
                             const QString message) = 0;

  virtual void onGroupMessage(const QString groupId, //
                              const PeerId peerId,   //
                              const IMMessage message) = 0;

  virtual void onGroupOccupants(const QString groupId, uint size) = 0;

  virtual void onGroupInfo(QString groupId, GroupInfo groupInfo) = 0;

  virtual void onGroupOccupantStatus(const QString groupId, //
                                    GroupOccupant) = 0;
};

class CallHandler {
public:
  virtual void onCall(const QString &friendId, //
                      const QString &callId,   //
                      bool audio, bool video) = 0;

  virtual void onCallRetract(const QString &friendId, //
                      int state) = 0;

  virtual void onCallAcceptByOther(const QString& callId, const PeerId& peerId) = 0;

  virtual void receiveCallStateAccepted(PeerId friendId, //
                                        QString callId,  //
                                        bool video) = 0;

  virtual void receiveCallStateRejected(PeerId friendId, //
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

  PeerId getSelfId() const;
  QString getSelfUsername() const;
  QString getSelfNick() const;
  Tox_User_Status getSelfStatus() const;

  void addSelfHandler(SelfHandler *);
  void addFriendHandler(FriendHandler *);
  void addGroupHandler(GroupHandler *);
  void addCallHandler(CallHandler *);
  void addFileHandler(FileHandler *);

  size_t getFriendCount();

  void getFriendList(std::list<lib::messenger::Friend> &);

  bool sendToGroup(const QString &g, const QString &msg, QString &receiptNum);

  bool sendToFriend(const QString &f, const QString &msg, QString &receiptNum,
                    bool encrypt = false);

  void receiptReceived(const QString &f, QString receipt);

  bool sendFileToFriend(const QString &f, const File &file);


  bool connectJingle();

  QString genUniqueId();

  // ============= setXX============/
  void setSelfNickname(const QString &nickname);
  void changePassword(const QString &password);
  void setSelfAvatar(const QByteArray &avatar);
  // void setMute(bool mute);

  /**
   * Friend (audio/video)
   */
  // 添加好友
  void sendFriendRequest(const QString &username, const QString &nick, const QString &message);
  // 接受朋友邀请
  void acceptFriendRequest(const QString &f);
  // 拒绝朋友邀请
  void rejectFriendRequest(const QString &f);
  
  void getFriendVCard(const QString &f);

  Tox_User_Status getFriendStatus(const QString &f);


  // 发起呼叫邀请
  bool callToFriend(const QString &f, const QString &sId, bool video);

  // 创建呼叫
  bool createCallToPeerId(const PeerId &to, const QString &sId,
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
