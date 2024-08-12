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

#include "message.h"
#include "base/uuid.h"
#include "friend.h"
#include "src/core/core.h"

void MessageProcessor::SharedParams::onUserNameSet(const QString& username) {
    QString sanename = username;
    sanename.remove(QRegularExpression("[\\t\\n\\v\\f\\r\\x0000]"));
    nameMention = QRegularExpression("\\b" + QRegularExpression::escape(username) + "\\b",
                                     QRegularExpression::CaseInsensitiveOption);
    sanitizedNameMention = QRegularExpression("\\b" + QRegularExpression::escape(sanename) + "\\b",
                                              QRegularExpression::CaseInsensitiveOption);
}

/**
 * @brief Set the public key on which a message should be highlighted
 * @param pk ToxPk in its hex string form
 */
void MessageProcessor::SharedParams::setPublicKey(const QString& pk) {
    // no sanitization needed, we expect a ToxPk in its string form
    pubKeyMention =
            QRegularExpression("\\b" + pk + "\\b", QRegularExpression::CaseInsensitiveOption);
}

MessageProcessor::MessageProcessor(ICoreIdHandler& idHandler_, const ContactId& f_,
                                   const MessageProcessor::SharedParams& sharedParams_)
        : idHandler{idHandler_}, f{f_}, sharedParams(sharedParams_) {}

/**
 * @brief Converts an outgoing message into one (or many) sanitized Message(s)
 */
std::vector<Message> MessageProcessor::processOutgoingMessage(bool isAction,
                                                              QString const& content) {
    std::vector<Message> ret;

    QStringList splitMsgs(content);

    QDateTime timestamp = QDateTime::currentDateTime();
    std::transform(splitMsgs.begin(), splitMsgs.end(), std::back_inserter(ret),
                   [&](const QString& part) {
                       Message message;
                       message.id = ok::base::UUID::make();
                       message.isAction = isAction;
                       message.to = f.getId();
                       message.from = idHandler.getSelfPeerId().toString();
                       message.content = part;
                       message.timestamp = timestamp;
                       qDebug() << "Generated a new message:" << message.id;
                       return message;
                   });
    return ret;
}

/**
 * @brief Converts an incoming message into a sanitized Message
 */
Message MessageProcessor::processIncomingMessage(Message& ret) {
    if (detectingMentions) {
        auto nameMention = sharedParams.GetNameMention();
        auto sanitizedNameMention = sharedParams.GetSanitizedNameMention();
        auto pubKeyMention = sharedParams.GetPublicKeyMention();

        for (auto const& mention : {nameMention, sanitizedNameMention, pubKeyMention}) {
            auto matchIt = mention.globalMatch(ret.content);
            if (!matchIt.hasNext()) {
                continue;
            }

            auto match = matchIt.next();

            auto pos = static_cast<size_t>(match.capturedStart());
            auto length = static_cast<size_t>(match.capturedLength());

            // skip matches on empty usernames
            if (length == 0) {
                continue;
            }

            ret.metadata.push_back({MessageMetadataType::selfMention, pos, pos + length});
            break;
        }
    }

    return ret;
}

FriendInfo::FriendInfo(const lib::messenger::IMFriend& aFriend)
        : id{ContactId{aFriend.id.toString()}}
        , alias{aFriend.alias}
        , is_friend{aFriend.isFriend()}
        , online(aFriend.online)
        , groups(aFriend.groups) {}

QDebug& operator<<(QDebug& debug, const FriendInfo& f) {
    QDebugStateSaver saver(debug);
    debug.nospace() << f.toString();
    return debug;
}
