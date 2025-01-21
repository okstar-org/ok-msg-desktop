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

#include <cassert>
#include <cstddef>
#include <string>
#include "IMFriend.h"
#include "IMGroup.h"
#include "IMMessage.h"
#include "base/jid.h"
#include "lib/ortc/ok_rtc.h"

class QDomElement;
class QDomDocument;

namespace lib::session {
class AuthSession;
}

namespace lib::messenger {
/**
 * 聊天
 */
class IM;
/**
 * 会话
 */
class IMJingle;
/**
 * 文件传输
 */
class IMFile;
/**
 * 音视频
 */
class IMCall;
/**
 *  会议
 */
class IMMeet;

class IMHandler{
public:
    virtual void onConnecting()= 0;
    virtual void onConnected()= 0;
    virtual void onDisconnected(int)= 0;
    virtual void onStarted()= 0;
    virtual void onStopped()= 0;
};

class SelfHandler {
public:
    virtual void onSelfIdChanged(const std::string& id) = 0;
    virtual void onSelfNameChanged(const std::string& name) = 0;
    virtual void onSelfAvatarChanged(const std::string& avatar) = 0;
    virtual void onSelfStatusChanged(IMStatus status, const std::string& msg) = 0;
    virtual void onSelfVCardChanged(IMVCard& imvCard) = 0;
};

class FriendHandler {
public:
    virtual void onFriend(const IMFriend& frnd) = 0;
    virtual void onFriendRequest(const std::string& friendId, const std::string& msg) = 0;
    virtual void onFriendRemoved(const std::string& friendId) = 0;
    virtual void onFriendStatus(const std::string& friendId, IMStatus status) = 0;
    virtual void onFriendMessage(const std::string& friendId, const IMMessage& message) = 0;
    virtual void onFriendMessageReceipt(const std::string& friendId, const std::string& msgId) = 0;
    virtual void onMessageSession(const std::string& contactId, const std::string& sid) = 0;
    virtual void onFriendNickChanged(const std::string& friendId, const std::string& name) = 0;
    virtual void onFriendAvatarChanged(const std::string& friendId, const std::string& avatar) = 0;

    virtual void onFriendAliasChanged(const IMContactId& fId, const std::string& alias) = 0;
    virtual void onFriendVCard(const IMContactId& fId, const IMVCard& imvCard) = 0;

    virtual void onFriendChatState(const std::string& friendId, int state) = 0;
    virtual void onMessageReceipt(const std::string& friendId, const std::string& receipt) = 0;
};

class GroupHandler {
public:
    virtual void onGroup(const std::string& groupId, const std::string& name) = 0;

    virtual void onGroupInvite(const std::string& groupId,  //
                               const std::string& peerId,   //
                               const std::string& message) = 0;

    virtual void onGroupSubjectChanged(const std::string& groupId, const std::string& subject) = 0;

    virtual void onGroupMessage(const std::string& groupId,  //
                                const IMPeerId& peerId,      //
                                const IMMessage& message) = 0;

    virtual void onGroupOccupants(const std::string& groupId, uint size) = 0;

    virtual void onGroupInfo(const std::string& groupId, const IMGroup& groupInfo) = 0;

    virtual void onGroupOccupantStatus(const std::string& groupId,  //
                                       const IMGroupOccupant&) = 0;
};

/**
 * OkIM模块对外接口
 */
class Messenger {
public:
    explicit Messenger(const std::string& host,
                       const std::string& name,
                       const std::string& password);
    ~Messenger();

    void start();
    [[nodiscard]] bool isStarted() const;
    void stop();
    void doConnect();

    void send(const std::string& xml);

    [[nodiscard]] IM* im() const {
        assert(_im);
        return _im;
    }

    IMContactId getSelfId() const;
    IMPeerId getSelfPeerId() const;
    std::string getSelfUsername() const;
    std::string getSelfNick() const;
    IMStatus getSelfStatus() const;

    void addIMHandler(IMHandler*);
    void addSelfHandler(SelfHandler*);
    void addGroupHandler(GroupHandler*);

    bool sendToGroup(const std::string& g, const std::string& msg, const std::string& id);

    void receiptReceived(const std::string& f, std::string receipt);

    std::string genUniqueId();

    /** self */
    void setSelfNickname(const std::string& nickname);
    void changePassword(const std::string& password);
    void setSelfAvatar(const std::string& avatar);

    /**
     * IMFriend (audio/video)
     */
    void addFriendHandler(FriendHandler*);

    size_t getFriendCount();

    void getFriendList(std::list<lib::messenger::IMFriend>&);

    void setFriendAlias(const std::string& f, const std::string& alias);

    // 添加好友
    void sendFriendRequest(const std::string& username, const std::string& nick,
                           const std::string& message);
    // 接受朋友邀请
    void acceptFriendRequest(const std::string& f);
    // 拒绝朋友邀请
    void rejectFriendRequest(const std::string& f);

    void getFriendVCard(const std::string& f);

    IMStatus getFriendStatus(const std::string& f);

    bool sendToFriend(const std::string& f,
                      const std::string& msg,
                      const std::string& id,
                      bool encrypt = false);
    bool removeFriend(const std::string& f);

    /**
     * Group
     */
    void loadGroupList();

    std::string createGroup(const std::string& group, const std::string& name);
    void joinGroup(const std::string& group);
    void setRoomName(const std::string& group, const std::string& nick);
    void setRoomDesc(const std::string& group, const std::string& desc);
    void setRoomSubject(const std::string& group, const std::string& subject);
    void setRoomAlias(const std::string& group, const std::string& alias);
    bool inviteGroup(const IMContactId& group, const IMContactId& f);
    bool leaveGroup(const std::string& group);
    bool destroyGroup(const std::string& group);

    void sendChatState(const std::string& friendId, int state);

    void requestBookmarks();

private:

    IM* _im;
    IMJingle* jingle;



    size_t sentCount = 0;

    // signals:
    //     void incoming(const std::string dom);
    //     void messageSent(const IMMessage& message);                  //

    // private slots:
    //    void onEncryptedMessage(std::string dom);
};

class IMCall;

enum class CallState {

    NONE = 0,

    ERROR0 = 1,

    FINISHED = 2,

    SENDING_A = 4,

    SENDING_V = 8,

    ACCEPTING_A = 16,

    ACCEPTING_V = 32,
};

enum class CallDirection { CallIn, CallOut };

enum class CallFSM {
    None,          // 新建
    Creating,      // 呼叫进行中
    Created,       // 对方接收
    Connected,     // 已连接
    Disconnected,  // 已断开
    Destroyed,     // 已销毁
};

class CallHandler {
public:
    // 来电
    virtual void onCall(const IMPeerId& peerId,  //
                        const std::string& callId,  //
                        bool audio, bool video) = 0;
    // 呼叫建立中
    virtual void onCallCreating(const IMPeerId& peerId,     //
                                const std::string& callId,  //
                                bool video) = 0;
    // 呼叫已建立
    virtual void onCallCreated(const IMPeerId& peerId,  //
                               const std::string& callId) = 0;
    // 呼叫撤回
    virtual void onCallRetract(const IMPeerId& peerId,  //
                               CallState state) = 0;

    virtual void onCallAcceptByOther(const IMPeerId& peerId, const std::string& callId) = 0;

    virtual void onPeerConnectionChange(const IMPeerId& friendId,  //
                                        const std::string& callId,  //
                                        ortc::PeerConnectionState state) = 0;

    virtual void onIceGatheringChange(const IMPeerId& friendId,  //
                                      const std::string& callId,  //
                                      ortc::IceGatheringState state) = 0;

    virtual void onIceConnectionChange(const lib::messenger::IMPeerId& peerId,
                                       const std::string& callId,
                                       lib::ortc::IceConnectionState state) = 0;

    virtual void receiveCallStateAccepted(const IMPeerId& friendId,  //
                                          const std::string& callId,  //
                                          bool video) = 0;

    virtual void receiveCallStateRejected(const IMPeerId& friendId,  //
                                          const std::string& callId,  //
                                          bool video) = 0;

    // 对方挂断
    virtual void onHangup(const IMPeerId& peerId,  //
                          CallState state) = 0;

    // 呼叫终止
    virtual void onEnd(const IMPeerId& peerId) = 0;

    virtual void onSelfVideoFrame(uint16_t w, uint16_t h,  //
                                  const uint8_t* y,        //
                                  const uint8_t* u,        //
                                  const uint8_t* v,        //
                                  int32_t ystride,         //
                                  int32_t ustride,         //
                                  int32_t vstride) = 0;

    virtual void onFriendVideoFrame(const std::string& friendId,  //
                                    uint16_t w, uint16_t h,   //
                                    const uint8_t* y,         //
                                    const uint8_t* u,         //
                                    const uint8_t* v,         //
                                    int32_t ystride,          //
                                    int32_t ustride,          //
                                    int32_t vstride) = 0;
};

class MessengerCall {
public:
    explicit MessengerCall(Messenger* messenger);
    ~MessengerCall();

    void addCallHandler(CallHandler*);

    // 发起呼叫邀请
    bool callToFriend(const std::string& f, const std::string& sId, bool video);
    // 创建呼叫
    bool callToPeerId(const IMPeerId& to, const std::string& sId, bool video);
    // 应答呼叫
    bool callAnswerToFriend(const IMPeerId& peer, const std::string& callId, bool video);
    // 取消呼叫
    void callCancel(const IMContactId& f, const std::string& sId);
    // 拒绝呼叫
    void callReject(const IMPeerId& f, const std::string& sId);

    // 静音功能
    void setCtrlState(ortc::CtrlState state);
    void setSpeakerVolume(uint32_t vol);

private:
    IMCall* call;
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
    std::string sid;
    int blockSize;
};

class FileHandler;

struct File {
public:
    // id = fileId & ibbId & msgId
    std::string id;
    // sId: session id
    std::string sId;
    std::string name;
    std::string path;
    uint32_t size;
    FileStatus status;
    FileDirection direction;
    FileTxIBB txIbb;
    std::vector<FileHandler*> handlers;
};

class FileHandler {
public:
    virtual void onFileRequest(const std::string& sId, const std::string& friendId,
                               const File& file) = 0;

    virtual void onFileRecvChunk(const std::string& sId, const std::string& friendId,
                                 const std::string& fileId, int seq, const std::string& chunk) = 0;

    virtual void onFileRecvFinished(const std::string& sId, const std::string& friendId,
                                    const std::string& fileId) = 0;

    // virtual void onFileSendInfo(const std::string& friendId, const File& file, int m_seq,
    // int m_sentBytes, bool end) = 0;

    virtual void onFileSendAbort(const std::string& sId, const std::string& friendId,
                                 const File& file, int m_sentBytes) = 0;

    virtual void onFileSendError(const std::string& sId, const std::string& friendId,
                                 const File& file, int m_sentBytes) = 0;

    virtual void onFileStreamOpened(const std::string& sId, const std::string& friendId,
                                    const File& file) = 0;

    virtual void onFileStreamClosed(const std::string& sId, const std::string& friendId,
                                    const File& file) = 0;

    virtual void onFileStreamData(const std::string& sId, const std::string& friendId,
                                  const File& file, const std::string& data, int m_seq,
                                  int m_sentBytes) = 0;

    virtual void onFileStreamDataAck(const std::string& sId, const std::string& friendId,
                                     const File& file, uint32_t ack) = 0;
    virtual void onFileStreamError(const std::string& sId, const std::string& friendId,
                                   const File& file, uint32_t m_sentBytes) = 0;
};

/**
 * 文件传输
 */
class MessengerFile {
public:
    explicit MessengerFile(Messenger* messenger);
    ~MessengerFile();

    void addHandler(FileHandler* h);

    /**
     * File
     */
    void fileRejectRequest(std::string friendId, const File& file);
    void fileAcceptRequest(std::string friendId, const File& file);
    void fileFinishRequest(std::string friendId, const std::string& sId);
    void fileFinishTransfer(std::string friendId, const std::string& sId);
    void fileCancel(std::string fileId);
    bool fileSendToFriend(const std::string& friendId, const File& file);

private:
    IMFile* fileSender;
};

/**
 * 会议
 */

struct Meet {
    std::string jid;
    std::string uid;
    uint32_t startAudioMuted;
    uint32_t startVideoMuted;
    bool rtcstatsEnabled;
};

struct SourceInfo {
    // 音频禁止
    bool audioMute = false;
    // 视频禁止
    bool videoMute = false;
};

/**
 * 会议成员
 */
struct Participant {
    // 用户邮箱
    std::string email;
    // 用户昵称
    std::string nick;
    // 会议成员唯一标识
    std::string resource;
    std::string avatarUrl;
    // IM终端标识(可定位到用户和终端)
    std::string jid;
    std::string affiliation;
    std::string role;
    // 设备信息
    SourceInfo sourceInfo;
};

class MessengerMeetHandler {
public:
    virtual void onMeetCreated(const ok::base::Jid& jid,
                               bool ready,
                               const std::map<std::string, std::string>& props) = 0;

    virtual void onMeetInitiate(const IMPeerId& peerId, const ortc::OJingleContentMap& map) = 0;

    virtual void onParticipantJoined(const ok::base::Jid& jid, const Participant& participant) = 0;

    virtual void onParticipantLeft(const ok::base::Jid& jid, const std::string& participant) = 0;

    virtual void onParticipantVideoFrame(const std::string& participant,
                                         const ortc::RendererImage& image) = 0;

    virtual void onParticipantMessage(const std::string& participant, const std::string& msg) = 0;

    virtual void onEnd() = 0;

    // void iceGatheringStateChanged(IMPeerId to, const std::string sId, ortc::IceGatheringState) =
    // 0; void iceConnectionStateChanged(IMPeerId to, const std::string sId,
    // ortc::IceConnectionState) = 0;
};

class MessengerMeet : public CallHandler {
public:
    explicit MessengerMeet(Messenger* messenger);
    ~MessengerMeet();
    /**
     * 创建会议
     * @param room
     */
    void create(const std::string& room);

    /**
     * 离开会议
     */
    void leave();

    void start(const IMPeerId& peerId, const ortc::OJingleContentMap& map);

    /**
     * 获取视频设备列表
     * @return
     */
    [[maybe_unused]] std::vector<std::string> getVideoDeviceList();

    /**
     * 发送消息
     * @param msg
     */
    void sendMessage(const std::string& msg);

    /**
     是音频、视频、扬声器
     */
    void setCtrlState(ortc::CtrlState state);

    void addHandler(MessengerMeetHandler* hdr);
    void removeHandler(MessengerMeetHandler* hdr);

protected:
    void onCall(const IMPeerId& peerId,  //
                const std::string& callId,  //
                bool audio, bool video) override;

    void onCallCreating(const IMPeerId& peerId,     //
                        const std::string& callId,  //
                        bool video) override;

    void onCallCreated(const IMPeerId& peerId,  //
                       const std::string& callId) override;

    // 呼叫撤回：其它终端接收
    void onCallRetract(const IMPeerId& peerId,  //
                       CallState state) override;

    void onCallAcceptByOther(const IMPeerId& peerId,  //
                             const std::string& callId) override;

    void onPeerConnectionChange(const IMPeerId& peerId,  //
                                const std::string& callId,  //
                                ortc::PeerConnectionState state) override;

    void onIceGatheringChange(const IMPeerId& friendId,  //
                              const std::string& callId,  //
                              ortc::IceGatheringState state) override;

    void onIceConnectionChange(const lib::messenger::IMPeerId& peerId,
                               const std::string& callId,
                               lib::ortc::IceConnectionState state) override;

    void receiveCallStateAccepted(const IMPeerId& peerId,  //
                                  const std::string& callId,  //
                                  bool video) override;

    void receiveCallStateRejected(const IMPeerId& peerId,  //
                                  const std::string& callId,  //
                                  bool video) override;
    // 对方挂断
    void onHangup(const IMPeerId& peerId,  //
                  CallState state) override;

    // 结束
    void onEnd(const IMPeerId& peerId) override;

    void onSelfVideoFrame(uint16_t w, uint16_t h,  //
                          const uint8_t* y,        //
                          const uint8_t* u,        //
                          const uint8_t* v,        //
                          int32_t ystride,         //
                          int32_t ustride,         //
                          int32_t vstride) override;

    void onFriendVideoFrame(const std::string& friendId,  //
                            uint16_t w, uint16_t h,   //
                            const uint8_t* y,         //
                            const uint8_t* u,         //
                            const uint8_t* v,         //
                            int32_t ystride,          //
                            int32_t ustride,          //
                            int32_t vstride) override;

private:
    IMMeet* meet;
    MessengerCall* call;
};

}  // namespace lib::messenger
