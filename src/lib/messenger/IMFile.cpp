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

#include "IM.h"
#include "IMFile.h"
#include "IMJingle.h"
#include "lib/session/AuthSession.h"
#include "base/logs.h"
#include <QEventLoop>
#include <bytestream.h>
#include <inbandbytestream.h>
#include <utility>

namespace lib {
namespace messenger {

QString File::toString() const
{
    return QString("{id:%1, sId:%2, name:%3, path:%4, size:%5, status:%6, direction:%7}")
           .arg(id).arg(sId).arg(name).arg(path).arg(size)
           .arg((int)status).arg((int)direction);
}

QDebug &operator<<(QDebug &debug, const File &f) {
  QDebugStateSaver saver(debug);
  debug.nospace() << f.toString();
  return debug;
}

IMFile::IMFile(QObject *parent): QObject(parent)
{
    qRegisterMetaType<File>("File");

    auto _session = ok::session::AuthSession::Instance();
    auto _im = _session->im();

    jingle = new IMJingle(_im, &fileHandlers, this);

    /*file handler*/
    connect(jingle, &IMJingle::receiveFileChunk, this,
            [&](const IMContactId &friendId, const QString &sId,
                int seq, const std::string& chunk) -> void {
              for (auto handler : fileHandlers) {
                handler->onFileRecvChunk(friendId.toString(), sId, seq, chunk);
              }
            });

    connect(jingle, &IMJingle::receiveFileFinished, this,
        [&](const IMContactId &friendId,const QString &sId) -> void {
          for (auto handler : fileHandlers) {
                handler->onFileRecvFinished(friendId.toString(), sId);
          }
        });

    connect(jingle, &IMJingle::receiveFileRequest, this,
            [&](const QString &friendId, const File &file) {
              for (auto h : fileHandlers) {
                h->onFileRequest(friendId, file);
              }
            });

}

IMFile::~IMFile()
{
//    disconnect(jingle, &IMJingle::receiveFileChunk, this);

}

void IMFile::addFileHandler(FileHandler *handler) {
  fileHandlers.emplace_back(handler);
}

void IMFile::fileRejectRequest(QString friendId, const File &file) {
    auto sId = file.sId;
    qDebug() << __func__<<sId;
    jingle->rejectFileRequest(friendId, sId);
}

void IMFile::fileAcceptRequest(QString friendId, const File &file) {
   auto sId = file.sId;
   qDebug() << __func__<<sId;
    jingle->acceptFileRequest(friendId, file);
}

void IMFile::fileCancel(QString fileId) {
  qDebug() << __func__ << "file" << fileId;
}

void IMFile::fileFinishRequest(QString friendId, const QString &sId) {
  qDebug() << __func__<<sId;
  jingle->finishFileRequest(friendId, sId);
}

void IMFile::fileFinishTransfer(QString friendId, const QString &sId) {
    qDebug() << __func__<<sId;
    jingle->finishFileTransfer(friendId, sId);
}


bool IMFile::fileSendToFriend(const QString &f, const File &file) {
    qDebug() << __func__ <<"friend:" << f <<"file:" <<file.id;

    auto bare = stdstring(f);

    auto im = ok::session::AuthSession::Instance()->im();
    auto resources = im->getOnlineResources(bare);
    if (resources.empty()) {
      qWarning() << "Can not find online friends:" << f;
      return false;
    }

    JID jid(bare);
    for (auto &r : resources) {
      jid.setResource(r);
      jingle->sendFileToResource(jid, file);
    }

    return jingle;
}



} // namespace IMFile
} // namespace lib
