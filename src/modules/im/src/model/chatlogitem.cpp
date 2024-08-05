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

#include "chatlogitem.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/friend.h"
#include "src/model/group.h"

#include <QDebug>
#include <cassert>

namespace {

/**
 * Helper template to get the correct deleter function for our type erased unique_ptr
 */
template <typename T> struct ChatLogItemDeleter {
    static void doDelete(void* ptr) { delete static_cast<T*>(ptr); }
};
}  // namespace

ChatLogItem::ChatLogItem(FriendId sender_, QString displayName, ChatLogFile file_)
        : ChatLogItem(sender_, displayName, ContentType::fileTransfer,
                      ContentPtr(new ChatLogFile(std::move(file_)),
                                 ChatLogItemDeleter<ChatLogFile>::doDelete)) {}

ChatLogItem::ChatLogItem(FriendId sender_, QString displayName, ChatLogMessage message_)
        : ChatLogItem(sender_, displayName, ContentType::message,
                      ContentPtr(new ChatLogMessage(std::move(message_)),
                                 ChatLogItemDeleter<ChatLogMessage>::doDelete)) {}

ChatLogItem::ChatLogItem(FriendId sender_, QString displayName, ContentType contentType_,
                         ContentPtr content_)
        : sender(std::move(sender_))
        , displayName(displayName)
        , contentType(contentType_)
        , content(std::move(content_)) {}

const FriendId& ChatLogItem::getSender() const { return sender; }

ChatLogItem::ContentType ChatLogItem::getContentType() const { return contentType; }

ChatLogFile& ChatLogItem::getContentAsFile() {
    assert(contentType == ContentType::fileTransfer);
    return *static_cast<ChatLogFile*>(content.get());
}

const ChatLogFile& ChatLogItem::getContentAsFile() const {
    assert(contentType == ContentType::fileTransfer);
    return *static_cast<ChatLogFile*>(content.get());
}

ChatLogMessage& ChatLogItem::getContentAsMessage() {
    assert(contentType == ContentType::message);
    return *static_cast<ChatLogMessage*>(content.get());
}

const ChatLogMessage& ChatLogItem::getContentAsMessage() const {
    assert(contentType == ContentType::message);
    return *static_cast<ChatLogMessage*>(content.get());
}

QDateTime ChatLogItem::getTimestamp() const {
    switch (contentType) {
        case ChatLogItem::ContentType::message: {
            const auto& message = getContentAsMessage();
            return message.message.timestamp;
        }
        case ChatLogItem::ContentType::fileTransfer: {
            const auto& file = getContentAsFile();
            return file.timestamp;
        }
    }

    assert(false);
    return QDateTime();
}

void ChatLogItem::setDisplayName(QString name) { displayName = name; }

const QString& ChatLogItem::getDisplayName() const { return displayName; }
