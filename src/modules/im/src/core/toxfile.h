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
#include <QMetaType>
#include <QString>
#include <memory>

class QFile;
class QTimer;

namespace lib::messenger {
class File;
}

// Note do not change values, these are directly inserted into the DB in their
// current form, changing order would mess up database state!
enum class FileStatus {
  INITIALIZING = 0,
  PAUSED = 1,
  TRANSMITTING = 2,
  BROKEN = 3,
  CANCELED = 4,
  FINISHED = 5,
};

// Note do not change values, these are directly inserted into the DB in their
// current form (can add fields though as db representation is an int)
enum class FileDirection : bool {
  SENDING = 0,
  RECEIVING = 1,
};

struct FileInfo {
public:
    FileInfo()=default;
    FileInfo(const QString &friendId,
             const QString &sId,
             const QString& id,
             const QString &fileName,
             const QString & filePath,
             quint64 fileSize,
             quint64 bytesSent,
             FileStatus status,
             FileDirection direction
            );
    QString friendId;
    //sessionId
    QString sId;
    //uuid
    QString fileId;
    QString fileName;
    QString filePath;
    quint64 fileSize;
    quint64 bytesSent;
    FileStatus status = FileStatus::INITIALIZING;
    FileDirection direction;

    QString sha256;

public:
    QString json() const;

    void parse(const QString &json);
};
Q_DECLARE_METATYPE(FileInfo);


struct ToxFile : public FileInfo {
  ToxFile() = default;
  ToxFile(const QString &friendId,
          QString sId,
          QString FileId,
          QString FileName,
          QString filePath,
          quint64 fileSize_,
          quint64 bytesSent,
          FileStatus status,
          FileDirection Direction);

  ToxFile(const QString &friendId, const lib::messenger::File &file);
  ToxFile(const FileInfo& fi);

  bool operator==(const ToxFile &other) const;
  bool operator!=(const ToxFile &other) const;

  void setFilePath(QString path);
  bool open(bool write);

  lib::messenger::File toIMFile();

  QString toString() const;

  std::shared_ptr<QFile> file;
  std::shared_ptr<QCryptographicHash> hashGenerator = std::make_shared<QCryptographicHash>(QCryptographicHash::Sha256);
};

Q_DECLARE_METATYPE(ToxFile);

#endif // CORESTRUCTS_H
