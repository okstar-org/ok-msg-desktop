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

#ifndef IMJINGLE_H
#define IMJINGLE_H

#include <list>
#include <QMap>
#include <map>

#include <client.h>
#include <jinglesession.h>
#include <jinglesessionhandler.h>
#include <jinglesessionmanager.h>

#include <error.h>
#include <inbandbytestream.h>
#include <messagesessionhandler.h>
#include <presencehandler.h>

#include "IMConference.h"
#include "IMJingleSession.h"
#include "lib/messenger/messenger.h"
#include "lib/session/AuthSession.h"

#include "lib/ortc/ok_rtc.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "lib/ortc/ok_rtc_manager.h"

namespace gloox {
namespace Jingle {
class JingleMessage;
}
}
namespace lib {
namespace messenger {

class IMFileTask;
enum class CallDirection;


class IMJingle : public QObject,
                 public MessageHandler,
                 public gloox::IqHandler,
                 public gloox::MessageSessionHandler,
                 public gloox::Jingle::SessionHandler,
                 public lib::ortc::OkRTCHandler,
                 public lib::ortc::OkRTCRenderer {
  Q_OBJECT

public:
  IMJingle(IM *im, std::vector<FileHandler *> *fileHandlers, QObject *parent = nullptr);

  ~IMJingle() override;

   virtual void handleMessageSession( MessageSession* session ) override;
   virtual void handleMessage( const Message& msg, MessageSession* session = 0 ) override;

  /**
   * 发起呼叫
   * @param friendId
   * @param video
   * @return
   */
  bool startCall(const QString &friendId, const QString &sId, bool video);

  bool sendCallToResource(const QString &friendId, const QString &sId, bool video);

  bool createCall(const IMPeerId &to, const QString &sId, bool video);

  bool answer(const QString &friendId, const QString &callId, bool video);
  void cancel(const QString &friendId);
  void cancelCall(const QString &friendId, const QString &sId);
  void join(const JID &room);
  void setMute(bool mute);
  void setRemoteMute(bool mute);

  /**
   * jingle-message
   */
  //处理JingleMessage消息
  void doJingleMessage(const IMPeerId &peerId, const gloox::Jingle::JingleMessage *jm);

  //发起呼叫邀请
  void proposeJingleMessage(const QString &friendId, const QString &callId, bool video);

  void rejectJingleMessage(const QString &friendId, const QString &callId);

  void acceptJingleMessage(const QString &friendId, const QString &callId);

  void retractJingleMessage(const QString &friendId, const QString &callId);



  /**
   * File
   */
  void rejectFileRequest(const QString &friendId,
                         const QString &sId);
  void acceptFileRequest(const QString &friendId,
                         const File &file);
  void finishFileRequest(const QString &friendId,
                         const QString &sId);
  void finishFileTransfer(const QString &friendId,
                          const QString &sId);

  bool sendFile(const QString &friendId, const File &file);
  bool sendFileToResource(const JID &friendId, const File &file);


protected:

  /**
   * iq handlers
   * @param iq
   * @return
   */
  bool handleIq(const IQ &iq) override;

  void handleIqID(const IQ &iq, int context) override;


  void handleSessionAction(Jingle::Action action, Jingle::Session *session,
                           const Jingle::Session::Jingle *jingle) override;

  void handleSessionActionError(Jingle::Action action, Jingle::Session *session,
                                const gloox::Error *error) override;

  void handleIncomingSession(Jingle::Session *session) override;

  /**
   * OkRTCHandler
   * @param oContext
   */

  void onCreatePeerConnection(const std::string &peerId, const std::string &sId,
                              bool ok) override;

  // onRTP
  void onRTP(const std::string &sId,      //
             const std::string &friendId, //
             const lib::ortc::JingleContext &oContext) override;

  // onIce
  void onIce(const std::string &sId,      //
             const std::string &friendId, //
             const lib::ortc::OIceUdp &) override;

  // Renderer
  void onRender(const std::string &peerId,
                lib::ortc::RendererImage image) override;

  IMJingleSession *findSession(const QString &sId);

  IMJingleSession* createSession(const IMPeerId &to, const QString &sId);

private:

  IM *im;

  std::vector<FileHandler *> *fileHandlers;

  // receiver -> sid
  QMap<IMPeerId, QString> m_friendSessionMap;
  //  std::map<IMPeerId, const Jingle::Session::Jingle *> m_jingleMap;
  // sid -> JingleContext
  //  std::map<std::string, lib::ortc::JingleContext> m_contextMap;
  // sid -> session
  QMap<QString, IMJingleSession *> m_sessionMap;

//  std::unique_ptr<lib::ortc::OkRTCManager> _rtcManager;
  std::unique_ptr<Jingle::SessionManager> _sessionManager;




  QList<Jingle::Content *> m_ices;

  QString getSessionByFriendId(const QString &friendId);

  IMJingleSession* cacheSessionInfo(Jingle::Session *session, lib::ortc::JingleCallType callType);

  void clearSessionInfo(Jingle::Session *session);

  void doSessionInitiate(Jingle::Session *session,
                         const Jingle::Session::Jingle *, const IMPeerId &);

  void doSessionTerminate(Jingle::Session *session,
                          const Jingle::Session::Jingle *, const IMPeerId &);

  void doSessionAccept(Jingle::Session *session,        //
                       const Jingle::Session::Jingle *, //
                       const IMPeerId &);
  void doSessionInfo(const Jingle::Session::Jingle *, const IMPeerId &);
  void doContentAdd(const Jingle::Session::Jingle *, const IMPeerId &);
  void doContentRemove(const Jingle::Session::Jingle *, const IMPeerId &);
  void doContentModify(const Jingle::Session::Jingle *, const IMPeerId &);
  void doContentAccept(const Jingle::Session::Jingle *, const IMPeerId &);
  void doContentReject(const Jingle::Session::Jingle *, const IMPeerId &);
  void doTransportAccept(const Jingle::Session::Jingle *, const IMPeerId &);
  void doTransportInfo(const Jingle::Session::Jingle *, const IMPeerId &);
  void doTransportReject(const Jingle::Session::Jingle *, const IMPeerId &);
  void doTransportReplace(const Jingle::Session::Jingle *, const IMPeerId &);
  void doSecurityInfo(const Jingle::Session::Jingle *, const IMPeerId &);
  void doDescriptionInfo(const Jingle::Session::Jingle *, const IMPeerId &);
  void doInvalidAction(const Jingle::Session::Jingle *, const IMPeerId &);



signals:
  /**
   * Call events
   * @param friendId
   * @param audio
   * @param video
   */
  void callStarted();
  // 呼叫请求
  void receiveCallRequest(QString friendId, QString callId, bool audio, bool video);
  // 呼叫撤回
  void receiveCallRetract(QString friendId, int state);
  void receiveCallAcceptByOther(QString callId, IMPeerId peerId);
  void receiveFriendHangup(QString friendId, int state);

  // 对方状态变化
  void receiveCallStateAccepted(IMPeerId peerId, QString callId, bool video);
  void receiveCallStateRejected(IMPeerId peerId, QString callId, bool video);


  void receiveFriendCall(QString friendId, QString callId, bool audio,
                         bool video);


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

  void receiveFileRequest(const QString &friendId,
                          const File &file);



  void receiveFileChunk(const IMContactId friendId, QString sId,
                        int seq, const std::string chunk);
  void receiveFileFinished(const IMContactId friendId, QString sId);

};

} // namespace IM
} // namespace lib
#endif // IMJINGLE_H
