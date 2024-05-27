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
#include "toxpk.h"
#include <QFile>
#include <QRegularExpression>
#include <base/jsons.h>
#include <lib/messenger/messenger.h>



#define TOX_HEX_ID_LENGTH 2 * TOX_ADDRESS_SIZE

/**
 * @file corestructs.h
 * @brief Some headers use Core structs but don't need to include all of core.h
 *
 * They should include this file directly instead to reduce compilation times
 *
 * @var uint8_t FileStatus::fileKind
 * @brief Data file (default) or avatar
 */


FileInfo::FileInfo(
                   const QString &sId,
                   const QString &id,
                   const QString &fileName,
                   const QString &filePath,
                   quint64 fileSize,
                   quint64 bytesSent,
                   FileStatus status,
                   FileDirection direction)
    : sId{sId}, fileId(id), fileName(fileName), filePath(filePath)
    , fileSize(fileSize), bytesSent(bytesSent), status(status), direction(direction)
{

}


void FileInfo::parse(const QString &json)
{
    auto doc = Jsons::toJSON(json.toUtf8());
    auto obj = doc.object();
    fileId = obj.value("id").toString();
    fileName = obj.value("name").toString();
    filePath = obj.value("path").toString();
    fileSize = obj.value("size").toInt();
    status = (FileStatus)obj.value("status").toInt();
    direction = (FileDirection)obj.value("direction").toInt();
}


ToxFile::ToxFile(const QString &sender,
                 const QString &friendId,
                 QString sId_,
                 QString fileId_,
                 QString filename_,
                 QString filePath_,
                 quint64 fileSize_,
                 quint64 bytesSent,
                 FileStatus status,
                 FileDirection direction)
    : FileInfo(sId_, fileId_, filename_,
               filePath_, fileSize_,
               bytesSent, status, direction)
    ,file(new QFile(filePath_)), sender{sender}, receiver{friendId}
{
}

ToxFile::ToxFile(const QString &sender, const QString &friendId, const lib::messenger::File &file)
    :FileInfo(file.sId, file.id, file.name, file.path, file.size, 0,
              (FileStatus)file.status,
              (FileDirection)file.direction)
    ,file(new QFile(file.path)), sender{sender},
      receiver{friendId}
{

}

ToxFile::ToxFile(const FileInfo &fi): FileInfo(fi)
{

}

bool ToxFile::operator==(const ToxFile &other) const {
  return (fileId == other.fileId);
}

bool ToxFile::operator!=(const ToxFile &other) const {
  return !(*this == other);
}

void ToxFile::setFilePath(QString path) {
  filePath = path;
  file->setFileName(path);
}

bool ToxFile::open(bool write) {
  return write ? file->open(QIODevice::ReadWrite)
               : file->open(QIODevice::ReadOnly);
}

lib::messenger::File ToxFile::toIMFile()
{
    return lib::messenger::File{fileId, sId, fileName,   filePath,  fileSize};
}

const QString& ToxFile::getFriendId() const
{
    return direction == FileDirection::RECEIVING ? sender : receiver;
}



