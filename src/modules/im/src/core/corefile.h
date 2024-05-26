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

#ifndef COREFILE_H
#define COREFILE_H

#include "lib/messenger/tox/tox.h"


#include "src/core/core.h"
#include "src/core/toxpk.h"
#include "src/model/status.h"
#include "toxfile.h"

#include "base/compatiblerecursivemutex.h"

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QString>

#include <cstddef>
#include <cstdint>
#include <memory>

#include "lib/messenger/messenger.h"


class CoreFile;

using CoreFilePtr = std::unique_ptr<CoreFile>;

class CoreFile : public QObject, public lib::messenger::FileHandler {
  Q_OBJECT

public:
  void handleAvatarOffer(QString friendId, QString fileId, bool accept);
  static CoreFilePtr makeCoreFile(Core *core, Tox *tox,
                                  CompatibleRecursiveMutex &coreLoopLock);

  void sendFile(QString friendId,
                QString filename,
                QString filePath,
                quint64 filesize,
                quint64 sent=0);

  void sendAvatarFile(QString friendId, const QByteArray &data);
  void pauseResumeFile(QString friendId, QString fileId);
  void cancelFileSend(QString friendId, QString fileId);

  void cancelFileRecv(QString friendId, QString fileId);
  void rejectFileRecvRequest(QString friendId, QString fileId);
  void acceptFileRecvRequest(QString friendId, QString fileId, QString path);

  unsigned corefileIterationInterval();

  /**
   * File handlers
   */

  void onFileRequest(const QString &friendId, const lib::messenger::File& file) override;
  void onFileRecvChunk(const QString &friendId, const QString& fileId, int seq, const std::string &chunk) override;
  void onFileRecvFinished(const QString &friendId, const QString& fileId) override;
  void onFileSendInfo(const QString &friendId, const lib::messenger::File &file, int m_seq,
                      int m_sentBytes, bool end) override;

  void onFileSendAbort(const QString &friendId, const lib::messenger::File &file,
                      int m_sentBytes) override;
  void onFileSendError(const QString &friendId, const lib::messenger::File &file,
                      int m_sentBytes) override;
signals:
  void fileSendStarted(ToxFile file);
  void fileReceiveRequested(ToxFile file);
  void fileTransferAccepted(ToxFile file);
  void fileTransferCancelled(ToxFile file);
  void fileTransferFinished(ToxFile file);
  void fileUploadFinished(const QString &path);
  void fileDownloadFinished(const QString &path);
  void fileTransferPaused(ToxFile file);
  void fileTransferInfo(ToxFile file);
  void fileTransferRemotePausedUnpaused(ToxFile file, bool paused);
  void fileTransferBrokenUnbroken(ToxFile file, bool broken);
  void fileNameChanged(const ToxPk &friendPk);
  void fileSendFailed(QString friendId, const QString &fname);

private:
  CoreFile(Tox *core, CompatibleRecursiveMutex &coreLoopLock);

  ToxFile *findFile(QString friendId, QString fileId);
  const QString& addFile(QString friendId, ToxFile &file);
  void removeFile(QString friendId, QString fileId);

  static QString getFriendKey(const QString& friendId, QString fileId) {
    return  friendId + "-"+ fileId;
  }

  lib::messenger::File buildHandlerFile(const ToxFile* toxFile);

  void connectCallbacks(Tox &tox);
  static void onFileReceiveCallback(Tox *tox, QString friendId, QString fileId,
                                    uint32_t kind, uint64_t filesize,
                                    const uint8_t *fname, size_t fnameLen,
                                    void *vCore);
  static void onFileControlCallback(Tox *tox, QString friendId, QString fileId,
                                    Tox_File_Control control, void *vCore);
  static void onFileDataCallback(Tox *tox, QString friendId, QString fileId,
                                 uint64_t pos, size_t length, void *vCore);
  static void onFileRecvChunkCallback(Tox *tox, QString friendId,
                                      QString fileId, uint64_t position,
                                      const uint8_t *data, size_t length,
                                      void *vCore);

  static QString getCleanFileName(QString filename);

private slots:
  void onConnectionStatusChanged(QString friendId, Status::Status state);

private:
  QHash<QString, ToxFile> fileMap;
  Tox *tox;
  CompatibleRecursiveMutex *coreLoopLock = nullptr;

};

#endif // COREFILE_H
