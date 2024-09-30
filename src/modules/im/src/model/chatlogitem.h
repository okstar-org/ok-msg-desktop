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

#ifndef CHAT_LOG_ITEM_H
#define CHAT_LOG_ITEM_H

#include "src/core/FriendId.h"
#include "src/core/toxfile.h"
#include "src/model/message.h"
#include "src/persistence/history.h"

#include <memory>

struct ChatLogMessage {
    MessageState state;
    Message message;
};

struct ChatLogFile {
    QDateTime timestamp;
    ToxFile file;
};

class ChatLogItem {
private:
    using ContentPtr = std::unique_ptr<void, void (*)(void*)>;

public:
    enum class ContentType {
        message,
        fileTransfer,
    };

    ChatLogItem(FriendId sender, QString id, QString displayName, ChatLogFile file);
    ChatLogItem(FriendId sender, QString id, QString displayName, ChatLogMessage message);
    ChatLogItem(FriendId sender, QString id, QString displayName, ContentType contentType,
                ContentPtr content);

    const FriendId& getSender() const;
    ContentType getContentType() const;
    ChatLogFile& getContentAsFile();
    const ChatLogFile& getContentAsFile() const;
    ChatLogMessage& getContentAsMessage();
    const ChatLogMessage& getContentAsMessage() const;
    QDateTime getTimestamp() const;
    void setDisplayName(QString name);
    const QString& getDisplayName() const;
    const QString& getId() const { return id; };

private:

    FriendId sender;
    QString displayName;
    ContentType contentType;
    ContentPtr content;
    QString id;
};

#endif /*CHAT_LOG_ITEM_H*/
