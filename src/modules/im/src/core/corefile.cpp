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

#include "corefile.h"
#include "core.h"
#include "src/model/status.h"
#include "src/model/toxclientstandards.h"
#include "src/persistence/settings.h"
#include "base/compatiblerecursivemutex.h"
#include "toxfile.h"
#include "toxstring.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QThread>
#include <cassert>
#include <memory>

/**
 * @class CoreFile
 * @brief Manages the file transfer service of toxcore
 */

CoreFilePtr CoreFile::makeCoreFile(Core *core, Tox *tox,
                                   CompatibleRecursiveMutex &coreLoopLock) {
  assert(core != nullptr);
  assert(tox != nullptr);

  CoreFilePtr result = CoreFilePtr{new CoreFile{tox, coreLoopLock}};
  connect(core, &Core::friendStatusChanged, result.get(),
          &CoreFile::onConnectionStatusChanged);

  return result;
}

CoreFile::CoreFile(Tox *core, CompatibleRecursiveMutex &coreLoopLock)
    : tox{core}, coreLoopLock{&coreLoopLock} {
  connectCallbacks(*tox);
}

/**
 * @brief Get corefile iteration interval.
 *
 * tox_iterate calls to get good file transfer performances
 * @return The maximum amount of time in ms that Core should wait between two
 * tox_iterate() calls.
 */
unsigned CoreFile::corefileIterationInterval() {
  /*
     Sleep at most 1000ms if we have no FT, 10 for user FTs
     There is no real difference between 10ms sleep and 50ms sleep when it
     comes to CPU usage – just keep the CPU usage low when there are no file
     transfers, and speed things up when there is an ongoing file transfer.
  */
  constexpr unsigned fileInterval = 10, idleInterval = 1000;

  for (ToxFile &file : fileMap) {
    if (file.status == ToxFile::TRANSMITTING) {
      return fileInterval;
    }
  }
  return idleInterval;
}

void CoreFile::connectCallbacks(Tox &tox) {
  qDebug() << "CoreFile::connectCallbacks";
  tox.addFileHandler(this);

  // be careful not to reconnect already used callbacks here
  //  tox_callback_file_chunk_request(&tox, CoreFile::onFileDataCallback);
  //  tox_callback_file_recv(&tox, CoreFile::onFileReceiveCallback);
  //  tox_callback_file_recv_chunk(&tox, CoreFile::onFileRecvChunkCallback);
  //  tox_callback_file_recv_control(&tox, CoreFile::onFileControlCallback);
}

void CoreFile::sendAvatarFile(QString friendId, const QByteArray &data) {
  //  QMutexLocker{coreLoopLock};
  //
  //  uint64_t filesize = 0;
  //  uint8_t *file_id = nullptr;
  //  uint8_t *file_name = nullptr;
  //  size_t nameLength = 0;
  //  uint8_t avatarHash[TOX_HASH_LENGTH];
  //  if (!data.isEmpty()) {
  //    static_assert(TOX_HASH_LENGTH <= TOX_FILE_ID_LENGTH,
  //                  "TOX_HASH_LENGTH > TOX_FILE_ID_LENGTH!");
  //    tox_hash(avatarHash, (uint8_t *)data.data(), data.size());
  //    filesize = data.size();
  //    file_id = avatarHash;
  //    file_name = avatarHash;
  //    nameLength = TOX_HASH_LENGTH;
  //  }
  //  Tox_Err_File_Send error;
  //  const uint32_t fileNum = 0 ;
  ////  tox_file_send(tox, friendId, TOX_FILE_KIND_AVATAR, filesize, file_id,
  ////                    file_name, nameLength, &error);
  //
  //  switch (error) {
  //  case TOX_ERR_FILE_SEND_OK:
  //    break;
  //  case TOX_ERR_FILE_SEND_FRIEND_NOT_CONNECTED:
  //    qCritical() << "Friend not connected";
  //    return;
  //  case TOX_ERR_FILE_SEND_FRIEND_NOT_FOUND:
  //    qCritical() << "Friend not found";
  //    return;
  //  case TOX_ERR_FILE_SEND_NAME_TOO_LONG:
  //    qCritical() << "Name too long";
  //    return;
  //  case TOX_ERR_FILE_SEND_NULL:
  //    qCritical() << "Send null";
  //    return;
  //  case TOX_ERR_FILE_SEND_TOO_MANY:
  //    qCritical() << "To many ougoing transfer";
  //    return;
  //  default:
  //    return;
  //  }
  //
  //  ToxFile file{fileNum, friendId, "", "", ToxFile::SENDING};
  //  file.filesize = filesize;
  //  file.fileKind = TOX_FILE_KIND_AVATAR;
  //  file.avatarData = data;
  //  file.resumeFileId.resize(TOX_FILE_ID_LENGTH);
  ////  tox_file_get_file_id(tox, friendId, fileNum,
  ////                       (uint8_t *)file.resumeFileId.data(), nullptr);
  //  addFile(friendId, fileNum, file);
}

void CoreFile::sendFile(QString friendId, QString filename, QString filePath,
                        long long filesize) {
    QMutexLocker{coreLoopLock};

    QString fileId = QDateTime::currentDateTime().toString();
    qDebug()<<"CoreFile::sendFile" << filePath <<"fileId"<< fileId;

    QString fileNum = filename;
    ToxFile file{fileId, friendId, fileNum, filename,
                 filePath, ToxFile::SENDING};

    file.filesize = filesize;
    file.status = ToxFile::FileStatus::INITIALIZING;
    file.resumeFileId = fileId.toLatin1();

//  file.resumeFileId.resize(TOX_FILE_ID_LENGTH);


    bool y = tox->sendFileToFriend(friendId, buildHandlerFile(&file));
    if (!y) {
      qWarning() << "sendFile: Sending file is failed.";
      emit fileSendFailed(friendId, filename);
      return;
    }
    qDebug() << QString("sendFile: Created file sender %1 with friend %2")
                    .arg(fileNum)
                    .arg(friendId);


  //  tox_file_get_file_id(tox, friendId, fileNum,
  //                       (uint8_t *)file.resumeFileId.data(), nullptr);
    if (!file.open(false)) {
      qWarning() << QString("sendFile: Can't open file, error: %1")
                        .arg(file.file->errorString());
    }

    addFile(friendId, fileNum, file);

    emit fileSendStarted(file);
}

void CoreFile::pauseResumeFile(QString friendId, QString fileId) {
qDebug()<<"暂不支持";
  //  QMutexLocker{coreLoopLock};
//
//  ToxFile *file = findFile(friendId, fileId);
//  if (!file) {
//    qWarning("pauseResumeFileSend: No such file in queue");
//    return;
//  }
//
//  if (file->status != ToxFile::TRANSMITTING &&
//      file->status != ToxFile::PAUSED) {
//    qWarning() << "pauseResumeFileSend: File is stopped";
//    return;
//  }
//
//  file->pauseStatus.localPauseToggle();
//
//  if (file->pauseStatus.paused()) {
//    file->status = ToxFile::PAUSED;
//    emit fileTransferPaused(*file);
//  } else {
//    file->status = ToxFile::TRANSMITTING;
//    emit fileTransferAccepted(*file);
//  }

  //  if (file->pauseStatus.localPaused()) {
  //    tox_file_control(tox, file->friendId, file->fileNum,
  //    TOX_FILE_CONTROL_PAUSE,
  //                     nullptr);
  //  } else {
  //    tox_file_control(tox, file->friendId, file->fileNum,
  //                     TOX_FILE_CONTROL_RESUME, nullptr);
  //  }
}

void CoreFile::cancelFileSend(QString friendId, QString fileId) {
    QMutexLocker{coreLoopLock};

    ToxFile *file = findFile(friendId, fileId);
    if (!file) {
      qWarning("cancelFileSend: No such file in queue");
      return;
    }

    file->status = ToxFile::CANCELED;
    tox->cancelFile(file->fileNum);
    emit fileTransferCancelled(*file);
//    tox_file_control(tox, file->friendId, file->fileNum,
//    TOX_FILE_CONTROL_CANCEL,
//                     nullptr);


    removeFile(friendId, fileId);
}

void CoreFile::cancelFileRecv(QString friendId, QString fileId) {
    QMutexLocker{coreLoopLock};

    ToxFile *file = findFile(friendId, fileId);
    if (!file) {
      qWarning("cancelFileRecv: No such file in queue");
      return;
    }
    file->status = ToxFile::CANCELED;
    tox->rejectFileRequest(friendId, buildHandlerFile(file));
    emit fileTransferCancelled(*file);
    removeFile(friendId, fileId);
}

void CoreFile::rejectFileRecvRequest(QString friendId, QString fileId) {
    QMutexLocker{coreLoopLock};

    ToxFile *file = findFile(friendId, fileId);
    if (!file) {
      qWarning("cancelFileRecv: No such file in queue");
      return;
    }
    file->status = ToxFile::CANCELED;
    tox->rejectFileRequest(friendId, buildHandlerFile(file));
    removeFile(friendId, fileId);
    emit fileTransferCancelled(*file);
}

void CoreFile::acceptFileRecvRequest(QString friendId,
                                     QString fileId,
                                     QString path) {
    QMutexLocker{coreLoopLock};

    ToxFile *file = findFile(friendId, fileId);
    if (!file) {
      qWarning("acceptFileRecvRequest: No such file in queue");
      return;
    }
    file->setFilePath(path);
    if (!file->open(true)) {
      qWarning() << "acceptFileRecvRequest: Unable to open file";
      return;
    }
    file->status = ToxFile::TRANSMITTING;
    tox->acceptFileRequest(friendId, buildHandlerFile(file));
    emit fileTransferAccepted(*file);
}

ToxFile *CoreFile::findFile(QString friendId, QString fileId) {
  QMutexLocker{coreLoopLock};
  
  qDebug() << "friendId" << friendId << "fileId" << fileId;

  QString key = getFriendKey(friendId, fileId);
  if (fileMap.contains(key)) {
    return &fileMap[key];
  }

  qWarning() << "findFile: File transfer with ID" << friendId << ':' << fileId
             << "doesn't exist";
  return nullptr;
}

void CoreFile::addFile(QString friendId, QString fileId, const ToxFile &file) {
  qDebug() << "addFile friend:" << friendId << "fileId:"<<fileId;
  QString key = getFriendKey(friendId, fileId);

  if (fileMap.contains(key)) {
    qWarning() << "addFile: Overwriting existing file transfer with same ID"
               << friendId << ':' << fileId;
  }

  fileMap.insert(key, file);
}

void CoreFile::removeFile(QString friendId, QString fileId) {
  qDebug() << "removeFile friendId:" << friendId << "file:"<< fileId;
  QString key = getFriendKey(friendId, fileId);
  if (!fileMap.contains(key)) {
    qWarning() << "removeFile: No such file in queue";
    return;
  }
  fileMap[key].file->close();
  fileMap.remove(key);
}

QString CoreFile::getCleanFileName(QString filename) {
  QRegularExpression regex{QStringLiteral(R"([<>:"/\\|?])")};
  filename.replace(regex, "_");

  return filename;
}

void CoreFile::onFileReceiveCallback(Tox *tox, QString friendId,
                                     QString fileId, uint32_t kind,
                                     uint64_t filesize, const uint8_t *fname,
                                     size_t fnameLen, void *vCore) {
  //  Core *core = static_cast<Core *>(vCore);
  //  CoreFile *coreFile = core->getCoreFile();
  //  auto filename = ToxString(fname, fnameLen);
  //  const ToxPk friendPk = core->getFriendPublicKey(friendId);
  //
  //  if (kind == TOX_FILE_KIND_AVATAR) {
  //    if (!filesize) {
  //      qDebug() << QString("Received empty avatar request %1:%2")
  //                      .arg(friendId)
  //                      .arg(fileId);
  //      // Avatars of size 0 means explicitely no avatar
  //      tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL,
  //      nullptr); emit
  //      core->friendAvatarRemoved(core->getFriendPublicKey(friendId)); return;
  //    } else {
  //      if (!ToxClientStandards::IsValidAvatarSize(filesize)) {
  //        qWarning() << QString("Received avatar request from %1 with size
  //        %2.")
  //                              .arg(friendId)
  //                              .arg(filesize) +
  //                          QString(" The max size allowed for avatars is %3.
  //                          "
  //                                  "Cancelling transfer.")
  //                              .arg(ToxClientStandards::MaxAvatarSize);
  //        tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL,
  //                         nullptr);
  //        return;
  //      }
  //      static_assert(TOX_HASH_LENGTH <= TOX_FILE_ID_LENGTH,
  //                    "TOX_HASH_LENGTH > TOX_FILE_ID_LENGTH!");
  //      uint8_t avatarHash[TOX_FILE_ID_LENGTH];
  //      tox_file_get_file_id(tox, friendId, fileId, avatarHash, nullptr);
  //      QByteArray avatarBytes{
  //          static_cast<const char *>(static_cast<const void *>(avatarHash)),
  //          TOX_HASH_LENGTH};
  //      emit core->fileAvatarOfferReceived(friendId, fileId, avatarBytes);
  //      return;
  //    }
  //  } else {
  //    const auto cleanFileName =
  //        CoreFile::getCleanFileName(filename.getQString());
  //    if (cleanFileName != filename.getQString()) {
  //      qDebug() << QStringLiteral("Cleaned filename");
  //      filename = ToxString(cleanFileName);
  //      emit coreFile->fileNameChanged(friendPk);
  //    } else {
  //      qDebug() << QStringLiteral("filename already clean");
  //    }
  //    qDebug() << QString("Received file request %1:%2 kind %3")
  //                    .arg(friendId)
  //                    .arg(fileId)
  //                    .arg(kind);
  //  }
  //
  //  ToxFile file{fileId, friendId, filename.getBytes(), "",
  //  ToxFile::RECEIVING}; file.filesize = filesize; file.fileKind = kind;
  //  file.resumeFileId.resize(TOX_FILE_ID_LENGTH);
  //  tox_file_get_file_id(tox, friendId, fileId,
  //                       (uint8_t *)file.resumeFileId.data(), nullptr);
  //  coreFile->addFile(friendId, fileId, file);
  //  if (kind != TOX_FILE_KIND_AVATAR) {
  //    emit coreFile->fileReceiveRequested(file);
  //  }
}

// TODO(sudden6): This whole method is a mess but needed to get stuff working
// for now
void CoreFile::handleAvatarOffer(QString friendId, QString fileId,
                                 bool accept) {
  //  if (!accept) {
  //    // If it's an avatar but we already have it cached, cancel
  //    qDebug() << QString("Received avatar request %1:%2, reject, since we
  //    have "
  //                        "it in cache.")
  //                    .arg(friendId)
  //                    .arg(fileId);
  //    tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL,
  //    nullptr); return;
  //  }
  //
  //  // It's an avatar and we don't have it, autoaccept the transfer
  //  qDebug()
  //      << QString(
  //             "Received avatar request %1:%2, accept, since we don't have it
  //             " "in cache.") .arg(friendId) .arg(fileId);
  //  tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_RESUME, nullptr);
  //
  //  ToxFile file{fileId, friendId, "<avatar>", "", ToxFile::RECEIVING};
  //  file.filesize = 0;
  //  file.fileKind = TOX_FILE_KIND_AVATAR;
  //  file.resumeFileId.resize(TOX_FILE_ID_LENGTH);
  //  tox_file_get_file_id(tox, friendId, fileId,
  //                       (uint8_t *)file.resumeFileId.data(), nullptr);
  //  addFile(friendId, fileId, file);
}

void CoreFile::onFileRequest(const QString &friendId,
                             const lib::messenger::FileHandler::File &file) {
  qDebug() << "onFileRequest friendId:"<< friendId;
  ToxFile toxFile(file.sId, friendId, file.id, file.name, "", ToxFile::RECEIVING);
  toxFile.filesize = file.size;
  toxFile.bytesSent = 0;
  addFile(friendId, toxFile.fileNum, toxFile);
  emit fileReceiveRequested(toxFile);
}

void CoreFile::onFileControlCallback(Tox *, QString friendId, QString fileId,
                                     Tox_File_Control control, void *vCore) {
  Core *core = static_cast<Core *>(vCore);
  CoreFile *coreFile = core->getCoreFile();
  ToxFile *file = coreFile->findFile(friendId, fileId);
  if (!file) {
    qWarning("onFileControlCallback: No such file in queue");
    return;
  }

  if (control == TOX_FILE_CONTROL_CANCEL) {
    if (file->fileKind != TOX_FILE_KIND_AVATAR)
      qDebug() << "File transfer" << friendId << ":" << fileId
               << "cancelled by friend";
    file->status = ToxFile::CANCELED;
    emit coreFile->fileTransferCancelled(*file);
    coreFile->removeFile(friendId, fileId);
  } else if (control == TOX_FILE_CONTROL_PAUSE) {
    qDebug() << "onFileControlCallback: Received pause for file " << friendId
             << ":" << fileId;
    file->pauseStatus.remotePause();
    file->status = ToxFile::PAUSED;
    emit coreFile->fileTransferRemotePausedUnpaused(*file, true);
  } else if (control == TOX_FILE_CONTROL_RESUME) {
    if (file->direction == ToxFile::SENDING &&
        file->fileKind == TOX_FILE_KIND_AVATAR)
      qDebug() << "Avatar transfer" << fileId << "to friend" << friendId
               << "accepted";
    else
      qDebug() << "onFileControlCallback: Received resume for file " << friendId
               << ":" << fileId;
    file->pauseStatus.remoteResume();
    file->status =
        file->pauseStatus.paused() ? ToxFile::PAUSED : ToxFile::TRANSMITTING;
    emit coreFile->fileTransferRemotePausedUnpaused(*file, false);
  } else {
    qWarning() << "Unhandled file control " << control << " for file "
               << friendId << ':' << fileId;
  }
}

void CoreFile::onFileDataCallback(Tox *tox, QString friendId, QString fileId,
                                  uint64_t pos, size_t length, void *vCore) {

  //  Core *core = static_cast<Core *>(vCore);
  //  CoreFile *coreFile = core->getCoreFile();
  //  ToxFile *file = coreFile->findFile(friendId, fileId);
  //  if (!file) {
  //    qWarning("onFileDataCallback: No such file in queue");
  //    return;
  //  }
  //
  //  // If we reached EOF, ack and cleanup the transfer
  //  if (!length) {
  //    file->status = ToxFile::FINISHED;
  //    if (file->fileKind != TOX_FILE_KIND_AVATAR) {
  //      emit coreFile->fileTransferFinished(*file);
  //      emit coreFile->fileUploadFinished(file->filePath);
  //    }
  //    coreFile->removeFile(friendId, fileId);
  //    return;
  //  }
  //
  //  std::unique_ptr<uint8_t[]> data(new uint8_t[length]);
  //  int64_t nread;
  //
  //  if (file->fileKind == TOX_FILE_KIND_AVATAR) {
  //    QByteArray chunk = file->avatarData.mid(pos, length);
  //    nread = chunk.size();
  //    memcpy(data.get(), chunk.data(), nread);
  //  } else {
  //    file->file->seek(pos);
  //    nread = file->file->read((char *)data.get(), length);
  //    if (nread <= 0) {
  //      qWarning("onFileDataCallback: Failed to read from file");
  //      file->status = ToxFile::CANCELED;
  //      emit coreFile->fileTransferCancelled(*file);
  //      tox_file_send_chunk(tox, friendId, fileId, pos, nullptr, 0, nullptr);
  //      coreFile->removeFile(friendId, fileId);
  //      return;
  //    }
  //    file->bytesSent += length;
  //    file->hashGenerator->addData((const char *)data.get(), length);
  //  }
  //
  //  if (!tox_file_send_chunk(tox, friendId, fileId, pos, data.get(), nread,
  //                           nullptr)) {
  //    qWarning("onFileDataCallback: Failed to send data chunk");
  //    return;
  //  }
  //  if (file->fileKind != TOX_FILE_KIND_AVATAR) {
  //    emit coreFile->fileTransferInfo(*file);
  //  }
}

void CoreFile::onFileSendInfo(const QString &friendId,                  //
                              const lib::messenger::FileHandler::File &file_,  //
                              int m_seq, int m_sentBytes, bool end) {

    ToxFile *file = findFile(friendId, file_.id);
    if (!file) {
      qWarning("onFileDataCallback: No such file in queue");
      return;
    }
    qDebug() << "onFileSendInfo " << friendId << "file" << file_.id 
             << "seq"<< m_seq
             << "sentBytes" << m_sentBytes;
    file->bytesSent += m_sentBytes;
    if (!end) {
      file->status = ToxFile::FileStatus::TRANSMITTING;
      emit fileTransferInfo(*file);
    } else {
      file->status = ToxFile::FileStatus::FINISHED;
      emit fileTransferFinished(*file);
      emit fileUploadFinished(file->filePath);
      removeFile(friendId, file->fileNum);
    }
}

void CoreFile::onFileSendAbort(const QString &friendId, const lib::messenger::FileHandler::File &file_, int m_sentBytes) {

    ToxFile *file = findFile(friendId, file_.id);
    if (!file) {
      qWarning("onFileDataCallback: No such file in queue");
      return;
    }
    file->bytesSent = m_sentBytes;
    file->status = ToxFile::FileStatus::CANCELED;
    emit fileTransferCancelled(*file);
    removeFile(friendId, file->fileNum);
}

void CoreFile::onFileSendError(const QString &friendId,
                               const lib::messenger::FileHandler::File &file_,
                               int m_sentBytes) {
    ToxFile *file = findFile(friendId, file_.id);
    if (!file) {
      qWarning("onFileDataCallback: No such file in queue");
      return;
    }
    file->bytesSent = m_sentBytes;
    file->status = ToxFile::FileStatus::CANCELED;
    emit fileSendFailed(friendId, file->fileName);
    removeFile(friendId, file->fileNum);
}

void CoreFile::onFileRecvChunkCallback(Tox *tox, QString friendId,
                                       QString fileId, uint64_t position,
                                       const uint8_t *data, size_t length,
                                       void *vCore) {
  //  Core *core = static_cast<Core *>(vCore);
  //  CoreFile *coreFile = core->getCoreFile();
  //  ToxFile *file = coreFile->findFile(friendId, fileId);
  //  if (!file) {
  //    qWarning("onFileRecvChunkCallback: No such file in queue");
  //    tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL,
  //    nullptr); return;
  //  }
  //
  //  if (file->bytesSent != position) {
  //    qWarning("onFileRecvChunkCallback: Received a chunk out-of-order,
  //    aborting "
  //             "transfer");
  //    if (file->fileKind != TOX_FILE_KIND_AVATAR) {
  //      file->status = ToxFile::CANCELED;
  //      emit coreFile->fileTransferCancelled(*file);
  //    }
  //    tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL,
  //    nullptr); coreFile->removeFile(friendId, fileId); return;
  //  }
  //
  //  if (!length) {
  //    file->status = ToxFile::FINISHED;
  //    if (file->fileKind == TOX_FILE_KIND_AVATAR) {
  //      QPixmap pic;
  //      pic.loadFromData(file->avatarData);
  //      if (!pic.isNull()) {
  //        qDebug() << "Got" << file->avatarData.size()
  //                 << "bytes of avatar data from" << friendId;
  //        emit core->friendAvatarChanged(core->getFriendPublicKey(friendId),
  //                                       file->avatarData);
  //      }
  //    } else {
  //      emit coreFile->fileTransferFinished(*file);
  //      emit coreFile->fileDownloadFinished(file->filePath);
  //    }
  //    coreFile->removeFile(friendId, fileId);
  //    return;
  //  }
  //
  //  if (file->fileKind == TOX_FILE_KIND_AVATAR) {
  //    file->avatarData.append((char *)data, length);
  //  } else {
  //    file->file->write((char *)data, length);
  //  }
  //  file->bytesSent += length;
  //  file->hashGenerator->addData((const char *)data, length);
  //
  //  if (file->fileKind != TOX_FILE_KIND_AVATAR) {
  //    emit coreFile->fileTransferInfo(*file);
  //  }
}

void CoreFile::onFileRecvChunk(const QString &friendId, const QString &fileId, int seq, const std::string &chunk) {
  ToxFile *file = findFile(friendId, fileId);
  if (!file) {
    qWarning("onFileControlCallback: No such file in queue");
    return;
  }
    if (file->bytesSent > file->filesize) {
      qWarning("onFileRecvChunkCallback: Received a chunk out-of-order, aborting transfer");
      if (file->fileKind != TOX_FILE_KIND_AVATAR) {
        file->status = ToxFile::CANCELED;
        emit fileTransferCancelled(*file);
      }

//    取消传输
      tox->cancelFile(fileId);
      removeFile(friendId, fileId);
      return;
    }

    QByteArray buf = QByteArray::fromStdString(chunk);
    file->file->write(buf);
    file->hashGenerator->addData(buf);
    file->bytesSent += buf.size();
    qDebug() << "Received bytes" << buf.size() << "/"<<file->filesize;
    emit fileTransferInfo(*file);
}

void CoreFile::onFileRecvFinished(const QString &friendId, const QString &fileId) {
    ToxFile *file = findFile(friendId, fileId);
    if (!file) {
      qWarning("onFileControlCallback: No such file in queue");
      return;
    }

    file->status = ToxFile::FINISHED;

    lib::messenger::FileHandler::File f = buildHandlerFile( file);
    tox->finishFileTransfer(friendId, f);

    emit fileTransferFinished(*file);
    emit fileDownloadFinished(file->filePath);

    removeFile(friendId, fileId);
}

void CoreFile::onConnectionStatusChanged(QString friendId,
                                         Status::Status state) {
  //  bool isOffline = state == Status::Status::Offline;
  //  // TODO: Actually resume broken file transfers
  //  // We need to:
  //  // - Start a new file transfer with the same 32byte file ID with toxcore
  //  // - Seek to the correct position again
  //  // - Update the fileNum in our ToxFile
  //  // - Update the users of our signals to check the 32byte tox file ID, not
  //  the
  //  // uint32_t file_num (fileId)
  //  ToxFile::FileStatus status =
  //      !isOffline ? ToxFile::TRANSMITTING : ToxFile::BROKEN;
  //  for (QString key : fileMap.keys()) {
  //    if (key >> 32 != friendId)
  //      continue;
  //    fileMap[key].status = status;
  //    emit fileTransferBrokenUnbroken(fileMap[key], isOffline);
  //  }
}

inline lib::messenger::FileHandler::File CoreFile::buildHandlerFile(const ToxFile *toxFile) {
    return {toxFile->fileNum, toxFile->fileName, toxFile->sId, toxFile->filePath, toxFile->filesize};
}