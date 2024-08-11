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

#include "friend.h"
#include "src/model/status.h"
#include "src/persistence/profile.h"
#include "src/widget/form/chatform.h"

Friend::Friend(const FriendId& friendPk,  //
               bool isFriend,             //
               const QString& userAlias,  //
               const QString& userName)   //
        : Contact(friendPk, userName, userAlias, false)
        ,  //
        id{friendPk}
        ,  //
        hasNewEvents{false}
        ,  //
        friendStatus{Status::Status::Offline}
        ,  //
        isFriend_{isFriend} {
    auto core = Core::getInstance();
    friendStatus = core->getFriendStatus(friendPk.toString());
}

Friend::~Friend() { qDebug() << __func__; }

QString Friend::toString() const { return getId().toString(); }

void Friend::setStatusMessage(const QString& message) {
    if (statusMessage != message) {
        statusMessage = message;
        emit statusMessageChanged(message);
    }
}

QString Friend::getStatusMessage() const { return statusMessage; }

void Friend::setEventFlag(bool flag) { hasNewEvents = flag; }

bool Friend::getEventFlag() const { return hasNewEvents; }

void Friend::setStatus(Status::Status s) {
    if (friendStatus != s) {
        auto oldStatus = friendStatus;
        friendStatus = s;
        emit statusChanged(friendStatus, hasNewEvents);
        if (!Status::isOnline(oldStatus) && Status::isOnline(friendStatus)) {
            emit onlineOfflineChanged(true);
        } else if (Status::isOnline(oldStatus) && !Status::isOnline(friendStatus)) {
            emit onlineOfflineChanged(false);
        }
    }
}

Status::Status Friend::getStatus() const { return friendStatus; }
