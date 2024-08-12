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
// Created by gaojie on 24-8-3.
//

#include "AccessToken.h"
#include "lib/backend/PassportService.h"

namespace ok::platform {

AccessToken::AccessToken() : expiresIn{0}, refreshExpiresIn{0} {}

AccessToken::AccessToken(backend::SysToken& sysToken)
        : username(sysToken.username)
        , tokenType(sysToken.tokenType)
        , accessToken(sysToken.accessToken)
        , expiresIn(sysToken.expiresIn)
        , refreshToken(sysToken.accessToken)
        , refreshExpiresIn(sysToken.refreshExpiresIn)
        , session_state(sysToken.session_state) {}
AccessToken::~AccessToken() = default;
}  // namespace ok::platform