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
// Created by gaojie on 24-12-7.
//

#pragma once

#include <message.h>
#include <presence.h>

namespace lib::messenger {

class IMFromHostHandler {
public:
    virtual void handleHostPresence(const gloox::JID& from, const gloox::Presence& presence) = 0;
    virtual void handleHostMessage(const gloox::JID& from, const gloox::Message& msg) = 0;
    virtual void handleHostMessageSession(const gloox::JID& from, const std::string& sid) = 0;
};

}  // namespace lib::messenger
