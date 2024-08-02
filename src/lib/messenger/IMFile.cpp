/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

#include "IMFile.h"
#include "IM.h"
#include "IMFile.h"
#include "IMFileTask.h"
#include "base/logs.h"
#include "lib/session/AuthSession.h"
#include <QEventLoop>
#include <bytestream.h>
#include <inbandbytestream.h>
#include <utility>

namespace lib {
namespace messenger {

QString File::toString() const {
  return QString("{id:%1, sId:%2, name:%3, path:%4, size:%5, status:%6, direction:%7}").arg(id).arg(sId).arg(name).arg(path).arg(size).arg((int)status).arg((int)direction);
}

QDebug &operator<<(QDebug &debug, const File &f) {
  QDebugStateSaver saver(debug);
  debug.nospace() << f.toString();
  return debug;
}

IMFile::IMFile(IM *im, QObject *parent) : IMJingle(im, parent) {
  qRegisterMetaType<File>("File");

//  jingle = IMJingle::getInstance();
//  jingle->setFileHandlers(&fileHandlers);

  /*file handler*/
//  connect(jingle, &IMFile::receiveFileChunk, this, [&](const IMContactId &friendId, const QString &sId, int seq, const std::string &chunk) -> void {
//    for (auto handler : fileHandlers) {
//      handler->onFileRecvChunk(friendId.toString(), sId, seq, chunk);
//    }
//  });
//
//  connect(jingle, &IMFile::receiveFileFinished, this, [&](const IMContactId &friendId, const QString &sId) -> void {
//    for (auto handler : fileHandlers) {
//      handler->onFileRecvFinished(friendId.toString(), sId);
//    }
//  });

//  connect(jingle, &IMFile::receiveFileRequest, this, [&](const QString &friendId, const File &file) {
//    for (auto h : fileHandlers) {
//      h->onFileRequest(friendId, file);
//    }
//  });
}

IMFile::~IMFile() {
  //    disconnect(jingle, &IMFile::receiveFileChunk, this);
}

void IMFile::addFileHandler(FileHandler *handler) { fileHandlers.emplace_back(handler); }

void IMFile::fileRejectRequest(QString friendId, const File &file) {
  auto sId = file.sId;
  qDebug() << __func__ << sId;
  rejectFileRequest(friendId, sId);
}

void IMFile::fileAcceptRequest(QString friendId, const File &file) {
  auto sId = file.sId;
  qDebug() << __func__ << sId;
  acceptFileRequest(friendId, file);
}

void IMFile::fileCancel(QString fileId) { qDebug() << __func__ << "file" << fileId; }

void IMFile::fileFinishRequest(QString friendId, const QString &sId) {
  qDebug() << __func__ << sId;
  finishFileRequest(friendId, sId);
}

void IMFile::fileFinishTransfer(QString friendId, const QString &sId) {
  qDebug() << __func__ << sId;
  finishFileTransfer(friendId, sId);
}

bool IMFile::fileSendToFriend(const QString &f, const File &file) {
  qDebug() << __func__ << "friend:" << f << "file:" << file.id;

  auto bare = stdstring(f);

  auto resources = im->getOnlineResources(bare);
  if (resources.empty()) {
    qWarning() << "Can not find online friends:" << f;
    return false;
  }

  JID jid(bare);
  for (auto &r : resources) {
    jid.setResource(r);
    sendFileToResource(jid, file);
  }

  return jingle;
}

void IMFile::addFile(const File &f) {
  m_waitSendFiles.append(f);
}


void IMFile::doStartFileSendTask(const Session *session,
                                          const File &file) {
  qDebug()<<__func__<<file.sId ;

  auto *fileTask = new IMFileTask(session->remote(), &file, im);
  connect(fileTask, &IMFileTask::fileSending,
          [&](const JID &m_friendId, const File &m_file, int m_seq,
              int m_sentBytes, bool end) {

            for(auto h: fileHandlers){
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
            for(auto h: fileHandlers){
              h->onFileSendAbort(qstring(m_friendId.bare()), m_file, m_sentBytes);
            }
          });

  connect(fileTask, &IMFileTask::fileError,
          [&](const JID &m_friendId, const File &m_file, int m_sentBytes) {
            for(auto h: fileHandlers){
              h->onFileSendError(qstring(m_friendId.bare()), m_file, m_sentBytes);
            }
            //            emit sendFileError(qstring(m_friendId.bare()), m_file,
            //                               m_sentBytes);
          });
  fileTask->start();
  m_fileSenderMap.insert(file.id, fileTask);
  qDebug()<<__func__<<("Send file task has been stared.")<<((file.id));
}


void IMFile::doStopFileSendTask(const Session *session,
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



/**
 * 文件传输
 * @param friendId
 * @param file
 */
void IMFile::rejectFileRequest(const QString &friendId,
                                 const QString &sId) {

//  cancelCall(IMContactId{friendId}, sId);

//  void IMJingle::cancelCall(const IMContactId &friendId, const QString &sId) {
    qDebug()<< __func__ << friendId << sId;

    IMJingleSession *s = findSession(sId);
    if (s) {
      s->doTerminate();
      s->setCallStage(CallStage::StageNone);
      clearSessionInfo(s->getSession());
    }
    retractJingleMessage(friendId, sId);
    //  else {
    // jingle-message
    //    if (s->direction() == CallDirection:: CallOut) {
    //    } else if (s->direction() == CallDirection:: CallIn) {
    //      rejectJingleMessage(friendId.toString(), sId);
    //    }
    //  }
//  }

}

void IMFile::acceptFileRequest(const QString &friendId, const File &file) {
  qDebug()<< __func__
           << file.name
           << file.sId
           << file.id;

  auto *ws = findSession(file.sId);
  if (!ws) {
    qWarning() <<"Unable to find session sId:" << file.sId;
    return;
  }

  // 协议：https://xmpp.org/extensions/xep-0234.html#requesting
  PluginList pluginList;

  Jingle::FileTransfer::FileList files;
  files.push_back(Jingle::FileTransfer::File   {
      .name = stdstring( file.name),
      .size = (long)file.size
  });

  auto ftf = new Jingle::FileTransfer(FileTransfer::Request, files);

  auto ibb = new Jingle::IBB(stdstring(file.txIbb.sid),
                             file.txIbb.blockSize);

  pluginList.emplace_back(ftf);
  pluginList.emplace_back(ibb);


  auto c = new Jingle::Content("file", pluginList);
  ws->getSession()->sessionAccept(c);
}

void IMFile::finishFileRequest(const QString &friendId,
                                 const QString &sId) {
  qDebug()<<__func__<<"sId:"<<(sId);
  auto *s = findSession(sId);
  if (!s) {
    qWarning() << "Can not find file session" << sId;
    return;
  }
  s->getSession()->sessionTerminate(new Session::Reason(Session::Reason::Success));
}

void IMFile::finishFileTransfer(const QString &friendId,
                                  const QString &sId) {
  qDebug()<<__func__<<"sId:"<<(sId);
  finishFileRequest(friendId, sId);
}

bool IMFile::sendFile(const QString &friendId,
                        const File &file) {
  qDebug()<<__func__ << (friendId) << (file.name);
  if (file.id.isEmpty()) {
    qWarning() << "file id is no existing";
    return false;
  }

  auto bare = stdstring(friendId);
  auto resources = im->getOnlineResources(bare);
  if (resources.empty()) {
    qWarning() << "目标用户不在线！";
    return false;
  }

  JID jid(bare);
  for (auto &r : resources) {
    jid.setResource(r);
    sendFileToResource(jid, file);
  }

  return true;
}

bool IMFile::sendFileToResource(const JID &jid, const File &file) {

  qDebug()<<__func__<< qstring(jid.full()) << "sId:"<<file.sId;
  auto session = _sessionManager->createSession(jid, this, stdstring(file.sId));
  if (!session) {
    qDebug() << "Can not create session!";
    return false;
  }

  auto ws = cacheSessionInfo(session, ortc::JingleCallType::file);

  PluginList pl;

  // offer-file
  FileTransfer::FileList files;
  FileTransfer::File f;
  f.name = stdstring(file.name);
  f.size = file.size;
  files.emplace_back(f);
  auto ft = new FileTransfer(FileTransfer::Offer, files);
  pl.emplace_back(ft);

  // ibb
  auto ibb = new Jingle::IBB(stdstring(file.id), 4096);
  pl.emplace_back(ibb);

  // content
  auto jc = new Jingle::Content("offer-a-file", pl);
  session->sessionInitiate(jc);

  // 缓存文件
  auto &nf = const_cast<File &>(file);
  nf.sId = qstring(session->sid());
  addFile(nf);

  return true;
}

} // namespace messenger
} // namespace lib
