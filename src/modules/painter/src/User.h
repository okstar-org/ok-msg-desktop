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

#pragma once

#include <map>

#include <base/basic_types.h>
#include <QString>
#include <string>
#include "base/OkAccount.h"

namespace module::painter {

using PeerUIN = QString;
using UserJID = ok::base::Jid;
typedef std::map<PeerUIN, UserJID> UserJIDMap;

struct PeerStatus {
    bool audioMuted = true;
    bool videoMuted = true;
    bool raisedHand = false;
    std::string videoType;
};

/**
 *
 */
class UserId {
public:
    UserId() = default;

    UserId(std::string jid, std::string name, std::string nick = "");

    const std::string& getJid() const;
    const std::string& getUsername() const;
    const std::string& getNick() const;

private:
    std::string jid;
    std::string username;
    std::string nick;
};

enum class UserType {
    NONE = 0,
    Member = 1,
    Teacher = 2,
};

class UserInfo {
public:
    explicit UserInfo() {}

    explicit UserInfo(const UserInfo& info)
            : id(info.id)
            , userType(info.userType)
            , uin(info.uin)
            , name(info.name)
            , username(info.username)
            , avatar(info.avatar) {}

    virtual ~UserInfo() {}

    uint64_t getId() const {
        return id;
    }

    const QString& getUin() const {
        return uin;
    }

    const QString& getUsername() const {
        return username;
    }

    const QString& getName() const {
        return name;
    }

    const QString& getAvatar() const {
        return avatar;
    }

    int getUserType() const {
        return userType;
    }

private:
    uint64_t id = 0;
    int userType = -1;
    QString uin;
    QString name;
    QString username;
    QString avatar;
};

}  // namespace module::painter
