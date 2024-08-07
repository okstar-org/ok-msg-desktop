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

//
// Created by gaojie on 24-5-28.
//

#ifndef OKMSG_PROJECT_IMFRIEND_H
#define OKMSG_PROJECT_IMFRIEND_H

#include <QString>
#include <QStringList>

namespace gloox {
class JID;
class RosterItem;
}  // namespace gloox

namespace lib::messenger {

enum class IMStatus {
    Available,   /**< The entity is online. */
    Chat,        /**< The entity is 'available for chat'. */
    Away,        /**< The entity is away. */
    DND,         /**< The entity is DND (Do Not Disturb). */
    XA,          /**< The entity is XA (eXtended Away). */
    Unavailable, /**< The entity is offline. */
    Probe,       /**< This is a presence probe. */
    Error,       /**< This is a presence error. */
    Invalid      /**< The stanza is invalid. */
};

/**
 * 联系人ID，即：朋友ID、群聊ID
 * 格式:[username]@[server]
 */
struct IMContactId {
    QString username;
    QString server;

    bool operator==(const QString& friendId) const;
    bool operator==(const IMContactId& friendId) const;
    bool operator!=(const IMContactId& friendId) const;
    bool operator<(const IMContactId& friendId) const;

    IMContactId() = default;
    IMContactId(const IMContactId&);
    explicit IMContactId(const QString& jid);
    explicit IMContactId(const gloox::JID& jid);

    [[nodiscard]] QString getUsername() const { return username; }
    [[nodiscard]] QString getServer() const { return server; }

    [[nodiscard]] QString toString() const {
        if (username.isEmpty()) {
            return {};
        }

        if (server.isEmpty()) {
            return username;
        }

        return username + "@" + server;
    }
};

struct IMPeerId : public IMContactId {
    /**
     * [username]@[server]/[resource]
     */
    QString resource;

    IMPeerId();
    explicit IMPeerId(const QString& peerId);
    explicit IMPeerId(const gloox::JID& jid);
    bool operator==(const IMPeerId& peerId) const;

    [[nodiscard]] inline QString toFriendId() const { return username + "@" + server; }

    [[nodiscard]] inline QString toString() const {
        if (resource.isEmpty()) return toFriendId();
        return toFriendId() + "/" + resource;
    }
};

class IMFriend {
public:
    IMFriend();
    explicit IMFriend(gloox::RosterItem* pItem);

    IMContactId id;
    QString alias;
    QStringList groups;
    int subscription;
    bool online;

    // 互相关注才是朋友
    [[nodiscard]] bool isFriend() const;

    [[nodiscard]] QString toString() const {
        return QString("{id: %1, alias: %2, subscription:%3, online:%4, groups:[%5]}")  //
                .arg(id.toString())
                .arg(alias)
                .arg(subscription)
                .arg(online)
                .arg(groups.join(","));
    }
};

}  // namespace lib::messenger
#endif  // OKMSG_PROJECT_IMFRIEND_H
