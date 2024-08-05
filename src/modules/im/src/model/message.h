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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include <QRegularExpression>
#include <QString>

#include <vector>

#include <src/core/FriendId.h>
#include <src/core/groupid.h>

#include "lib/messenger/IMMessage.h"
#include "src/core/icoreidhandler.h"

#include "MsgId.h"
#include "contact.h"

// NOTE: This could be extended in the future to handle all text processing (see
// ChatMessage::createChatMessage)
enum class MessageMetadataType {
    selfMention,
};

// May need to be extended in the future to have a more varianty type (imagine
// if we wanted to add message replies and shoved a reply id in here)
struct MessageMetadata {
    MessageMetadataType type;
    // Indicates start position within a Message::content
    size_t start;
    // Indicates end position within a Message::content
    size_t end;
};

struct Message {
public:
    bool isGroup{false};
    bool isAction;
    MsgId id;
    QString from;
    QString to;
    QString displayName;
    QString content;
    QString dataId;
    QDateTime timestamp;
    std::vector<MessageMetadata> metadata;
    QString toString() const {
        return QString("{id:%1, from:%2, to:%3, "
                       "time:%4, content:%5, sId:%6}")
                .arg(id)
                .arg(from)
                .arg(to)
                .arg(timestamp.toString())
                .arg(content)
                .arg(dataId);
    }
};

struct FriendMessage : Message {
    //    ToxPk to;
};

struct GroupMessage : public Message {
public:
    GroupMessage() { isGroup = true; }
    QString nick;
    QString resource;

    QString toString() const {
        return QString("{id:%1, from:%2, to:%3, time:%4, content:%5, nick:%6, resource:%7}")
                .arg(id)
                .arg(from)
                .arg(to)
                .arg(timestamp.toString())
                .arg(content)
                .arg(nick)
                .arg(resource);
    }
};

struct FriendInfo {
    FriendId id;
    QString alias;
    bool is_friend;
    bool online;
    QStringList groups;

    explicit FriendInfo(const lib::messenger::IMFriend& aFriend);

    [[nodiscard]] inline const FriendId& getId() const { return id; }

    [[nodiscard]] inline const QString& getAlias() const { return alias; }

    [[nodiscard]] inline bool isFriend() const { return is_friend; };

    [[nodiscard]] inline QString toString() const {
        return QString("{id: %1, alias: %2, is_friend:%3, online:%4, groups:[%5]}")  //
                .arg(id.toString())
                .arg(alias)
                .arg(is_friend)
                .arg(online)
                .arg(groups.join(","));
    }

    friend QDebug& operator<<(QDebug& debug, const FriendInfo& f);
};

struct GroupInfo {
    QString name;
    QString description;
    QString subject;
    QString creationdate;
    uint64_t occupants = 0;
};

struct GroupOccupant {
    QString jid;
    QString nick;
    QString affiliation;
    QString role;
    int status;
    QList<int> codes;
};

enum class ChatType {
    Chat,      // 单聊
    GroupChat  // 群聊
};

enum class ConferenceType { TEXT, AV };

class MessageProcessor {
public:
    /**
     * Parameters needed by all message processors. Used to reduce duplication
     * of expensive data looked at by all message processors
     */
    class SharedParams {
    public:
        // 模式匹配
        const QRegularExpression& GetNameMention() const { return nameMention; }

        const QRegularExpression& GetSanitizedNameMention() const { return sanitizedNameMention; }
        const QRegularExpression& GetPublicKeyMention() const { return pubKeyMention; }

        void onUserNameSet(const QString& username);
        void setPublicKey(const QString& pk);

    private:
        QRegularExpression nameMention;
        QRegularExpression sanitizedNameMention;
        QRegularExpression pubKeyMention;
    };

    MessageProcessor(ICoreIdHandler& idHandler,
                     const ContactId& f,
                     const SharedParams& sharedParams);

    std::vector<Message> processOutgoingMessage(bool isAction, QString const& content);

    Message processIncomingMessage(Message& message);

    /**
     * @brief Enables mention detection in the processor
     */
    inline void enableMentions() { detectingMentions = true; }

    /**
     * @brief Disables mention detection in the processor
     */
    inline void disableMentions() { detectingMentions = false; };

private:
    bool detectingMentions = false;
    ICoreIdHandler& idHandler;
    const ContactId& f;
    const SharedParams& sharedParams;
};

#endif /*MESSAGE_H*/
