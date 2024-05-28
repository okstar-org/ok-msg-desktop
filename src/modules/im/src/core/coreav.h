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

#include "src/core/toxcall.h"
#include "base/compatiblerecursivemutex.h"

#include <QMutex>
#include <QObject>
#include <QReadWriteLock>
#include <atomic>
#include <memory>

#include "lib/messenger/messenger.h"
#include "lib/messenger/tox/tox.h"


class Friend;
class Group;
class IAudioControl;
class QThread;
class QTimer;
class CoreVideoSource;
class CameraSource;
class VideoSource;
class VideoFrame;
class Core;
struct vpx_image;

class CoreAV : public QObject, public lib::messenger::CallHandler {
  Q_OBJECT

public:
  using CoreAVPtr = std::unique_ptr<CoreAV>;
  static CoreAVPtr makeCoreAV(Tox *core, CompatibleRecursiveMutex &coreLock);

  void setAudio(IAudioControl &newAudio);
  IAudioControl *getAudio();

  ~CoreAV();

  bool isCallStarted(const ContactId *f) const;
  bool isCallActive(const ContactId *f) const;
  bool isCallVideoEnabled(const ContactId *f) const;
  bool sendCallAudio(QString friendId, const int16_t *pcm, size_t samples,
                     uint8_t chans, uint32_t rate) const;
  void sendCallVideo(QString friendId, std::shared_ptr<VideoFrame> frame);
  bool sendGroupCallAudio(QString groupNum, const int16_t *pcm, size_t samples,
                          uint8_t chans, uint32_t rate) const;

//  CoreVideoSource *getVideoSourceFromSelf() const{
//    return selfVideoSource.get();
//  }

  VideoSource *getVideoSourceFromCall(QString callNumber) const;
  void sendNoVideo();

  void joinGroupCall(const Group &group);
  void leaveGroupCall(QString groupNum);
  void muteCallInput(const Group *g, bool mute);
  void muteCallOutput(const Group *g, bool mute);
  bool isGroupCallInputMuted(const Group *g) const;
  bool isGroupCallOutputMuted(const Group *g) const;

  bool isCallInputMuted(const ContactId *f) const;
  bool isCallOutputMuted(const ContactId *f) const;
  void toggleMuteCallInput(const ContactId *f);
  void toggleMuteCallOutput(const ContactId *f);
  static void groupCallCallback(void *tox, QString group, QString peer,
                                const int16_t *data, unsigned samples,
                                uint8_t channels, uint32_t sample_rate,
                                void *core);
  void invalidateGroupCallPeerSource(QString group, ToxPk peerPk);

public slots:
  bool startCall(QString friendId, bool video);
  bool answerCall(QString friendId, bool video);
  bool cancelCall(QString friendId);
  void timeoutCall(QString friendId);
  void start();

signals:
  void avInvite(QString friendId, bool video);
  void avStart(QString friendId, bool video);
  void avEnd(QString friendId, bool error = false);
  void createCallToPeerId(lib::messenger::IMPeerId friendId, QString callId, bool video);

private slots:
  void doCreateCallToPeerId(lib::messenger::IMPeerId friendId, QString callId, bool video);

  static void callCallback(ToxAV *toxAV, QString friendId, QString callId,
                           bool audio, bool video, void *self);
  static void stateCallback(ToxAV *, QString friendId, uint32_t state,
                            void *self);
  static void bitrateCallback(ToxAV *toxAV, QString friendId, uint32_t arate,
                              uint32_t vrate, void *self);
  static void audioBitrateCallback(ToxAV *toxAV, QString friendId,
                                   uint32_t rate, void *self);
  static void videoBitrateCallback(ToxAV *toxAV, QString friendId,
                                   uint32_t rate, void *self);
  void onFriendVideoFrame(const QString &friendId, //
                          uint16_t w, uint16_t h,  //
                          const uint8_t *y,        //
                          const uint8_t *u,        //
                          const uint8_t *v,        //
                          int32_t ystride,         //
                          int32_t ustride,         //
                          int32_t vstride) override;

  void onSelfVideoFrame(uint16_t w, uint16_t h, //
                        const uint8_t *y,       //
                        const uint8_t *u,       //
                        const uint8_t *v,       //
                        int32_t ystride,        //
                        int32_t ustride,        //
                        int32_t vstride) override;

private:
  struct ToxAVDeleter {
    void operator()(ToxAV *tox) { /* toxav_kill(tox);*/
    }
  };

  CoreAV(std::unique_ptr<ToxAV, ToxAVDeleter> tox,
         CompatibleRecursiveMutex &toxCoreLock);
  void connectCallbacks(ToxAV &toxav);

  void process();
  static void audioFrameCallback(ToxAV *toxAV, QString friendId,
                                 const int16_t *pcm, size_t sampleCount,
                                 uint8_t channels, uint32_t samplingRate,
                                 void *self);

  void videoFrameCallback(ToxAV *toxAV, QString friendId, uint16_t w,
                          uint16_t h, const uint8_t *y, const uint8_t *u,
                          const uint8_t *v, int32_t ystride, int32_t ustride,
                          int32_t vstride, void *self);

  void videoFramePush(CoreVideoSource *vs, uint16_t w, uint16_t h,
                          const uint8_t *y, const uint8_t *u, const uint8_t *v,
                          int32_t ystride, int32_t ustride, int32_t vstride);

  void onCall(const QString &friendId, const QString& callId, bool audio, bool video) override;

  void onCallRetract(const QString &friendId, int state) override;

   void onCallAcceptByOther(const QString& callId, const lib::messenger::IMPeerId & peerId) override;

  void receiveCallStateAccepted(lib::messenger::IMPeerId friendId, QString callId, bool video) override;

  void receiveCallStateRejected(lib::messenger::IMPeerId friendId, QString callId, bool video) override;

  void onHangup(const QString &friendId,
                TOXAV_FRIEND_CALL_STATE state) override;

private:
  static constexpr uint32_t VIDEO_DEFAULT_BITRATE = 2500;

private:
//  std::unique_ptr<CoreVideoSource> selfVideoSource;
  // atomic because potentially accessed by different threads
  std::atomic<IAudioControl *> audio;
  std::unique_ptr<ToxAV, ToxAVDeleter> toxav;
  std::unique_ptr<QThread> coreavThread;
  QTimer *iterateTimer = nullptr;
  using ToxFriendCallPtr = std::unique_ptr<ToxFriendCall>;
  /**
   * @brief Maps friend IDs to ToxFriendCall.
   * @note Need to use STL container here, because Qt containers need a copy
   * constructor.
   */
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

  /**
   * @brief needed to synchronize with the Core thread, some toxav_* functions
   *        must not execute at the same time as tox_iterate()
   * @note This must be a recursive mutex as we're going to lock it in callbacks
   */
  CompatibleRecursiveMutex &coreLock;
};

#endif // COREAV_H
