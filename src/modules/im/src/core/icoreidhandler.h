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

#ifndef ICORE_ID_HANDLER_H
#define ICORE_ID_HANDLER_H

#include "FriendId.h"
#include "toxid.h"

class ICoreIdHandler {
public:
    virtual ~ICoreIdHandler() = default;
    virtual ToxId getSelfPeerId() const = 0;
    virtual FriendId getSelfId() const = 0;
    virtual QString getUsername() const = 0;
    virtual QString getNick() const = 0;
};

#endif /*ICORE_ID_HANDLER_H*/
