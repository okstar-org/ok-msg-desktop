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

#include <stdint.h>
#include <string>
#include <vector>

namespace lib::messenger {

/**
 * 群聊
 */
struct IMGroup {
    std::string name;
    std::string description;
    std::string subject;
    std::string creationdate;
    uint32_t occupants = 0;
};

/**
 * 群聊名称
 */
struct IMGroupOccupant {
    std::string jid;
    std::string nick;
    std::string affiliation;
    std::string role;
    int status;
    // https://xmpp.org/registrar/mucstatus.html
    std::vector<int> codes;
};

}  // namespace lib::messenger

