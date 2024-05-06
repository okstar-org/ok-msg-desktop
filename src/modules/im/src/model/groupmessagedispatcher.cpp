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

#include "groupmessagedispatcher.h"
#include "src/persistence/igroupsettings.h"

#include <QtCore>

GroupMessageDispatcher::GroupMessageDispatcher(
    Group &g_, MessageProcessor processor_, ICoreIdHandler &idHandler_,
    ICoreGroupMessageSender &messageSender_,
    const IGroupSettings &groupSettings_)
    : group(g_), processor(processor_), idHandler(idHandler_),
      messageSender(messageSender_), groupSettings(groupSettings_) {
  processor.enableMentions();
}

std::pair<DispatchedMessageId, DispatchedMessageId>
GroupMessageDispatcher::sendMessage(bool isAction, QString const &content,
                                    bool encrypt) {
  const auto firstMessageId = nextMessageId;
  auto lastMessageId = firstMessageId;

  for (auto const &message : processor.processOutgoingMessage(isAction, content)) {
    auto messageId = nextMessageId++;
    lastMessageId = messageId;

    if (message.isAction) {
      messageSender.sendGroupAction(group.getId(), message.content);
    } else {
      messageSender.sendGroupMessage(group.getId(), message.content);
    }

    // Emit both signals since we do not have receipts for groups
    //
    // NOTE: We could in theory keep track of our sent message and wait for
    // toxcore to send it back to us to indicate a completed message, but
    // this isn't necessarily the design of toxcore and associating the
    // received message back would be difficult.
    emit this->messageSent(messageId, message);
    emit this->messageComplete(messageId);
  }

  return std::make_pair(firstMessageId, lastMessageId);
}

/**
 * @brief Processes and dispatches received message from toxcore
 * @param[in] sender
 * @param[in] isAction True if is action
 * @param[in] content Message content
 */
void GroupMessageDispatcher::onMessageReceived(const ToxPk &sender,
                                               bool isAction,
                                               QString const &content,
                                               QString const& nick,
                                               QString const &from,
                                               const QDateTime &time) {

  //qDebug() << "onMessageReceived nick:" << nick<< "msg:" << content;
  auto self = idHandler.getSelfId().toString();
  if (self == from) {
    qWarning() << "Is self message (from is mine).";
    return;
  }

  auto myNick= idHandler.getNick();
  qDebug()<< "Self nick:"<<myNick;

//  if(nick == idHandler.getNick()){
//    qWarning()<<"Is self message (nick is mine).";
//    return;
//  }

  if (groupSettings.getBlackList().contains(sender.toString())) {
    qDebug() << "onGroupMessageReceived: Filtered:" << sender.toString();
    return;
  }

  auto msg = processor.processIncomingMessage(isAction, content, from, time, nick);
  emit messageReceived(sender, msg);
}
