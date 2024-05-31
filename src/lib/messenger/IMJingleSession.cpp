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

#include "IMFile.h"
#include "IMFileTask.h"
#include "IMJingleSession.h"
#include "lib/ortc/ok_rtc_proxy.h"
#include "lib/ortc/ok_rtc_renderer.h"
#include "base/basic_types.h"
#include <QDebug>

namespace lib {
namespace messenger {

IMJingleSession::IMJingleSession(IM* im,
                                 const IMPeerId &peerId,
                                 const QString &sId_,
                                 lib::ortc::JingleCallType callType,
                                 Session *mSession,
                                 std::list<ortc::IceServer> iceServers,
                                 std::vector<FileHandler *> *fileHandlers,
                                 ortc::OkRTCHandler *handler,
                                 ortc::OkRTCRenderer *renderer)
    : im{im},
      sId(sId_),
      session(mSession),
      accepted(false),
      fileHandlers{fileHandlers},
      m_callType{callType}
{
  qDebug() << __func__
           <<"type:" << (int)m_callType
          <<"sid:" << sId
           <<"to peer:"<<peerId.toString();

  if(m_callType == ortc::JingleCallType::audio || m_callType == ortc::JingleCallType::video){
    _rtcManager = std::make_unique<lib::ortc::OkRTCManager>(iceServers, handler, renderer); //
    _rtcManager->start(stdstring(peerId.toString()), stdstring(sId), m_callType);
  }
  qDebug()<< __func__ << "be created.";
}

IMJingleSession::~IMJingleSession() {
  qDebug() << __func__ << sId;
}

Session *IMJingleSession::getSession() const { return session; }

void IMJingleSession::onAccept()
{

    if (m_callType == lib::ortc::JingleCallType::file) {
      // file
      for (auto &file : m_waitSendFiles) {
        doStartFileSendTask(session, file);
      }
    }else{
        // av


        // RTC 接受会话
//        getRtcManager()->SetRemoteDescription(stdstring(peerId.toString()),
//                                                  answer);

//        emit receiveFriendHangup(
//            peerId.username, answer.hasVideo() ? TOXAV_FRIEND_CALL_STATE_SENDING_V
//                                               : TOXAV_FRIEND_CALL_STATE_SENDING_A);
    }
}

void IMJingleSession::onTerminate()
{
    qDebug()<<__func__;
}

void IMJingleSession::createOffer(const std::string &peerId)
{
    qDebug() << __func__ << "to" << peerId.c_str();
    _rtcManager->CreateOffer(peerId);
}

const Session::Jingle *IMJingleSession::getJingle() const { return jingle; }

void IMJingleSession::setJingle(const Session::Jingle *jingle) {
    IMJingleSession::jingle = jingle;
}

CallDirection IMJingleSession::direction() const
{
    auto sender = session->initiator().bareJID();
    auto self = im->self().bareJID();

    return (sender==self) ? CallDirection::CallOut:CallDirection::CallIn;
}

void IMJingleSession::setCallStage(CallStage state)
{
    m_callStage=state;
}

void IMJingleSession::setContext(const ortc::JingleContext &jc) {
  context = jc;
}


void IMJingleSession::doStartFileSendTask(const Session *session,
                                   const File &file) {
  qDebug()<<__func__<<file.sId ;

  auto *fileTask = new IMFileTask(session->remote(), &file, im);
  connect(fileTask, &IMFileTask::fileSending,
          [&](const JID &m_friendId, const File &m_file, int m_seq,
              int m_sentBytes, bool end) {

            for(auto h: *fileHandlers){
                h->onFileSendInfo(qstring(m_friendId.bare()),
                                  m_file,
                                  m_seq,
                                  m_sentBytes, end);
            }

//            emit sendFileInfo(qstring(m_friendId.bare()), m_file, m_seq,
//                              m_sentBytes, end);
          });

  connect(fileTask, &IMFileTask::fileAbort,
          [&](const JID &m_friendId, const File &m_file,
              int m_sentBytes) {
//            emit sendFileAbort(qstring(m_friendId.bare()), m_file, m_sentBytes);
            for(auto h: *fileHandlers){
                h->onFileSendAbort(qstring(m_friendId.bare()), m_file, m_sentBytes);
            }
          });

  connect(fileTask, &IMFileTask::fileError,
          [&](const JID &m_friendId, const File &m_file, int m_sentBytes) {
      for(auto h: *fileHandlers){
          h->onFileSendError(qstring(m_friendId.bare()), m_file, m_sentBytes);
      }
//            emit sendFileError(qstring(m_friendId.bare()), m_file,
//                               m_sentBytes);
          });
  fileTask->start();
  m_fileSenderMap.insert(file.id, fileTask);
  qDebug()<<__func__<<("Send file task has been stared.")<<((file.id));
}


void IMJingleSession::doStopFileSendTask(const Session *session,
                                  const File &file) {
    Q_UNUSED(session)
  qDebug()<<__func__<<file.sId ;
  auto *fileTask = m_fileSenderMap.value(file.sId);
  if (!fileTask) {
    return;
  }

  qDebug()<<__func__<<"Send file task will be clear."<<file.id;
  if (fileTask->isRunning()) {
    fileTask->forceQuit();
  }
  disconnect(fileTask);
  delete fileTask;

  // 返回截断后续处理
  m_fileSenderMap.remove(file.sId);
  qDebug() << "Send file task has been clean."<<file.id;

}

} // namespace IM
} // namespace lib
