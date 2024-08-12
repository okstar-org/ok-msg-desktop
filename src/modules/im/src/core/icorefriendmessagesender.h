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

#ifndef ICORE_FRIEND_MESSAGE_SENDER_H
#define ICORE_FRIEND_MESSAGE_SENDER_H

#include "src/model/MsgId.h"

#include <QString>
#include <cstdint>

class ICoreFriendMessageSender {
public:
    virtual ~ICoreFriendMessageSender() = default;
    virtual bool sendAction(QString friendId, const QString& action, const MsgId& msgId,
                            bool encrypt = false) = 0;
    virtual bool sendMessage(QString friendId, const QString& message, const MsgId& msgId,
                             bool encrypt = false) = 0;
};

#endif /* ICORE_FRIEND_MESSAGE_SENDER_H */
