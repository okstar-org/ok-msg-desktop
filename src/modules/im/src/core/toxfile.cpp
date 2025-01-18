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

#include "src/core/toxfile.h"
#include <base/jsons.h>
#include <lib/messenger/messenger.h>
#include <QFile>
#include <QRegularExpression>
#include "base/basic_types.h"
#include "src/model/FriendId.h"

FileInfo::FileInfo(const QString& sId,
                   const QString& id,
                   const QString& fileName,
                   const QString& filePath,
                   quint64 fileSize,
                   quint64 bytesSent,
                   FileStatus status,
                   FileDirection direction)
        : sId{sId}
        , fileId(id)
        , fileName(fileName)
        , filePath(filePath)
        , fileSize(fileSize)
        , bytesSent(bytesSent)
        , status(status)
        , direction(direction) {}

QString FileInfo::json() const {
    return QString("{\"id\":\"%1\", \"name\":\"%2\", "
                   "\"path\":\"%3\", \"size\":%4, "
                   "\"status\":%5, \"direction\":%6, "
                   "\"sId\":\"%7\"}")
            .arg(fileId)
            .arg(fileName)
            .arg(filePath)
            .arg(fileSize)
            .arg((int)status)
            .arg((int)direction)
            .arg(sId);
}

void FileInfo::parse(const QString& json) {
    auto doc = ok::base::Jsons::toJSON(json.toUtf8());
    auto obj = doc.object();
    fileId = obj.value("id").toString();
    fileName = obj.value("name").toString();
    filePath = obj.value("path").toString();
    sId = obj.value("sid").toString();
    fileSize = obj.value("size").toInt();
    status = (FileStatus)obj.value("status").toInt();
    direction = (FileDirection)obj.value("direction").toInt();
}

ToxFile::ToxFile(const QString& sender,
                 const QString& friendId,
                 const QString& sId_,
                 const QString& fileId_,
                 const QString& filename_,
                 const QString& filePath_,
                 quint64 fileSize_,
                 quint64 bytesSent,
                 FileStatus status,
                 FileDirection direction)
        : FileInfo(sId_, fileId_, filename_, filePath_, fileSize_, bytesSent, status, direction)
        , file(new QFile(filePath_))
        , sender{sender}
        , receiver{friendId}
        , timestamp(QDateTime::currentDateTime()) {}

ToxFile::ToxFile(const QString& sender, const QString& friendId, const QString& msgId,
                 const lib::messenger::File& file)
        : FileInfo(qstring(file.sId),  qstring(file.id), qstring(file.name),qstring(file.path), file.size, 0, (FileStatus)file.status,
                   (FileDirection)file.direction)
        , file(new QFile(qstring(file.path)))
        , sender{sender}
        , receiver{friendId}
        , timestamp(QDateTime::currentDateTime()) {}

ToxFile::ToxFile(const FileInfo& fi) : FileInfo(fi), file(new QFile(fi.filePath)) {}

ToxFile::~ToxFile() {}

bool ToxFile::operator==(const ToxFile& other) const { return (fileId == other.fileId); }

bool ToxFile::operator!=(const ToxFile& other) const { return !(*this == other); }

void ToxFile::setFilePath(QString path) {
    filePath = path;
    file->setFileName(path);
}

bool ToxFile::open(bool write) {
    return write ? file->open(QIODevice::ReadWrite) : file->open(QIODevice::ReadOnly);
}

lib::messenger::File ToxFile::toIMFile() {
    return lib::messenger::File{fileId.toStdString(), sId.toStdString(), fileName.toStdString(), filePath.toStdString(), fileSize};
}

const QString& ToxFile::getFriendId() const {
    return direction == FileDirection::RECEIVING ? sender : receiver;
}
