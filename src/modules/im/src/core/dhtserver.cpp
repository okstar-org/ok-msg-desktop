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

#include "dhtserver.h"

/**
 * @brief   Compare equal operator
 * @param   other   the compared instance
 * @return  true, if equal; false otherwise
 */
bool DhtServer::operator==(const DhtServer& other) const {
    return this == &other || (port == other.port && address == other.address &&
                              userId == other.userId && name == other.name);
}

/**
 * @brief   Compare not equal operator
 * @param   other   the compared instance
 * @return  true, if not equal; false otherwise
 */
bool DhtServer::operator!=(const DhtServer& other) const { return !(*this == other); }
