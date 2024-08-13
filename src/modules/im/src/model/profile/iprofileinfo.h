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

#include "src/base/interface.h"

#include <QObject>

class ToxId;

class IProfileInfo {
public:
    enum class RenameResult { OK, EmptyName, ProfileAlreadyExists, Error };

    enum class SaveResult { OK, EmptyPath, NoWritePermission, Error };

    enum class SetAvatarResult { OK, EmptyPath, CanNotOpen, CanNotRead, TooLarge };
    virtual ~IProfileInfo() = default;

    virtual bool setPassword(const QString& password) = 0;
    virtual bool deletePassword() = 0;
    virtual bool isEncrypted() const = 0;

    virtual void copyId() const = 0;

    virtual void setUsername(const QString& name) = 0;
    virtual QString getUsername() const = 0;
    virtual const QString& getDisplayName() const = 0;

    virtual void setAvatar(const QPixmap& avatar) = 0;
    virtual const QPixmap& getAvatar() = 0;
    virtual void setStatusMessage(const QString& status) = 0;

    virtual RenameResult renameProfile(const QString& name) = 0;
    virtual SaveResult exportProfile(const QString& path) const = 0;
    virtual QStringList removeProfile() = 0;
    virtual void logout() = 0;
    virtual void exit() = 0;

    virtual void copyQr(const QImage& image) const = 0;
    virtual SaveResult saveQr(const QImage& image, const QString& path) const = 0;

    virtual SetAvatarResult setAvatar(const QString& path) = 0;
    virtual void removeAvatar() = 0;

    DECLARE_SIGNAL(idChanged, const ToxId&);
    DECLARE_SIGNAL(usernameChanged, const QString&);
    DECLARE_SIGNAL(avatarChanged, const QString&);
    DECLARE_SIGNAL(statusMessageChanged, const QString&);
};
