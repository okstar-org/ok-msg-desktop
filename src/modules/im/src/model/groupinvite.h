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

#ifndef GROUPINVITE_H
#define GROUPINVITE_H

#include <QByteArray>
#include <QDateTime>
#include "message.h"

class GroupInvite {
public:
    GroupInvite() = default;
    GroupInvite(QString groupId, QString friendId, ConferenceType inviteType,
                const QByteArray& data);
    bool operator==(const GroupInvite& other) const;

    const QString& getGroupId() const;
    const QString& getFriendId() const;
    ConferenceType getType() const;
    QByteArray getInvite() const;
    QDateTime getInviteDate() const;

private:
    QString groupId;
    QString friendId;
    ConferenceType type{ConferenceType::TEXT};
    QByteArray invite;
    QDateTime date;
};

#endif  // GROUPINVITE_H
