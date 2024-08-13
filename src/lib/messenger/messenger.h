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

#include "IMFriend.h"
#include "IMGroup.h"
#include "IMMessage.h"
#include "base/task.h"
#include "base/timer.h"

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

}  // namespace messenger
}  // namespace lib

namespace ok {
namespace session {
class AuthSession;
}
}  // namespace ok

namespace lib {
namespace messenger {

class IM;
class IMFile;
class IMCall;

/**
 * 连接状态
 *
 */
enum class IMConnectStatus {
    CONNECTING,
    AUTH_FAILED,
    CONNECTED,
    DISCONNECTED,
    TIMEOUT,
    CONN_ERROR,
    TLS_ERROR,
    OUT_OF_RESOURCE,
    NO_SUPPORT
};

class SelfHandler {
public:
    virtual void onSelfIdChanged(QString id) = 0;
    virtual void onSelfNameChanged(QString name) = 0;
    virtual void onSelfAvatarChanged(const std::string avatar) = 0;
    virtual void onSelfStatusChanged(IMStatus status, const std::string& msg) = 0;
};

class FriendHandler {
public:
    virtual void onFriend(const IMFriend& frnd) = 0;
    virtual void onFriendRequest(QString friendId, QString msg) = 0;
    virtual void onFriendRemoved(QString friendId) = 0;
    virtual void onFriendStatus(QString friendId, IMStatus status) = 0;
    virtual void onFriendMessage(QString friendId, IMMessage message) = 0;
    virtual void onMessageSession(QString contactId, QString sid) = 0;
    virtual void onFriendNickChanged(QString friendId, QString name) = 0;
    virtual void onFriendAvatarChanged(const QString friendId, const std::string avatar) = 0;

    virtual void onFriendAliasChanged(const IMContactId& fId, const QString& alias) = 0;

    virtual void onFriendChatState(QString friendId, int state) = 0;
    virtual void onMessageReceipt(QString friendId, QString receipt) = 0;
};

class GroupHandler {
public:
    virtual void onGroup(const QString groupId, const QString name) = 0;

    virtual void onGroupInvite(const QString groupId,  //
                               const QString peerId,   //
                               const QString message) = 0;

    virtual void onGroupSubjectChanged(const QString& groupId, const QString& subject) = 0;

    virtual void onGroupMessage(const QString groupId,  //
                                const IMPeerId peerId,  //
                                const IMMessage message) = 0;

    virtual void onGroupOccupants(const QString groupId, uint size) = 0;

    virtual void onGroupInfo(QString groupId, IMGroup groupInfo) = 0;

    virtual void onGroupOccupantStatus(const QString groupId,  //
                                       IMGroupOccupant) = 0;
};


/**
 * OkIM模块对外接口
 */
class Messenger : public QObject {
    Q_OBJECT
public:
    explicit Messenger(const QString& host,
                       const QString& name,
                       const QString& password,
                       QObject* parent = nullptr);
    ~Messenger() override;

    void start();
    void stop();

    void send(const QString& xml);

    IM* im() const {
        assert(_im);
        return _im;
    }

    IMCall* imCall()const{
        assert(_imCall);
        return _imCall;
    }

    IMPeerId getSelfId() const;
    QString getSelfUsername() const;
    QString getSelfNick() const;
    IMStatus getSelfStatus() const;

    void addSelfHandler(SelfHandler*);
    void addGroupHandler(GroupHandler*);

    bool sendToGroup(const QString& g, const QString& msg, const QString& id);

    void receiptReceived(const QString& f, QString receipt);

    QString genUniqueId();

    /** self */
    void setSelfNickname(const QString& nickname);
    void changePassword(const QString& password);
    void setSelfAvatar(const QByteArray& avatar);
    // void setMute(bool mute);

    /**
     * IMFriend (audio/video)
     */
    void addFriendHandler(FriendHandler*);

    size_t getFriendCount();

    void getFriendList(std::list<lib::messenger::IMFriend>&);

    void setFriendAlias(const QString& f, const QString& alias);

    // 添加好友
    void sendFriendRequest(const QString& username, const QString& nick, const QString& message);
    // 接受朋友邀请
    void acceptFriendRequest(const QString& f);
    // 拒绝朋友邀请
    void rejectFriendRequest(const QString& f);

    void getFriendVCard(const QString& f);

    IMStatus getFriendStatus(const QString& f);

    bool sendToFriend(const QString& f,
                      const QString& msg,
                      const QString& id,
                      bool encrypt = false);
    bool removeFriend(const QString& f);

    /**
     * Group
     */
    void loadGroupList();
    bool initRoom();

    QString createGroup(const QString& group, const QString& name);
    void joinGroup(const QString& group);
    void setRoomName(const QString& group, const QString& nick);
    void setRoomDesc(const QString& group, const QString& desc);
    void setRoomSubject(const QString& group, const QString& subject);
    void setRoomAlias(const QString& group, const QString& alias);
    bool inviteGroup(const IMContactId& group, const IMContactId& f);
    bool leaveGroup(const QString& group);
    bool destroyGroup(const QString& group);

    void sendChatState(const QString& friendId, int state);

    void requestBookmarks();

private:
    bool connectIM();

    IM* _im;
    IMJingle* jingle;
    IMCall* _imCall;

    // key: sId value:Jingle
    //  QMap<QString, lib::messenger::IMJingle*> jingleMap;
    std::unique_ptr<lib::messenger::IMConference> _conference;

    std::vector<FriendHandler*> friendHandlers;
    std::vector<SelfHandler*> selfHandlers;
    std::vector<GroupHandler*> groupHandlers;

    size_t sentCount = 0;
    std::unique_ptr<base::DelayedCallTimer> _delayer;

signals:
    void started();
    void stopped();
    void connected();
    void disconnect();
    void incoming(const QString dom);
    void receivedGroupMessage(lib::messenger::IMMessage imMsg);  //
    void messageSent(const IMMessage& message);                  //

private slots:
    void onConnectResult(lib::messenger::IMConnectStatus);
    void onStarted();
    void onStopped();
    void onReceiveGroupMessage(lib::messenger::IMMessage imMsg);
    void onDisconnect();
    void onEncryptedMessage(QString dom);
    void onGroupReceived(QString groupId, QString name);
};

class IMCall;

enum class CallState {

    /**
     * The empty bit mask. None of the bits specified below are set.
     */
    NONE = 0,

    /**
     * Set by the AV core if an error occurred on the remote end or if friend
     * timed out. This is the final state after which no more state
     * transitions can occur for the call. This call state will never be triggered
     * in combination with other call states.
     */
    ERROR0 = 1,

    /**
     * The call has finished. This is the final state after which no more state
     * transitions can occur for the call. This call state will never be
     * triggered in combination with other call states.
     */
    FINISHED = 2,

    /**
     * The flag that marks that friend is sending audio.
     */
    SENDING_A = 4,

    /**
     * The flag that marks that friend is sending video.
     */
    SENDING_V = 8,

    /**
     * The flag that marks that friend is receiving audio.
     */
    ACCEPTING_A = 16,

    /**
     * The flag that marks that friend is receiving video.
     */
    ACCEPTING_V = 32,
};

class CallHandler {
public:
    virtual void onCall(const IMPeerId& peerId,  //
                        const QString& callId,   //
                        bool audio, bool video) = 0;

    virtual void onCallRetract(const QString& friendId,  //
                               int state) = 0;

    virtual void onCallAcceptByOther(const QString& callId, const IMPeerId& peerId) = 0;

    virtual void receiveCallStateAccepted(IMPeerId friendId,  //
                                          QString callId,     //
                                          bool video) = 0;

    virtual void receiveCallStateRejected(IMPeerId friendId,  //
                                          QString callId,     //
                                          bool video) = 0;

    virtual void onHangup(const QString& friendId,  //
                          CallState state) = 0;

    virtual void onSelfVideoFrame(uint16_t w, uint16_t h,  //
                                  const uint8_t* y,        //
                                  const uint8_t* u,        //
                                  const uint8_t* v,        //
                                  int32_t ystride,         //
                                  int32_t ustride,         //
                                  int32_t vstride) = 0;

    virtual void onFriendVideoFrame(const QString& friendId,  //
                                    uint16_t w, uint16_t h,   //
                                    const uint8_t* y,         //
                                    const uint8_t* u,         //
                                    const uint8_t* v,         //
                                    int32_t ystride,          //
                                    int32_t ustride,          //
                                    int32_t vstride) = 0;
};

class MessengerCall : public QObject{
    Q_OBJECT
public:
    MessengerCall(Messenger *messenger, QObject* parent = nullptr);
    void addCallHandler(CallHandler*);

    // 发起呼叫邀请
    bool callToFriend(const QString& f, const QString& sId, bool video);
    // 创建呼叫
    bool callToPeerId(const IMPeerId& to, const QString& sId, bool video);
    // 应答呼叫
    bool callAnswerToFriend(const IMPeerId& peer, const QString& callId, bool video);
    // 取消呼叫
    void callRetract(const IMContactId& f, const QString& sId);
    // 拒绝呼叫
    void callReject(const IMPeerId& f, const QString& sId);

    // 静音功能
    void setMute(bool mute);
    void setRemoteMute(bool mute);

private:
    IMCall* call;

    signals:
        void receiveSelfVideoFrame(uint16_t w, uint16_t h,  //
                                   const uint8_t* y,        //
                                   const uint8_t* u,        //
                                   const uint8_t* v,        //
                                   int32_t ystride,         //
                                   int32_t ustride,         //
                                   int32_t vstride);

        void receiveFriendVideoFrame(const QString& friendId,  //
                                     uint16_t w, uint16_t h,   //
                                     const uint8_t* y,         //
                                     const uint8_t* u,         //
                                     const uint8_t* v,         //
                                     int32_t ystride,          //
                                     int32_t ustride,          //
                                     int32_t vstride);
};

// 不要修改顺序和值
enum class FileStatus {
    INITIALIZING = 0,
    PAUSED = 1,
    TRANSMITTING = 2,
    BROKEN = 3,
    CANCELED = 4,
    FINISHED = 5,
};

// 不要修改顺序和值
enum class FileDirection {
    SENDING = 0,
    RECEIVING = 1,
};

enum class FileControl { RESUME, PAUSE, CANCEL };

struct FileTxIBB {
    QString sid;
    int blockSize;
};

struct File {
public:
    // id(file id = ibb id) 和 sId(session id)
    QString id;
    QString sId;
    QString name;
    QString path;
    quint64 size;
    FileStatus status;
    FileDirection direction;
    FileTxIBB txIbb;
    [[__nodiscard__]] QString toString() const;
    friend QDebug& operator<<(QDebug& debug, const File& f);
};

class FileHandler {
public:
    virtual void onFileRequest(const QString& friendId, const File& file) = 0;
    virtual void onFileRecvChunk(const QString& friendId, const QString& fileId, int seq,
                                 const std::string& chunk) = 0;
    virtual void onFileRecvFinished(const QString& friendId, const QString& fileId) = 0;
    virtual void onFileSendInfo(const QString& friendId, const File& file, int m_seq,
                                int m_sentBytes, bool end) = 0;
    virtual void onFileSendAbort(const QString& friendId, const File& file, int m_sentBytes) = 0;
    virtual void onFileSendError(const QString& friendId, const File& file, int m_sentBytes) = 0;
};

class MessengerFile : public QObject{
    Q_OBJECT
public:
    MessengerFile(Messenger *messenger, QObject* parent = nullptr);

    void addFileHandler(FileHandler*);

    /**
     * File
     */
    void fileRejectRequest(QString friendId, const File& file);
    void fileAcceptRequest(QString friendId, const File& file);
    void fileFinishRequest(QString friendId, const QString& sId);
    void fileFinishTransfer(QString friendId, const QString& sId);
    void fileCancel(QString fileId);
    bool fileSendToFriend(const QString& f, const File& file);

//    /**
//     * 启动文件发送任务
//     * @param session
//     * @param file
//     */
//    void doStartFileSendTask(const Jingle::Session* session, const File& file);
//
//    /**
//     * 停止文件发送任务
//     * @param session
//     * @param file
//     */
//    void doStopFileSendTask(const Jingle::Session* session, const File& file);

private:
    IMFile *fileSender;

};


}  // namespace messenger
}  // namespace lib
