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

GroupMessageDispatcher::GroupMessageDispatcher(const GroupId& g_, MessageProcessor::SharedParams p,
                                               ICoreIdHandler& idHandler_,
                                               ICoreGroupMessageSender& messageSender_,
                                               const IGroupSettings& groupSettings_)
        : groupId(g_)
        , processor(MessageProcessor(idHandler_, g_, p))
        , idHandler(idHandler_)
        , messageSender(messageSender_)
        , groupSettings(groupSettings_) {
    //    processor.enableMentions();
}

GroupMessageDispatcher::~GroupMessageDispatcher() { qDebug() << __func__; }

std::pair<DispatchedMessageId, MsgId> GroupMessageDispatcher::sendMessage(bool isAction,
                                                                          QString const& content,
                                                                          bool encrypt) {
    Q_UNUSED(encrypt);

    //  const auto firstMessageId = nextMessageId;
    //  auto lastMessageId = firstMessageId;

    for (auto& message : processor.processOutgoingMessage(isAction, content)) {
        qDebug() << "Preparing to send a message:" << message.id;
        auto dispatchedId = nextMessageId++;
        //    lastMessageId = dispatchedId;

        emit messageSent(dispatchedId, message);
        emit messageComplete(dispatchedId);

        bool sent;
        if (message.isAction) {
            sent = messageSender.sendGroupAction(groupId.getId(), message.content, message.id);
        } else {
            sent = messageSender.sendGroupMessage(groupId.getId(), message.content, message.id);
        }
        qDebug() << "sendMessage=>" << sent
                 << QString("{msgId:%1, dispatcherId:%2}").arg(message.id).arg(dispatchedId.get());

        sentMsgIdMap.insert(message.id, dispatchedId);

        return std::make_pair(dispatchedId, message.id);
    }

    return {};
}

/**
 * @brief Processes and dispatches received message
 * @param[in] sender
 * @param[in] isAction True if is action
 * @param[in] content Message content
 */
void GroupMessageDispatcher::onMessageReceived(GroupMessage& msg) {
    qDebug() << __func__ << "id:" << msg.id << "nick:" << msg.nick << "msg:" << msg.content;
    if (sentMsgIdMap.contains(msg.id)) {
        qWarning() << "Is a sent message!";
        return;
    }

    auto self = idHandler.getSelfPeerId().toString();
    if (self == msg.from) {
        qWarning() << "Is self message (from is mine).";
        return;
    }

    //  auto myNick= idHandler.getNick();
    //  qDebug()<< "Self nick:"<<myNick;

    //  if(nick == idHandler.getNick()){
    //    qWarning()<<"Is self message (nick is mine).";
    //    return;
    //  }

    //  if (groupSettings.getBlackList().contains(sender.toString())) {
    //    qDebug() << "the sender is in backlist" << sender.toString();
    //    return;
    //  }

    auto msg0 = processor.processIncomingMessage(msg);
    emit messageReceived(FriendId(msg.from), msg0);
}

void GroupMessageDispatcher::clearOutgoingMessages() {
    // noop
}
