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

#pragma once

#include <list>
#include <string>
#include <vector>

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
    std::string username;
    std::string server;

    bool operator==(const std::string& friendId) const;
    bool operator==(const IMContactId& friendId) const;
    bool operator!=(const IMContactId& friendId) const;
    bool operator<(const IMContactId& friendId) const;

    IMContactId() = default;
    IMContactId(const IMContactId&);
    explicit IMContactId(const std::string& jid);
    explicit IMContactId(const gloox::JID& jid);

    [[nodiscard]] std::string getUsername() const {
        return username;
    }
    [[nodiscard]] std::string getServer() const {
        return server;
    }

    [[nodiscard]] std::string toString() const {
        if (username.empty()) {
            return {};
        }

        if (server.empty()) {
            return username;
        }

        return username + "@" + server;
    }
};

struct IMPeerId : public IMContactId {
    /**
     * [username]@[server]/[resource]
     */
    std::string resource;

    IMPeerId();
    explicit IMPeerId(const std::string& peerId);
    explicit IMPeerId(const gloox::JID& jid);
    bool operator==(const IMPeerId& peerId) const;

    [[nodiscard]] inline std::string toFriendId() const {
        return username + "@" + server;
    }

    [[nodiscard]] inline std::string toString() const {
        if (resource.empty()) return toFriendId();
        return toFriendId() + "/" + resource;
    }
};

class IMFriend {
public:
    IMFriend();
    explicit IMFriend(gloox::RosterItem* pItem);

    IMContactId id;
    std::string alias;
    std::list<std::string> groups;
    int subscription;
    bool online;

    // 互相关注才是朋友
    [[nodiscard]] bool isFriend() const;
};

struct IMVCard {
    struct Adr {
        std::string street;
        std::string locality;
        std::string region;
        std::string country;
    };

    struct Tel {
        int type;  // 0:work
        bool mobile;
        std::string number;
    };

    struct Email {
        int type;  // 0:work
        std::string number;
    };

    struct Photo {
        std::string type;
        std::string bin;
        std::string url;
    };

    std::string fullName;
    std::string nickname;
    std::string title;
    std::vector<Adr> adrs;
    std::vector<Email> emails;
    std::vector<Tel> tels;
    Photo photo;
};

}  // namespace lib::messenger
