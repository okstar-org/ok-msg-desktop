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

#include "aboutfriend.h"

#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/ifriendsettings.h"
#include "src/persistence/profile.h"

AboutFriend::AboutFriend(const Friend* f, IFriendSettings* const s) : f{f}, settings{s} {
    s->connectTo_contactNoteChanged(
            this, [this](const FriendId& pk, const QString& note) { emit noteChanged(note); });
    s->connectTo_autoAcceptCallChanged(
            this, [this](const FriendId& pk, IFriendSettings::AutoAcceptCallFlags flag) {
                emit autoAcceptCallChanged(flag);
            });
    s->connectTo_autoAcceptDirChanged(this, [this](const FriendId& pk, const QString& dir) {
        emit autoAcceptDirChanged(dir);
    });
    s->connectTo_autoGroupInviteChanged(
            this, [this](const FriendId& pk, bool enable) { emit autoGroupInviteChanged(enable); });
}

QString AboutFriend::getName() const { return f->getName(); }

const QString& AboutFriend::getAlias() const { return f->getAlias(); }

QString AboutFriend::getStatusMessage() const { return f->getStatusMessage(); }

FriendId AboutFriend::getPublicKey() const { return f->getPublicKey(); }

QPixmap AboutFriend::getAvatar() const { return f->getAvatar(); }

QString AboutFriend::getNote() const {
    const FriendId pk = f->getPublicKey();
    return settings->getContactNote(pk);
}

void AboutFriend::setNote(const QString& note) {
    const FriendId pk = f->getPublicKey();
    settings->setContactNote(pk, note);
    settings->saveFriendSettings(pk);
}

QString AboutFriend::getAutoAcceptDir() const {
    const FriendId pk = f->getPublicKey();
    return settings->getAutoAcceptDir(pk);
}

void AboutFriend::setAutoAcceptDir(const QString& path) {
    const FriendId pk = f->getPublicKey();
    settings->setAutoAcceptDir(pk, path);
    settings->saveFriendSettings(pk);
}

IFriendSettings::AutoAcceptCallFlags AboutFriend::getAutoAcceptCall() const {
    const FriendId pk = f->getPublicKey();
    return settings->getAutoAcceptCall(pk);
}

void AboutFriend::setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag) {
    const FriendId pk = f->getPublicKey();
    settings->setAutoAcceptCall(pk, flag);
    settings->saveFriendSettings(pk);
}

bool AboutFriend::getAutoGroupInvite() const {
    const FriendId pk = f->getPublicKey();
    return settings->getAutoGroupInvite(pk);
}

void AboutFriend::setAutoGroupInvite(bool enabled) {
    const FriendId pk = f->getPublicKey();
    settings->setAutoGroupInvite(pk, enabled);
    settings->saveFriendSettings(pk);
}

bool AboutFriend::clearHistory() {
    const FriendId pk = f->getPublicKey();
    History* const history = Nexus::getProfile()->getHistory();
    if (history) {
        history->removeFriendHistory(pk.toString());
        return true;
    }

    return false;
}

bool AboutFriend::isHistoryExistence() {
    auto profile = Nexus::getProfile();
    auto core = profile->getCore();

    History* const history = Nexus::getProfile()->getHistory();
    if (history) {
        const FriendId pk = f->getPublicKey();
        return history->historyExists(core->getSelfId(), pk);
    }

    return false;
}
