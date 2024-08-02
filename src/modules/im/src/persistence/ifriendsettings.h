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

#ifndef I_FRIEND_SETTINGS_H
#define I_FRIEND_SETTINGS_H

#include "src/base/interface.h"

#include <QFlag>
#include <QObject>

#include <src/core/contactid.h>

class FriendId;

class IFriendSettings {
public:
    enum class AutoAcceptCall { None = 0x00, Audio = 0x01, Video = 0x02, AV = Audio | Video };
    Q_DECLARE_FLAGS(AutoAcceptCallFlags, AutoAcceptCall)

    virtual ~IFriendSettings() = default;

    virtual QString getContactNote(const FriendId& pk) const = 0;
    virtual void setContactNote(const FriendId& pk, const QString& note) = 0;

    virtual QString getAutoAcceptDir(const FriendId& pk) const = 0;
    virtual void setAutoAcceptDir(const FriendId& pk, const QString& dir) = 0;

    virtual AutoAcceptCallFlags getAutoAcceptCall(const FriendId& pk) const = 0;
    virtual void setAutoAcceptCall(const FriendId& pk, AutoAcceptCallFlags accept) = 0;

    virtual bool getAutoGroupInvite(const FriendId& pk) const = 0;
    virtual void setAutoGroupInvite(const FriendId& pk, bool accept) = 0;

    virtual QString getFriendAlias(const ContactId& pk) const = 0;
    virtual void setFriendAlias(const FriendId& pk, const QString& alias) = 0;

    virtual int getFriendCircleID(const FriendId& pk) const = 0;
    virtual void setFriendCircleID(const FriendId& pk, int circleID) = 0;

    virtual QDateTime getFriendActivity(const FriendId& pk) const = 0;
    virtual void setFriendActivity(const FriendId& pk, const QDateTime& date) = 0;

    virtual void saveFriendSettings(const FriendId& pk) = 0;
    virtual void removeFriendSettings(const FriendId& pk) = 0;

signals:
    DECLARE_SIGNAL(autoAcceptCallChanged, const FriendId& pk, AutoAcceptCallFlags accept);
    DECLARE_SIGNAL(autoGroupInviteChanged, const FriendId& pk, bool accept);
    DECLARE_SIGNAL(autoAcceptDirChanged, const FriendId& pk, const QString& dir);
    DECLARE_SIGNAL(contactNoteChanged, const FriendId& pk, const QString& note);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IFriendSettings::AutoAcceptCallFlags)
#endif  // I_FRIEND_SETTINGS_H
