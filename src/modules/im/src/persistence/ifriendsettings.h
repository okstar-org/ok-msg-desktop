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

#include "src/model/interface.h"

#include <QObject>
#include <QFlag>

class ToxPk;

class IFriendSettings
{
public:
    enum class AutoAcceptCall
    {
        None = 0x00,
        Audio = 0x01,
        Video = 0x02,
        AV = Audio | Video
    };
    Q_DECLARE_FLAGS(AutoAcceptCallFlags, AutoAcceptCall)

    virtual ~IFriendSettings() = default;

    virtual QString getContactNote(const ToxPk& pk) const = 0;
    virtual void setContactNote(const ToxPk& pk, const QString& note) = 0;

    virtual QString getAutoAcceptDir(const ToxPk& pk) const = 0;
    virtual void setAutoAcceptDir(const ToxPk& pk, const QString& dir) = 0;

    virtual AutoAcceptCallFlags getAutoAcceptCall(const ToxPk& pk) const = 0;
    virtual void setAutoAcceptCall(const ToxPk& pk, AutoAcceptCallFlags accept) = 0;

    virtual bool getAutoGroupInvite(const ToxPk& pk) const = 0;
    virtual void setAutoGroupInvite(const ToxPk& pk, bool accept) = 0;

    virtual QString getFriendAlias(const ToxPk& pk) const = 0;
    virtual void setFriendAlias(const ToxPk& pk, const QString& alias) = 0;

    virtual int getFriendCircleID(const ToxPk& pk) const = 0;
    virtual void setFriendCircleID(const ToxPk& pk, int circleID) = 0;

    virtual QDateTime getFriendActivity(const ToxPk& pk) const = 0;
    virtual void setFriendActivity(const ToxPk& pk, const QDateTime& date) = 0;

    virtual void saveFriendSettings(const ToxPk& pk) = 0;
    virtual void removeFriendSettings(const ToxPk& pk) = 0;

signals:
    DECLARE_SIGNAL(autoAcceptCallChanged, const ToxPk& pk, AutoAcceptCallFlags accept);
    DECLARE_SIGNAL(autoGroupInviteChanged, const ToxPk& pk, bool accept);
    DECLARE_SIGNAL(autoAcceptDirChanged, const ToxPk& pk, const QString& dir);
    DECLARE_SIGNAL(contactNoteChanged, const ToxPk& pk, const QString& note);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IFriendSettings::AutoAcceptCallFlags)
#endif // I_FRIEND_SETTINGS_H
