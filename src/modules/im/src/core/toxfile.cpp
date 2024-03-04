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
#include <QFile>
#include <QRegularExpression>
#include <lib/messenger/messenger.h>
//#include <tox/tox.h>


#define TOX_HEX_ID_LENGTH 2 * TOX_ADDRESS_SIZE

/**
 * @file corestructs.h
 * @brief Some headers use Core structs but don't need to include all of core.h
 *
 * They should include this file directly instead to reduce compilation times
 *
 * @var uint8_t ToxFile::fileKind
 * @brief Data file (default) or avatar
 */

/**
 * @brief ToxFile constructor
 */
ToxFile::ToxFile(QString sId_,
                 QString friendId_,
                 QString fileNum_,
                 QString filename_,
                 QString filePath_,
                 FileDirection Direction)
    : fileKind{0},
      sId(sId_),
      friendId(friendId_),
      fileNum(fileNum_),
      fileName{filename_},
      filePath{filePath_},
      file{new QFile(filePath_)},
      bytesSent{0},
      filesize{0},
      status{INITIALIZING},
      direction{Direction} {}

bool ToxFile::operator==(const ToxFile &other) const {
  return (fileNum == other.fileNum) && (friendId == other.friendId) &&
         (direction == other.direction);
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
