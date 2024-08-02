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

#include "profileinfo.h"
#include "src/core/core.h"
#include "src/model/toxclientstandards.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFile>
#include <QImageReader>

/**
 * @class ProfileInfo
 * @brief Implement interface, that provides invormation about self profile.
 * Also, provide methods to work with profile file.
 * @note Should be used only when QAppliaction constructed.
 */

/**
 * @brief ProfileInfo constructor.
 * @param core Pointer to Tox Core.
 * @param profile Pointer to Profile.
 * @note All pointers parameters shouldn't be null.
 */
ProfileInfo::ProfileInfo(Core* core, Profile* profile) : profile{profile}, core{core} {
    connect(core, &Core::idSet, this, &ProfileInfo::idChanged);
    connect(core, &Core::usernameSet, this, &ProfileInfo::usernameChanged);
    connect(core, &Core::avatarSet, this, &ProfileInfo::avatarChanged);
    connect(core, &Core::statusMessageSet, this, &ProfileInfo::statusMessageChanged);

    //    connectTo_usernameChanged(this,
    //                              [this](const QString& val) {
    //                                profile->rename(val);
    //                              });
}

/**
 * @brief Set a user password for profile.
 * @param password New password.
 * @return True on success, false otherwise.
 */
bool ProfileInfo::setPassword(const QString& password) {
    QString errorMsg = profile->setPassword(password);
    return errorMsg.isEmpty();
}

/**
 * @brief Delete a user password for profile.
 * @return True on success, false otherwise.
 */
bool ProfileInfo::deletePassword() {
    QString errorMsg = profile->setPassword("");
    return errorMsg.isEmpty();
}

/**
 * @brief Check if current profile is encrypted.
 * @return True if encrypted, false otherwise.
 */
bool ProfileInfo::isEncrypted() const { return profile->isEncrypted(); }

/**
 * @brief Copy self ToxId to clipboard.
 */
void ProfileInfo::copyId() const {
    ToxId selfId = core->getSelfPeerId();
    QString txt = selfId.toString();
    QClipboard* clip = QApplication::clipboard();
    clip->setText(txt, QClipboard::Clipboard);
    if (clip->supportsSelection()) {
        clip->setText(txt, QClipboard::Selection);
    }
}

/**
 * @brief Set self user name.
 * @param name New name.
 */
void ProfileInfo::setUsername(const QString& name) { core->setUsername(name); }

void ProfileInfo::setAvatar(const QPixmap& avatar) {
    // set avatar.
    profile->setAvatarOnly(avatar);
}

const QPixmap& ProfileInfo::getAvatar() { return profile->loadAvatar(); }

/**
 * @brief Set self status message.
 * @param status New status message.
 */
void ProfileInfo::setStatusMessage(const QString& status) { core->setStatusMessage(status); }

/**
 * @brief Get name of tox profile file.
 * @return Profile name.
 */
QString ProfileInfo::getUsername() const { return profile->getName(); }

const QString& ProfileInfo::getDisplayName() const { return profile->getDisplayName(); }

/**
 * @brief Remove characters not supported for profile name from string.
 * @param src Source string.
 * @return Sanitized string.
 */
static QString sanitize(const QString& src) {
    QString name = src;
    // these are pretty much Windows banned filename characters
    QList<QChar> banned{'/', '\\', ':', '<', '>', '"', '|', '?', '*'};
    for (QChar c : banned) {
        name.replace(c, '_');
    }

    // also remove leading and trailing periods
    if (name[0] == '.') {
        name[0] = '_';
    }

    if (name.endsWith('.')) {
        name[name.length() - 1] = '_';
    }

    return name;
}

/**
 * @brief Rename profile file.
 * @param name New profile name.
 * @return Result code of rename operation.
 */
IProfileInfo::RenameResult ProfileInfo::renameProfile(const QString& name) {
    QString cur = profile->getName();
    if (name.isEmpty()) {
        return RenameResult::EmptyName;
    }

    QString newName = sanitize(name);

    if (Profile::exists(newName)) {
        return RenameResult::ProfileAlreadyExists;
    }

    if (!profile->rename(name)) {
        return RenameResult::Error;
    }

    return RenameResult::OK;
}

// TODO: Find out what is dangerous?
/**
 * @brief Dangerous way to find out if a path is writable.
 * @param filepath Path to file which should be deleted.
 * @return True, if file writeable, false otherwise.
 */
static bool tryRemoveFile(const QString& filepath) {
    QFile tmp(filepath);
    bool writable = tmp.open(QIODevice::WriteOnly);
    tmp.remove();
    return writable;
}

/**
 * @brief Save profile in custom place.
 * @param path Path to save profile.
 * @return Result code of save operation.
 */
IProfileInfo::SaveResult ProfileInfo::exportProfile(const QString& path) const {
    QString current = profile->getName() + Core::TOX_EXT;
    if (path.isEmpty()) {
        return SaveResult::EmptyPath;
    }

    if (!tryRemoveFile(path)) {
        return SaveResult::NoWritePermission;
    }

    if (!QFile::copy(Settings::getInstance().getSettingsDirPath() + current, path)) {
        return SaveResult::Error;
    }

    return SaveResult::OK;
}

/**
 * @brief Remove profile.
 * @return List of files, which couldn't be removed automaticaly.
 */
QStringList ProfileInfo::removeProfile() {
    QStringList manualDeleteFiles = profile->remove();
    QMetaObject::invokeMethod(&Nexus::getInstance(), "showLogin");
    return manualDeleteFiles;
}

/**
 * @brief Log out from current profile.
 */
void ProfileInfo::logout() {
    qDebug() << __func__;
    emit Nexus::getInstance().destroyProfile(getUsername());
}

void ProfileInfo::exit() {
    qDebug() << __func__;
    emit Nexus::getInstance().exit(getUsername());
}

/**
 * @brief Copy image to clipboard.
 * @param image Image to copy.
 */
void ProfileInfo::copyQr(const QImage& image) const { QApplication::clipboard()->setImage(image); }

/**
 * @brief Save image to file.
 * @param image Image to save.
 * @param path Path to save.
 * @return Result code of save operation.
 */
IProfileInfo::SaveResult ProfileInfo::saveQr(const QImage& image, const QString& path) const {
    QString current = profile->getName() + ".png";
    if (path.isEmpty()) {
        return SaveResult::EmptyPath;
    }

    if (!tryRemoveFile(path)) {
        return SaveResult::NoWritePermission;
    }

    // nullptr - image format same as file extension,
    // 75-quality, png file is ~6.3kb
    if (!image.save(path, nullptr, 75)) {
        return SaveResult::Error;
    }

    return SaveResult::OK;
}

/**
 * @brief Convert QImage to png image.
 * @param pic Picture to convert.
 * @return Byte array with png image.
 */
QByteArray picToPng(const QImage& pic) {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pic.save(&buffer, "PNG");
    buffer.close();
    return bytes;
}

/**
 * @brief Set self avatar.
 * @param path Path to image, which should be the new avatar.
 * @return Code of set avatar operation.
 */
IProfileInfo::SetAvatarResult ProfileInfo::setAvatar(const QString& path) {
    if (path.isEmpty()) {
        return SetAvatarResult::EmptyPath;
    }

    QFile file(path);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        return SetAvatarResult::CanNotOpen;
    }
    QByteArray avatar;
    const auto err = createAvatarFromFile(file, avatar);
    if (err == SetAvatarResult::OK) {
        profile->setAvatar(avatar);
    }
    return err;
}

/**
 * @brief Create an avatar from an image file
 * @param file Image file, which should be the new avatar.
 * @param avatar Output avatar of correct file type and size.
 * @return SetAvatarResult
 */
IProfileInfo::SetAvatarResult ProfileInfo::createAvatarFromFile(QFile& file, QByteArray& avatar) {
    QByteArray fileContents{file.readAll()};
    auto err = byteArrayToPng(fileContents, avatar);
    if (err != SetAvatarResult::OK) {
        return err;
    }

    err = scalePngToAvatar(avatar);
    return err;
}

/**
 * @brief Create a png from image data
 * @param inData byte array from an image file.
 * @param outPng byte array which the png will be written to.
 * @return SetAvatarResult
 */
IProfileInfo::SetAvatarResult ProfileInfo::byteArrayToPng(QByteArray inData, QByteArray& outPng) {
    QBuffer inBuffer{&inData};
    QImageReader reader{&inBuffer};
    QImage image;
    const auto format = reader.format();
    // read whole image even if we're not going to use the QImage, to make sure the image is valid
    if (!reader.read(&image)) {
        return SetAvatarResult::CanNotRead;
    }

    if (format == "png") {
        // FIXME: re-encode the png even though inData is already valid. This strips the metadata
        // since we don't have a good png metadata stripping method currently.
        outPng = picToPng(image);
    } else {
        outPng = picToPng(image);
    }
    return SetAvatarResult::OK;
}

/*
 * @brief Scale a png to an acceptable file size.
 * @param avatar byte array containing the avatar.
 * @return SetAvatarResult
 */
IProfileInfo::SetAvatarResult ProfileInfo::scalePngToAvatar(QByteArray& avatar) {
    // We do a first rescale to 256x256 in case the image was huge, then keep tryng from here
    constexpr int scaleSizes[] = {256, 128, 64, 32};

    for (auto scaleSize : scaleSizes) {
        if (ToxClientStandards::IsValidAvatarSize(avatar.size())) break;
        QImage image;
        image.loadFromData(avatar);
        image = image.scaled(scaleSize, scaleSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        avatar = picToPng(image);
    }

    // If this happens, you're really doing it on purpose.
    if (!ToxClientStandards::IsValidAvatarSize(avatar.size())) {
        return SetAvatarResult::TooLarge;
    }
    return SetAvatarResult::OK;
}

/**
 * @brief Remove self avatar.
 */
void ProfileInfo::removeAvatar() { profile->removeSelfAvatar(); }
