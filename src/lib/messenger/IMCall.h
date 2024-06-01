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

//
// Created by gaojie on 24-5-29.
//

#ifndef OKMSG_PROJECT_IMCALL_H
#define OKMSG_PROJECT_IMCALL_H

#include "IMFriend.h"

#include <QObject>
#include <QString>

#include "tox/toxav.h"




enum TOXAV_FRIEND_CALL_STATE {

  /**
   * The empty bit mask. None of the bits specified below are set.
   */
  TOXAV_FRIEND_CALL_STATE_NONE = 0,

  /**
   * Set by the AV core if an error occurred on the remote end or if friend
   * timed out. This is the final state after which no more state
   * transitions can occur for the call. This call state will never be triggered
   * in combination with other call states.
   */
  TOXAV_FRIEND_CALL_STATE_ERROR = 1,

  /**
   * The call has finished. This is the final state after which no more state
   * transitions can occur for the call. This call state will never be
   * triggered in combination with other call states.
   */
  TOXAV_FRIEND_CALL_STATE_FINISHED = 2,

  /**
   * The flag that marks that friend is sending audio.
   */
  TOXAV_FRIEND_CALL_STATE_SENDING_A = 4,

  /**
   * The flag that marks that friend is sending video.
   */
  TOXAV_FRIEND_CALL_STATE_SENDING_V = 8,

  /**
   * The flag that marks that friend is receiving audio.
   */
  TOXAV_FRIEND_CALL_STATE_ACCEPTING_A = 16,

  /**
   * The flag that marks that friend is receiving video.
   */
  TOXAV_FRIEND_CALL_STATE_ACCEPTING_V = 32,

};

namespace ok::session {
class AuthSession;
}

namespace lib::ortc {
class OkRTCManager;
}

namespace lib::messenger {

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

class Messenger;
class IMJingle;
enum class CallDirection;

struct IMCall0 {
    QString id;
    QString sId;
    CallDirection direction;
};

class IMCall : public QObject {
    Q_OBJECT
public:
    IMCall ( QObject *parent = nullptr);
    void addCallHandler(CallHandler *);
    bool callToGroup(const QString &g);

    // 发起呼叫邀请
    bool callToFriend(const QString &f, const QString &sId, bool video);
    // 创建呼叫
    bool callToPeerId(const IMPeerId &to, const QString &sId, bool video);
    // 应答呼叫
    bool callAnswerToFriend(const QString &f, const QString &callId, bool video);
    // 取消呼叫
    bool callCancelToFriend(const QString &f, const QString &sId);

    // 静音功能
    void setMute(bool mute);
    void setRemoteMute(bool mute);


signals:
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

private:
     void connectJingle(IMJingle *jingle);
     IMJingle *jingle;
     ok::session::AuthSession *session;
     std::vector<CallHandler *> callHandlers;
     lib::ortc::OkRTCManager *rtcManager;
};

} // namespace lib::messenger
using ToxAV = lib::messenger::IMCall;
#endif // OKMSG_PROJECT_IMCALL_H
