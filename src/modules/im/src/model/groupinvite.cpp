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

#include "groupinvite.h"

/**
 * @class GroupInvite
 *
 * @brief This class contains information needed to create a group invite
 */

GroupInvite::GroupInvite(QString groupId, QString friendId, ConferenceType inviteType,
                         const QByteArray& data)
        : groupId{groupId}
        , friendId{friendId}
        , type{inviteType}
        , invite{data}
        , date{QDateTime::currentDateTime()} {}

bool GroupInvite::operator==(const GroupInvite& other) const {
    return friendId == other.friendId && type == other.type && invite == other.invite &&
           date == other.date;
}

const QString& GroupInvite::getGroupId() const { return groupId; }

const QString& GroupInvite::getFriendId() const { return friendId; }

ConferenceType GroupInvite::getType() const { return type; }

QByteArray GroupInvite::getInvite() const { return invite; }

QDateTime GroupInvite::getInviteDate() const { return date; }
