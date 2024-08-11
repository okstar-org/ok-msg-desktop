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

#ifndef I_ABOUT_FRIEND_H
#define I_ABOUT_FRIEND_H

#include "src/base/interface.h"
#include "src/persistence/ifriendsettings.h"

#include <QObject>

class Friend;

class IAboutFriend {
public:
    virtual ~IAboutFriend() = default;

    virtual const Friend* getFriend() const = 0;

    virtual QString getName() const = 0;
    virtual const QString& getAlias() const = 0;

    virtual QString getStatusMessage() const = 0;
    virtual FriendId getPublicKey() const = 0;

    virtual QPixmap getAvatar() const = 0;

    virtual QString getNote() const = 0;
    virtual void setNote(const QString& note) = 0;

    virtual QString getAutoAcceptDir() const = 0;
    virtual void setAutoAcceptDir(const QString& path) = 0;

    virtual IFriendSettings::AutoAcceptCallFlags getAutoAcceptCall() const = 0;
    virtual void setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag) = 0;

    virtual bool getAutoGroupInvite() const = 0;
    virtual void setAutoGroupInvite(bool enabled) = 0;

    virtual bool clearHistory() = 0;
    virtual bool isHistoryExistence() = 0;

    /* signals */
    DECLARE_SIGNAL(nameChanged, const QString&);
    DECLARE_SIGNAL(statusChanged, const QString&);
    DECLARE_SIGNAL(publicKeyChanged, const QString&);

    DECLARE_SIGNAL(avatarChanged, const QPixmap&);
    DECLARE_SIGNAL(noteChanged, const QString&);

    DECLARE_SIGNAL(autoAcceptDirChanged, const QString&);
    DECLARE_SIGNAL(autoAcceptCallChanged, IFriendSettings::AutoAcceptCallFlags);
    DECLARE_SIGNAL(autoGroupInviteChanged, bool);
};

#endif  // I_ABOUT_FRIEND_H
