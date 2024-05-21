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

#ifndef CORESTRUCTS_H
#define CORESTRUCTS_H

#include "src/core/toxfilepause.h"

#include <QCryptographicHash>
#include <QString>
#include <memory>


class QFile;
class QTimer;

struct ToxFile {
  // Note do not change values, these are directly inserted into the DB in their
  // current form, changing order would mess up database state!
  enum FileStatus {
    INITIALIZING = 0,
    PAUSED = 1,
    TRANSMITTING = 2,
    BROKEN = 3,
    CANCELED = 4,
    FINISHED = 5,
  };

  // Note do not change values, these are directly inserted into the DB in their
  // current form (can add fields though as db representation is an int)
  enum FileDirection : bool {
    SENDING = 0,
    RECEIVING = 1,
  };

  ToxFile() = default;
  ToxFile(QString sId,
          QString FriendId,
          QString FileNum,
          QString FileName,
          QString filePath,
          FileDirection Direction);

  bool operator==(const ToxFile &other) const;
  bool operator!=(const ToxFile &other) const;

  void setFilePath(QString path);
  bool open(bool write);

  uint8_t fileKind;
  QString sId;
  QString friendId;
  QString fileNum;
  QString fileName;
  QString filePath;
  quint64 bytesSent;
  quint64 filesize;
  FileStatus status;
  FileDirection direction;
  QByteArray avatarData;
  QByteArray resumeFileId;

  std::shared_ptr<QFile> file;
  std::shared_ptr<QCryptographicHash> hashGenerator =
      std::make_shared<QCryptographicHash>(QCryptographicHash::Sha256);
  ToxFilePause pauseStatus;
};

#endif // CORESTRUCTS_H
