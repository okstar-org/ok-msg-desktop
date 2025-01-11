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

//
// Created by gaojie on 25-1-9.
//

#include "CacheManager.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QSaveFile>
#include "base/hashs.h"

namespace lib::cache {

static QString AVATAR_EXT = "png";

inline const QString makeAvatarPath(const QDir& path, const QString& owner) {
    return path.path() + QDir::separator() + "avatars" + QDir::separator() + owner + "." +
           AVATAR_EXT;
}

CacheManager::CacheManager(const QDir& path, QObject* parent) : QObject(parent), path(path) {
    qDebug() << __func__ << path;
}
CacheManager::~CacheManager() {
    qDebug() << __func__;
}

QByteArray CacheManager::loadAvatarData(const QString& owner) const {
    if (owner.isEmpty()) {
        qWarning() << "empty owner!";
        return {};
    }

    auto filePath = makeAvatarPath(path, owner);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open file" << path.path();
        return {};
    }

    QByteArray pic = file.readAll();
    if (pic.isEmpty()) {
        qWarning() << "Empty avatar at" << path;
    }

    return pic;
}

bool CacheManager::saveAvatarData(const QString& owner, const QByteArray& buf) {
    if (owner.isEmpty()) {
        qWarning() << "empty owner!";
        return false;
    }
    if (buf.isEmpty()) {
        qWarning() << "empty buffer!";
        return false;
    }

    auto filePath = makeAvatarPath(path, owner);
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << filePath << " couldn't be saved";
        return false;
    }
    file.write(buf);
    return file.commit();
}

QByteArray CacheManager::getAvatarHash(const QString& owner) const {
    return ok::base::Hashs::hash(loadAvatarData(owner), QCryptographicHash::Algorithm::Md5);
}

bool CacheManager::deleteAvatarData(const QString& owner) {
    return QFile::remove(makeAvatarPath(path, owner));
}

}  // namespace lib::cache
