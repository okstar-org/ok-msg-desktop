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

#include <cassert>

#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QSaveFile>

#include "AuthSession.h"
#include "lib/storage/StorageManager.h"
#include "lib/storage/cache/CacheManager.h"
#include "lib/storage/settings/OkSettings.h"
#include "profile.h"


namespace lib::session {

static QString FILE_PROFILE_EXT = ".profile";

/**
 * @class Profile
 * @brief Global user profiles.
 */
Profile::Profile(storage::StorageManager* sm, const AuthSession* authSession, QObject* parent)
        : QObject(parent), authSession(authSession), storageManager(sm) {
    qDebug() << __func__;
    auto& info = authSession->getSignInInfo();
    selfId = ok::base::Jid(info.username, info.host);
    qDebug() << __func__ << "selfId:" << selfId.bare();
    messenger = new lib::messenger::Messenger(info.host.toStdString(), info.username.toStdString(), info.password.toStdString());
    qInfo() << __func__ << "Messenger created";
}

Profile::~Profile() {
    qDebug() << __func__;
    delete messenger;
}

/**
 * @brief Lists all the files in the config dir with a given extension
 * @param extension Raw extension, e.g. "jpeg" not ".jpeg".
 * @return Vector of filenames.
 */
QStringList Profile::getFilesByExt(const QString& extension) {
    QStringList out;
    auto& setDir = storageManager->getDir();

    QDir dir(setDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dir.setNameFilters(QStringList("*." + extension));

    QFileInfoList list = dir.entryInfoList();
    out.reserve(list.size());

    for (auto& file : list) {
        out += file.completeBaseName();
    }
    return out;
}

/**
 * 查询所有profiles
 */
const QStringList& Profile::getAllProfileNames() {
    profiles.clear();
    QStringList files = getFilesByExt(FILE_PROFILE_EXT);
    for (const QString& file : files) {
        profiles.append(file);
    }
    return profiles;
}

bool Profile::exists() {
    auto dir = storageManager->getGlobalSettings();
    auto pf = dir->getGlobalSettingsFile() + QDir::separator() + getUsername() + FILE_PROFILE_EXT;
    return QFile::exists(pf);
}

/**
 * @brief Removes the profile permanently.
 */
bool Profile::remove() {
    auto dir = storageManager->getGlobalSettings();
    auto pf = dir->getGlobalSettingsFile() + QDir::separator() + getUsername() + FILE_PROFILE_EXT;
    return QFile::remove(pf);
}

const QString& Profile::getUsername() const {
    return authSession->getSignInInfo().username;
}

const SignInInfo& Profile::getSignIn() const {
    return authSession->getSignInInfo();
}

/**
 * @brief Get a contact's avatar from cache.
 * @param owner IMFriend PK to load avatar.
 * @return Avatar as QByteArray.
 */
QByteArray Profile::loadAvatarData(const ok::base::Jid& owner) {
    auto cm = storageManager->getCacheManager();
    return cm->loadAvatarData(owner);
}

bool Profile::setAvatar(QByteArray& pic) {
    qDebug() << __func__;
    if (pic.isEmpty()) {
        qWarning() << __func__ << "empty picture!";
    }

    if (pic == avatar) {
        return false;
    }

    avatar = pic;
    emit selfAvatarChanged(pic);

    auto cm = storageManager->getCacheManager();
    return cm->saveAvatarData(selfId, pic);
}

const QByteArray& Profile::getAvatar() {
    if (avatar.isNull() || avatar.isEmpty()) {
        avatar = loadAvatarData(selfId);
    }
    return avatar;
}

/**
 * @brief Removes our own avatar.
 */
bool Profile::removeAvatar() {
    auto cm = storageManager->getCacheManager();
    return cm->deleteAvatarData(selfId);
}



/**
 * @brief Get the tox hash of a cached avatar.
 * @param owner IMFriend PK to get hash.
 * @return Avatar tox hash.
 */
QByteArray Profile::getFriendAvatarHash(const ok::base::Jid& owner) {
    auto cm = storageManager->getCacheManager();
    return cm->getAvatarHash(owner);
}

/**
 * @brief Removes friend avatar.
 */
bool Profile::removeFriendAvatar(const ok::base::Jid& owner) {
    auto cm = storageManager->getCacheManager();
    return cm->deleteAvatarData(owner);
}

bool Profile::saveFriendAvatar(const ok::base::Jid& owner, const QByteArray& avatar) {
    auto cm = storageManager->getCacheManager();
    return cm->saveAvatarData(owner, avatar);
}

storage::StorageManager *Profile::create(const QString &profile) {
    qDebug() << __func__ << profile;
    return storageManager->create(profile);
}

void Profile::setNickname(const QString& nickname_) {
    if (nickname != nickname_) {
        nickname = nickname_;
        emit nickChanged(nickname);
    }
}

messenger::Messenger *Profile::getMessenger()
{
    return messenger;
}
}  // namespace lib::session
