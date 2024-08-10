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
#include <QFile>
#include <QMetaType>
#include <QString>
#include <memory>

class QFile;
class QTimer;
class FriendId;


// 不要修改顺序和值
enum class FileStatus {
    INITIALIZING = 0,
    PAUSED = 1,
    TRANSMITTING = 2,
    BROKEN = 3,
    CANCELED = 4,
    FINISHED = 5,
};

// 不要修改顺序和值
enum class FileDirection {
    SENDING = 0,
    RECEIVING = 1,
};

namespace lib::messenger {
struct File;
enum class FileStatus;
enum class FileDirection;
}  // namespace lib::messenger

struct FileInfo {
public:
    FileInfo() = default;
    FileInfo(const QString& sId,
             const QString& id,
             const QString& fileName,
             const QString& filePath,
             quint64 fileSize,
             quint64 bytesSent,
             FileStatus status,
             FileDirection direction);

    // sessionId
    QString sId;
    // uuid
    QString fileId;
    QString fileName;
    QString filePath;
    quint64 fileSize;
    quint64 bytesSent;
    FileStatus status;
    FileDirection direction;
    QString sha256;

public:
    [[nodiscard]] QString json() const;
    void parse(const QString& json);
    void setFilePath(const QString& path) { this->filePath = path; }
};
Q_DECLARE_METATYPE(FileInfo);

struct ToxFile : public FileInfo {
    explicit ToxFile() = default;
    explicit ToxFile(const QString& sender,
            const QString& friendId,
            QString sId,
            QString FileId,
            QString FileName,
            QString filePath,
            quint64 fileSize_,
            quint64 bytesSent,
            FileStatus status,
            FileDirection Direction);

    explicit ToxFile(const QString& sender, const QString& friendId, const lib::messenger::File& file);
    explicit ToxFile(const FileInfo& fi);
    ~ToxFile();

    bool operator==(const ToxFile& other) const;
    bool operator!=(const ToxFile& other) const;

    void setFilePath(QString path);
    bool open(bool write);

    lib::messenger::File toIMFile();

    const QString& getFriendId() const;

    inline QString toString() const {
        return QString("{id:%1, sId:%2, name:%3, path:%4, size:%5}")
                .arg(fileId)
                .arg(sId)
                .arg(fileName)
                .arg(filePath)
                .arg(fileSize);
    }

    QString sender;
    QString receiver;

    std::shared_ptr<QFile> file;
    //  std::shared_ptr<QCryptographicHash> hashGenerator =
    //  std::make_shared<QCryptographicHash>(QCryptographicHash::Sha256);
};

#endif  // CORESTRUCTS_H
