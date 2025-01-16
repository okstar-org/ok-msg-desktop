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

#include "User.h"

#include <utility>

namespace module::classroom {

UserId::UserId(std::string jid_,   //
               std::string name_,  //
               std::string nick_)  //
        : jid(std::move(jid_))
        ,  //
        username(std::move(name_))
        ,                       //
        nick(std::move(nick_))  //
{}

const std::string& UserId::getJid() const {
    return (jid);
}

const std::string& UserId::getUsername() const {
    return (username);
}

const std::string& UserId::getNick() const {
    return (nick);
}

}  // namespace module::classroom