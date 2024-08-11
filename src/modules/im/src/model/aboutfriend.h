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

#ifndef ABOUT_FRIEND_H
#define ABOUT_FRIEND_H

#include "iaboutfriend.h"
#include "src/base/interface.h"
#include "src/persistence/ifriendsettings.h"

#include <QObject>

class Friend;
class IFriendSettings;

class AboutFriend : public QObject, public IAboutFriend {
    Q_OBJECT

public:
    AboutFriend(const Friend* f, IFriendSettings* const settings);
    const Friend* getFriend() const override { return f; }
    QString getName() const override;
    const QString& getAlias() const override;

    QString getStatusMessage() const override;
    FriendId getPublicKey() const override;

    QPixmap getAvatar() const override;

    QString getNote() const override;
    void setNote(const QString& note) override;

    QString getAutoAcceptDir() const override;
    void setAutoAcceptDir(const QString& path) override;

    IFriendSettings::AutoAcceptCallFlags getAutoAcceptCall() const override;
    void setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag) override;

    bool getAutoGroupInvite() const override;
    void setAutoGroupInvite(bool enabled) override;

    bool clearHistory() override;
    bool isHistoryExistence() override;

    SIGNAL_IMPL(AboutFriend, nameChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, statusChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, publicKeyChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, avatarChanged, const QPixmap&)
    SIGNAL_IMPL(AboutFriend, noteChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, autoAcceptDirChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, autoAcceptCallChanged, IFriendSettings::AutoAcceptCallFlags)
    SIGNAL_IMPL(AboutFriend, autoGroupInviteChanged, bool)

private:
    const Friend* const f;
    IFriendSettings* const settings;
};

#endif  // ABOUT_FRIEND_H
