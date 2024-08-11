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

#include <QObject>
#include "iprofileinfo.h"
#include "src/base/interface.h"
#include "src/core/FriendId.h"

class Core;
class QFile;
class QPoint;
class Profile;

class ProfileInfo : public QObject, public IProfileInfo {
    Q_OBJECT
public:
    ProfileInfo(Core* core, Profile* profile);

    bool setPassword(const QString& password) override;
    bool deletePassword() override;
    bool isEncrypted() const override;

    void copyId() const override;

    void setUsername(const QString& name) override;

    void setAvatar(const QPixmap& avatar) override;
    const QPixmap& getAvatar() override;
    void setStatusMessage(const QString& status) override;

    QString getUsername() const override;
    virtual const QString& getDisplayName() const override;

    RenameResult renameProfile(const QString& name) override;
    SaveResult exportProfile(const QString& path) const override;
    QStringList removeProfile() override;
    void logout() override;
    void exit() override;

    void copyQr(const QImage& image) const override;
    SaveResult saveQr(const QImage& image, const QString& path) const override;

    SetAvatarResult setAvatar(const QString& path) override;
    void removeAvatar() override;

    SIGNAL_IMPL(ProfileInfo, idChanged, const ToxId& id)
    SIGNAL_IMPL(ProfileInfo, usernameChanged, const QString& username)
    SIGNAL_IMPL(ProfileInfo, avatarChanged, const QString& avatar)
    SIGNAL_IMPL(ProfileInfo, statusMessageChanged, const QString& message)

private:
    IProfileInfo::SetAvatarResult createAvatarFromFile(QFile& file, QByteArray& avatar);
    IProfileInfo::SetAvatarResult byteArrayToPng(QByteArray inData, QByteArray& outPng);
    IProfileInfo::SetAvatarResult scalePngToAvatar(QByteArray& avatar);
    Profile* const profile;
    Core* const core;

signals:
    void logouted();
};
