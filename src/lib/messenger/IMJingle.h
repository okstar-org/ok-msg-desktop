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

#include "gloox/client.h"
#include "gloox/jinglesession.h"
#include "gloox/jinglesessionhandler.h"
#include "gloox/jinglesessionmanager.h"

#include "gloox/error.h"
#include "gloox/inbandbytestream.h"
#include "gloox/presencehandler.h"

#include "IM.h"
#include "IMConference.h"
#include "IMFile.h"
#include "IMJingleSession.h"
#include "lib/messenger/messenger.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "lib/ortc/ok_rtc_manager.h"
#include "lib/ortc/ok_rtc_proxy.h"
#include "lib/session/AuthSession.h"
#include "lib/messenger/tox/tox.h"


using namespace std;
namespace lib {
namespace messenger {

class IMJingle : public QObject,
                 public Jingle::SessionHandler,
                 public lib::ortc::OkRTCHandler,
                 public lib::ortc::OkRTCRenderer {
  Q_OBJECT

public:
  IMJingle(IM *client);
  ~IMJingle() override;

  void clientChanged(gloox::Client *client);

  /**
   * 发起呼叫
   * @param friendId
   * @param video
   * @return
   */
  bool startCall(const QString &friendId, const QString &sId, bool video);

  bool sendCallToResource(const QString &friendId, const QString &sId, bool video);

  bool createCall(const PeerId &to, const QString &sId, bool video);

  bool answer(const QString &friendId, const QString &callId, bool video);
  void cancel(const QString &friendId);
  void cancelCall(const QString &friendId, const QString &sId);
  void join(const JID &room);
  void setMute(bool mute);
  void setRemoteMute(bool mute);
  /**
   * File(接收)
   */
  void rejectFileRequest(const QString &friendId,
                         const FileHandler::File &file);
  void acceptFileRequest(const QString &friendId,
                         const FileHandler::File &file);
  void finishFileRequest(const QString &friendId,
                         const FileHandler::File &file);
  void finishFileTransfer(const QString &friendId,
                          const FileHandler::File &file);
  /**
   * File(发送)
   */
  bool sendFile(const QString &friendId, const FileHandler::File &file);
  bool sendFileToResource(const JID &friendId, const FileHandler::File &file);

protected:
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

  IMJingleSession *findSession(const std::string &sId);

private:
  void forClient(Client *client);

  IM *client;

  lib::ortc::JingleCallType m_callType;
  CallStage m_callStage;
  CallDirection m_callDirection;
  bool isAccepted = false;

  // friendId -> sid
  std::map<PeerId, std::string> m_friendSessionMap;
  //  std::map<PeerId, const Jingle::Session::Jingle *> m_jingleMap;
  // sid -> JingleContext
  //  std::map<std::string, lib::ortc::JingleContext> m_contextMap;
  // sid -> session
  std::map<std::string, IMJingleSession *> m_sessionMap;

//  std::unique_ptr<lib::ortc::OkRTCManager> _rtcManager;
  std::unique_ptr<Jingle::SessionManager> _sessionManager;

  QList<FileHandler::File> m_waitSendFiles;

  // file sending task map
  QMap<QString, IMFile *> m_fileSenderMap;

  QList<Jingle::Content *> m_ices;

  std::string getSessionByFriendId(const QString &friendId);

  void cacheSessionInfo(Jingle::Session *session, lib::ortc::JingleCallType callType);
  void clearSessionInfo(Jingle::Session *session);

  void doSessionInitiate(Jingle::Session *session,
                         const Jingle::Session::Jingle *, const PeerId &);

  void doSessionTerminate(Jingle::Session *session,
                          const Jingle::Session::Jingle *, const PeerId &);

  void doSessionAccept(Jingle::Session *session,        //
                       const Jingle::Session::Jingle *, //
                       const PeerId &);
  void doSessionInfo(const Jingle::Session::Jingle *, const PeerId &);
  void doContentAdd(const Jingle::Session::Jingle *, const PeerId &);
  void doContentRemove(const Jingle::Session::Jingle *, const PeerId &);
  void doContentModify(const Jingle::Session::Jingle *, const PeerId &);
  void doContentAccept(const Jingle::Session::Jingle *, const PeerId &);
  void doContentReject(const Jingle::Session::Jingle *, const PeerId &);
  void doTransportAccept(const Jingle::Session::Jingle *, const PeerId &);
  void doTransportInfo(const Jingle::Session::Jingle *, const PeerId &);
  void doTransportReject(const Jingle::Session::Jingle *, const PeerId &);
  void doTransportReplace(const Jingle::Session::Jingle *, const PeerId &);
  void doSecurityInfo(const Jingle::Session::Jingle *, const PeerId &);
  void doDescriptionInfo(const Jingle::Session::Jingle *, const PeerId &);
  void doInvalidAction(const Jingle::Session::Jingle *, const PeerId &);

  /**
   * 启动文件发送任务
   * @param session
   * @param file
   */
  void doStartFileSendTask(const Jingle::Session *session,
                           const FileHandler::File &file);

  /**
   * 停止文件发送任务
   * @param session
   * @param file
   */
  void doStopFileSendTask(const Jingle::Session *session,
                          const FileHandler::File &file);

signals:
  void receiveFriendCall(QString friendId, QString callId, bool audio,
                         bool video);

  void receiveFriendHangup(QString friendId, int state);

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
                          const FileHandler::File &file);

  void sendFileInfo(const QString &friendId, const FileHandler::File &file,
                    int m_seq, int m_sentBytes, bool end);

  void sendFileAbort(const QString &friendId, const FileHandler::File &file,
                     int m_sentBytes);
  void sendFileError(const QString &friendId, const FileHandler::File &file,
                     int m_sentBytes);
};

} // namespace IM
} // namespace lib
#endif // IMJINGLE_H
