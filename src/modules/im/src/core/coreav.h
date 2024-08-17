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

#ifndef COREAV_H
#define COREAV_H

#include <QMutex>
#include <QObject>
#include <QReadWriteLock>
#include <atomic>
#include <memory>

#include "base/compatiblerecursivemutex.h"
#include "src/core/toxcall.h"

class Friend;
class Group;
class IAudioControl;
class QThread;
class QTimer;
class CoreVideoSource;
class VideoSource;
class VideoFrame;
class Core;
struct vpx_image;

class CoreAV : public QObject, public lib::messenger::CallHandler {
    Q_OBJECT
public:
    using CoreAVPtr = std::unique_ptr<CoreAV>;

    static CoreAVPtr makeCoreAV(Core* core);
    static CoreAV* getInstance();

    void setAudio(IAudioControl& newAudio);
    IAudioControl* getAudio();

    ~CoreAV();
    void start();

    bool isCallStarted(const ContactId* f) const;
    bool isCallActive(const ContactId* f) const;
    bool isCallVideoEnabled(const ContactId* f) const;
    bool sendCallAudio(QString friendId, const int16_t* pcm, size_t samples, uint8_t chans,
                       uint32_t rate) const;
    void sendCallVideo(QString friendId, std::shared_ptr<VideoFrame> frame);
    bool sendGroupCallAudio(QString groupNum, const int16_t* pcm, size_t samples, uint8_t chans,
                            uint32_t rate) const;

    CoreVideoSource* getSelfVideoSource() { return selfVideoSource.get(); }

    VideoSource* getVideoSourceFromCall(QString callNumber) const;
    void sendNoVideo();

    void joinGroupCall(const Group& group);
    void leaveGroupCall(QString groupNum);
    void muteCallInput(const ContactId* g, bool mute);
    void muteCallOutput(const ContactId* g, bool mute);
    bool isGroupCallInputMuted(const Group* g) const;
    bool isGroupCallOutputMuted(const Group* g) const;

    bool isCallInputMuted(const ContactId* f) const;
    bool isCallOutputMuted(const ContactId* f) const;
    void toggleMuteCallInput(const ContactId* f);
    void toggleMuteCallOutput(const ContactId* f);
    static void groupCallCallback(void* tox, QString group, QString peer, const int16_t* data,
                                  unsigned samples, uint8_t channels, uint32_t sample_rate,
                                  void* core);
    void invalidateGroupCallPeerSource(QString group, FriendId peerPk);

public slots:
    bool startCall(QString friendId, bool video);
    bool answerCall(ToxPeer peerId, bool video);
    bool cancelCall(QString friendId);
    void rejectCall(const ToxPeer& peerId);
    void timeoutCall(QString friendId);

signals:
    void avInvite(ToxPeer peerId, bool video);
    void avStart(FriendId friendId, bool video);
    void avEnd(FriendId friendId, bool error = false);
    void createCallToPeerId(lib::messenger::IMPeerId friendId, QString callId, bool video);

private slots:
    void doCreateCallToPeerId(lib::messenger::IMPeerId friendId, QString callId, bool video);

    void stateCallback(QString friendId, lib::messenger::CallState state);

    void bitrateCallback(QString friendId, uint32_t arate, uint32_t vrate, void* self);
    void audioBitrateCallback(QString friendId, uint32_t rate, void* self);
    void videoBitrateCallback(QString friendId, uint32_t rate, void* self);
    void onFriendVideoFrame(const QString& friendId,  //
                            uint16_t w, uint16_t h,   //
                            const uint8_t* y,         //
                            const uint8_t* u,         //
                            const uint8_t* v,         //
                            int32_t ystride,          //
                            int32_t ustride,          //
                            int32_t vstride) override;

    void onSelfVideoFrame(uint16_t w, uint16_t h,  //
                          const uint8_t* y,        //
                          const uint8_t* u,        //
                          const uint8_t* v,        //
                          int32_t ystride,         //
                          int32_t ustride,         //
                          int32_t vstride) override;

private:
    CoreAV(Core* core);

    void process();

    void audioFrameCallback(QString friendId, const int16_t* pcm, size_t sampleCount,
                            uint8_t channels, uint32_t samplingRate, void* self);

    //  void videoFrameCallback(ToxAV *toxAV, QString friendId, uint16_t w,
    //                          uint16_t h, const uint8_t *y, const uint8_t *u,
    //                          const uint8_t *v, int32_t ystride, int32_t ustride,
    //                          int32_t vstride, void *self);

    vpx_image makeVpxFrame(uint16_t w, uint16_t h, const uint8_t* y, const uint8_t* u,
                           const uint8_t* v, int32_t ystride, int32_t ustride, int32_t vstride);

    void videoFramePush(CoreVideoSource* vs, const vpx_image& frame);

    void onCall(const lib::messenger::IMPeerId& peerId, const QString& callId, bool audio,
                bool video) override;

    void onCallRetract(const QString& friendId, lib::messenger::CallState state) override;

    void onCallAcceptByOther(const QString& callId,
                             const lib::messenger::IMPeerId& peerId) override;

    void receiveCallStateAccepted(lib::messenger::IMPeerId friendId, QString callId,
                                  bool video) override;

    void receiveCallStateRejected(lib::messenger::IMPeerId friendId, QString callId,
                                  bool video) override;

    void onHangup(const QString& friendId, lib::messenger::CallState state) override;

private:
    static constexpr uint32_t VIDEO_DEFAULT_BITRATE = 2500;

private:
    //  std::unique_ptr<CoreVideoSource> selfVideoSource;
    // atomic because potentially accessed by different threads
    Core* core;
    std::atomic<IAudioControl*> audioCtrl;
    lib::messenger::MessengerCall* imCall;
    std::unique_ptr<QThread> coreavThread;
    QTimer* iterateTimer = nullptr;

    std::unique_ptr<CoreVideoSource> selfVideoSource;

    /**
     * @brief Maps friend IDs to ToxFriendCall.
     * @note Need to use STL container here, because Qt containers need a copy
     * constructor.
     */
    using ToxFriendCallPtr = std::unique_ptr<ToxFriendCall>;
    std::map<QString, ToxFriendCallPtr> calls;

    using ToxGroupCallPtr = std::unique_ptr<ToxGroupCall>;
    /**
     * @brief Maps group IDs to ToxGroupCalls.
     * @note Need to use STL container here, because Qt containers need a copy
     * constructor.
     */
    std::map<QString, ToxGroupCallPtr> groupCalls;

    // protect 'calls' and 'groupCalls'
    mutable QReadWriteLock callsLock{QReadWriteLock::Recursive};
};

#endif  // COREAV_H
