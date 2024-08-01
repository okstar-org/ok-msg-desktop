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

#ifndef OK_FRIEND_ID_H
#define OK_FRIEND_ID_H

#include <QByteArray>
#include <QHash>
#include "src/core/contactid.h"

namespace lib::messenger {
class IMContactId;
class IMPeerId;
}  // namespace lib::messenger

/**
 * 朋友ID
 * 格式：user@server
 * 比如：jidlpdyibulw@meet.chuanshaninfo.com
 */
class FriendId : public ContactId {
public:
    FriendId();
    FriendId(const FriendId& other);
    explicit FriendId(const QByteArray& rawId);
    explicit FriendId(const QString& rawId);
    explicit FriendId(const ContactId& rawId);
    explicit FriendId(const lib::messenger::IMContactId& fId);

    bool operator==(const FriendId& other) const;
    bool operator<(const FriendId& other) const;

    int getSize() const;

    QByteArray getByteArray() const;

    QString toString() const override;
};

inline uint qHash(const FriendId& id) { return qHash(id.getByteArray()); }

class ToxPeer : public FriendId {
public:
    explicit ToxPeer() = default;

    explicit ToxPeer(const lib::messenger::IMPeerId& peerId);

    explicit ToxPeer(const QString& rawId);

    bool isValid() const override;

    QString toString() const override;

    QString getResource() const { return resource; }

    const FriendId toFriendId() const { return FriendId{username + "@" + server}; }

    QString resource;
};

#endif  // TOXPK_H
