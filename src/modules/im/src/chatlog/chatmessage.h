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

#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QDateTime>
#include "chatline.h"
#include "src/core/toxfile.h"
#include "src/model/chatlogitem.h"
#include "src/persistence/history.h"

class QGraphicsScene;

class ChatMessage {
public:
    using Ptr = std::shared_ptr<ChatMessage>;

    enum SystemMessageType {
        INFO,
        ERROR,
        TYPING,
    };

    enum MessageType {
        NORMAL,
        ACTION,
        ALERT,
    };

    static IChatItem::Ptr createChatMessage(const ChatLogItem& item, const QString& rawMessage,
                                            MessageType type, bool isMe, MessageState state,
                                            const QDateTime& date, bool colorizeName = false);

    static IChatItem::Ptr createChatInfoMessage(const QString& rawMessage,
                                                SystemMessageType type,
                                                const QDateTime& date);
    static IChatItem::Ptr createFileTransferMessage(const ChatLogItem& item, ToxFile file,
                                                    bool isMe, const QDateTime& date);
    static IChatItem::Ptr createTypingNotification();
    static IChatItem::Ptr createBusyNotification();

protected:
    static QString detectQuotes(const QString& str, MessageType type);
    static QString wrapDiv(const QString& str, const QString& div);

private:
    bool action = false;
};

#endif  // CHATMESSAGE_H
