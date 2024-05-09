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

#include <src/core/groupid.h>
#include <src/core/toxpk.h>

#include "lib/messenger/IMMessage.h"

class Friend;

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
  bool isAction;
  QString id;
  QString content;
  QString from;
  QDateTime timestamp;
  QString displayName;
  std::vector<MessageMetadata> metadata;
};


struct GroupMessage : public Message{
public:
    GroupId groupId;
    QString nick;
    QString toString() const {
        return QString("{id:%1, from:%2, content:%3}").arg(id).arg(from).arg(content);
    }
};

struct FriendMessage : Message{
    ToxPk from;
    ToxPk to;
};

class MessageProcessor {

public:
  /**
   * Parameters needed by all message processors. Used to reduce duplication
   * of expensive data looked at by all message processors
   */
  class SharedParams {

  public:
      //模式匹配
    QRegularExpression GetNameMention() const { return nameMention; }
    QRegularExpression GetSanitizedNameMention() const {
      return sanitizedNameMention;
    }
    QRegularExpression GetPublicKeyMention() const { return pubKeyMention; }
    void onUserNameSet(const QString &username);
    void setPublicKey(const QString &pk);

  private:
    QRegularExpression nameMention;
    QRegularExpression sanitizedNameMention;
    QRegularExpression pubKeyMention;
  };

  MessageProcessor(const SharedParams &sharedParams);

  std::vector<Message> processOutgoingMessage(bool isAction,

                                              QString const &content);

  Message processIncomingMessage(bool isAction,
                                 QString const &id,
                                 QString const &from,
                                 QString const &message,
                                 const QDateTime& time,
                                 QString const &displayName);

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
  const SharedParams &sharedParams;
};

#endif /*MESSAGE_H*/
