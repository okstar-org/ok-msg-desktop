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

#ifndef IMESSAGE_DISPATCHER_H
#define IMESSAGE_DISPATCHER_H

#include "message.h"
#include "src/model/friend.h"
#include "src/model/message.h"

#include <QObject>
#include <QString>

#include <cstdint>

using DispatchedMessageId = NamedType<size_t, struct SentMessageIdTag, Orderable, Incrementable>;
Q_DECLARE_METATYPE(DispatchedMessageId);

class IMessageDispatcher : public QObject {
    Q_OBJECT
public:
    virtual ~IMessageDispatcher() = default;

    virtual std::pair<DispatchedMessageId, MsgId> sendMessage(bool isAction, const QString& content,
                                                              bool encrypt = false) = 0;

    virtual void clearOutgoingMessages() = 0;

signals:
    /**
     * @brief Emitted when a message is received and processed
     */
    void messageReceived(const FriendId& sender, const Message& message);

    /**
     * @brief Emitted when a message is processed and sent
     * @param id message id for completion
     * @param message sent message
     */
    void messageSent(DispatchedMessageId id, const Message& message);

    /**
     * @brief Emitted when a receiver report is received from the associated chat
     * @param id Id of message that is completed
     */
    void messageComplete(DispatchedMessageId id);

    /**
     * @brief 消息被对方接收或者被读
     * @param id
     */
    void messageReceipt(DispatchedMessageId id);

    void fileReceived(const FriendId& f, const ToxFile& file);

    void fileCancelled(const FriendId& f, const QString& fileId);
};

#endif /* IMESSAGE_DISPATCHER_H */
