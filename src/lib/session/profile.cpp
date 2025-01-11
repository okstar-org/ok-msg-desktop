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
#include "base/hashs.h"
// #include "base/images.h"
#include "gui.h"
#include "lib/storage/StorageManager.h"
#include "lib/storage/cache/CacheManager.h"
#include "lib/storage/settings/OkSettings.h"
#include "profile.h"


namespace lib::session {

static QString FILE_PROFILE_EXT = ".profile";

/**
 * @class Profile
 * @brief Manages user profiles.
 *
 * @var bool Profile::newProfile
 * @brief True if this is a newly created profile, with no .tox save file yet.
 *
 * @var bool Profile::isRemoved
 * @brief True if the profile has been removed by remove().
 */
Profile::Profile(storage::StorageManager* sm, const AuthSession* authSession, QObject* parent)
        : QObject(parent), authSession(authSession), storageManager(sm) {
    qDebug() << __func__;
}

Profile::~Profile() {
    qDebug() << __func__;
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
        //        if (!inifiles.contains(toxfile)) {
        //            Nexus::getProfile()->getSettings()->createPersonal(toxfile);
        //        }
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

    //    if (isRemoved) {
    //        qWarning() << "Profile " << username << " is already removed!";
    //        return {};
    //    }
    //    isRemoved = true;
    //
    //    qDebug() << "Removing profile" << username;
    //    for (int i = 0; i < profiles.size(); ++i) {
    //        if (profiles[i] == username) {
    //            profiles.removeAt(i);
    //            i--;
    //        }
    //    }
    //    QString path = Nexus::getProfile()->getSettings()->getSettingsDirPath() + username;
    //    ProfileLocker::unlock();
    //
    //    QFile profileMain{path + ".tox"};
    //    QFile profileConfig{path + ".ini"};
    //
    //    QStringList ret;
    //
    //    if (!profileMain.remove() && profileMain.exists()) {
    //        ret.push_back(profileMain.fileName());
    //        qWarning() << "Could not remove file " << profileMain.fileName();
    //    }
    //    if (!profileConfig.remove() && profileConfig.exists()) {
    //        ret.push_back(profileConfig.fileName());
    //        qWarning() << "Could not remove file " << profileConfig.fileName();
    //    }
    //
    //    QString dbPath = getDbPath(username);
    //    if (database && database->isOpen() && !database->remove() && QFile::exists(dbPath)) {
    //        ret.push_back(dbPath);
    //        qWarning() << "Could not remove file " << dbPath;
    //    }
    //
    //    history.reset();
    //    database.reset();
    //    return ret;
}

const QString& Profile::getUsername() const {
    return authSession->getSignInInfo().username;
}

const SignInInfo& Profile::getSignIn() const {
    return authSession->getSignInInfo();
}

/**
 *
 * @brief Write the .tox save, encrypted if needed.
 * @param data Byte array of profile save.
 * @return true if successfully saved, false otherwise
 * @warning Invalid on deleted profiles.
 */
// bool Profile::saveToxSave(QByteArray data) {
//     assert(!isRemoved);
//     ProfileLocker::assertLock();
//     assert(ProfileLocker::getCurLockName() == username);
//
//     QString path = Nexus::getProfile()->getSettings()->getSettingsDirPath() + username + ".tox";
//     qDebug() << "Saving tox save to " << path;
//     QSaveFile saveFile(path);
//     if (!saveFile.open(QIODevice::WriteOnly)) {
//         qCritical() << "Tox save file " << path << " couldn't be opened";
//         return false;
//     }
//
//     if (encrypted) {
//         data = passkey->encrypt(data);
//         if (data.isEmpty()) {
//             qCritical() << "Failed to encrypt, can't save!";
//             saveFile.cancelWriting();
//             return false;
//         }
//     }
//
//     saveFile.write(data);
//
//     // check if everything got written
//     if (saveFile.flush()) {
//         saveFile.commit();
//     } else {
//         saveFile.cancelWriting();
//         qCritical() << "Failed to write, can't save!";
//         return false;
//     }
//     return true;
// }

/**
 * @brief Gets the path of the avatar file cached by this profile and
 * corresponding to this owner ID.
 * @param owner Path to avatar of friend with this PK will returned.
 * @param forceUnencrypted If true, return the path to the plaintext file even
 * if this is an encrypted profile.
 * @return Path to the avatar.
 */
// QString Profile::avatarPath(const QString& owner, bool forceUnencrypted) {
//     auto base = Nexus::getProfile()->getSettings()->getSettingsDirPath();
//     return base + "avatars/" + owner + ".png";
// }

/**
 * @brief Get a contact's avatar from cache.
 * @param owner IMFriend PK to load avatar.
 * @return Avatar as QPixmap.
 */
// QPixmap Profile::loadAvatar(const QString& owner) {
//
//     auto& cm = storageManager->getCacheManager();
//      cm.loadAvatarData(owner);
//
//     QPixmap pixmap;
//     const QByteArray avatarData = loadAvatarData(owner);
//     if (!avatarData.isEmpty()) {
//         pixmap.loadFromData(avatarData);
//     }
//     return pixmap;
// }

/**
 * @brief Get a contact's avatar from cache.
 * @param owner IMFriend PK to load avatar.
 * @return Avatar as QByteArray.
 */
QByteArray Profile::loadAvatarData(const QString& owner) {
    auto cm = storageManager->getCacheManager();
    return cm->loadAvatarData(owner);
}

bool Profile::setAvatar(QByteArray& pic) {
    if (pic.isEmpty()) {
        qWarning() << "empty picture!";
        return false;
    }

    auto cm = storageManager->getCacheManager();
    bool ok = cm->saveAvatarData(selfId, pic);
    if (ok) emit selfAvatarChanged(pic);
    return ok;
}

/**
 * @brief Removes our own avatar.
 */
bool Profile::removeAvatar() {
    auto cm = storageManager->getCacheManager();
    return cm->deleteAvatarData(getUsername());
}

/**
 * @brief Sets a friends avatar
 * @param pic Picture to use as avatar, if empty an Identicon will be used
 * depending on settings
 * @param owner pk of friend
 */
// void Profile::setFriendAvatar(const FriendId& owner, const QByteArray& avatarData) {
//     QPixmap pixmap;
//     if (!avatarData.isEmpty()) {
//         bool loaded = ok::base::Images::putToPixmap(avatarData, pixmap);
//         if (!loaded) {
//             return;
//         }
//         emit friendAvatarSet(owner, pixmap);
//     } else if (Nexus::getProfile()->getSettings().getShowIdenticons()) {
//         pixmap = QPixmap::fromImage(Identicon(owner.getByteArray()).toImage(32));
//         emit friendAvatarSet(owner, pixmap);
//     } else {
//         pixmap.load(":/img/contact_dark.svg");
//         emit friendAvatarRemoved(owner);
//     }
//     friendAvatarChanged(owner, pixmap);
//     saveFriendAvatar(owner, avatarData);
// }

/**
 * @brief Adds history message about friendship request attempt if history is
 * enabled
 * @param friendPk Pk of a friend which request is destined to
 * @param message Friendship request message
 */
// void Profile::onRequestSent(const FriendId& friendPk, const QString& message) {
//     if (!isHistoryEnabled()) {
//         return;
//     }
//
//     const QString pkStr = friendPk.toString();
//     const QString inviteStr = Core::tr("/me offers friendship, \"%1\"").arg(message);
//     const QString selfStr = core->getSelfId().toString();
//     const QDateTime datetime = QDateTime::currentDateTime();
//     const QString name = core->getUsername();
//     //  history->addNewMessage(pkStr, inviteStr, selfStr, datetime, true, name);
// }

/**
 * @brief Get the tox hash of a cached avatar.
 * @param owner IMFriend PK to get hash.
 * @return Avatar tox hash.
 */
QByteArray Profile::getFriendAvatarHash(const QString& owner) {
    auto cm = storageManager->getCacheManager();
    return cm->getAvatarHash(owner);
}

/**
 * @brief Removes friend avatar.
 */
bool Profile::removeFriendAvatar(const QString& owner) {
    auto cm = storageManager->getCacheManager();
    return cm->deleteAvatarData(owner);
}

bool Profile::saveFriendAvatar(const QString& owner, const QByteArray& avatar) {
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

/**
 * @brief Checks, if profile has a password.
 * @return True if we have a password set (doesn't check the actual file on
 * disk).
 */
// bool Profile::isEncrypted() const {
//     return encrypted;
// }

/**
 * @brief Checks if profile is encrypted.
 * @note Checks the actual file on disk.
 * @param name Profile name.
 * @return True if profile is encrypted, false otherwise.
 */
// bool Profile::isEncrypted(QString name) {
//   uint8_t data[TOX_PASS_ENCRYPTION_EXTRA_LENGTH] = {0};
//   QString path = Nexus::getProfile()->getSettings().getSettingsDirPath() + name +
//   ".tox"; QFile saveFile(path); if (!saveFile.open(QIODevice::ReadOnly)) {
//     qWarning() << "Couldn't open tox save " << path;
//     return false;
//   }
//
//   saveFile.read((char *)data, TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
//   saveFile.close();
//
//   return tox_is_data_encrypted(data);
//    return false;
//}

/**
 * @brief Tries to rename the profile.
 * @param newName New name for the profile.
 * @return False on error, true otherwise.
 */
// bool Profile::rename(QString newName) {
//     QString path = Nexus::getProfile()->getSettings().getSettingsDirPath() + username,
//             newPath = Nexus::getProfile()->getSettings().getSettingsDirPath() + newName;
//
//     if (!ProfileLocker::lock(newName)) {
//         return false;
//     }
//
//     QFile::rename(path + ".tox", newPath + ".tox");
//     QFile::rename(path + ".ini", newPath + ".ini");
//     if (database) {
//         database->rename(newName);
//     }
//
//     auto& qs = lib::settings::OkSettings::getInstance();
//     bool resetAutorun = qs.getAutorun();
//     qs.setAutorun(false);
//     qs.setCurrentProfile(newName);
//     if (resetAutorun) {
//         qs.setAutorun(true);  // fixes -p flag in autostart command line
//     }
//
//     username = newName;
//     return true;
// }

// const ToxEncrypt* Profile::getPasskey() const {
//     return passkey.get();
// }

/**
 * @brief Changes the encryption password and re-saves everything with it
 * @param newPassword Password for encryption, if empty profile will be
 * decrypted.
 * @param oldPassword Supply previous password if already encrypted or empty
 * QString if not yet encrypted.
 * @return Empty QString on success or error message on failure.
 */
// QString Profile::setPassword(const QString& newPassword) {
//     if (newPassword.isEmpty()) {
//         return {};
//     }
//
//     core->setPassword(newPassword);
//
//     // apply new encryption
//     onSaveToxSave();
//
//     QString error{};
//
//     QByteArray avatar = loadAvatarData(core->getSelfPeerId().getPublicKey());
//     saveFriendAvatar(core->getSelfPeerId().getPublicKey(), avatar);
//
//     return error;
// }

/**
 * @brief Retrieves the path to the database file for a given profile.
 * @param profileName Profile name.
 * @return Path to database.
 */
// QString Profile::getDbPath(const QString& profileName) {
//     return Nexus::getProfile()->getSettings().getSettingsDirPath() + profileName + ".db";
// }

}  // namespace lib::session
