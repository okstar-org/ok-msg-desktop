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

#include "friendmessagedispatcher.h"
#include "src/model/status.h"
#include "src/model/message.h"
#include "src/persistence/settings.h"

namespace {

/**
 * @brief Sends message to friend using messageSender
 * @param[in] messageSender
 * @param[in] f
 * @param[in] message
 * @param[out] receipt
 */
bool sendMessageToCore(ICoreFriendMessageSender &messageSender,
                       const ToxPk &f,
                       const Message &message, ReceiptNum &receipt,
                       bool encrypt) {
  QString friendId = f.getId();

  auto sendFn = message.isAction
                    ? std::mem_fn(&ICoreFriendMessageSender::sendAction)
                    : std::mem_fn(&ICoreFriendMessageSender::sendMessage);

  return sendFn(messageSender, friendId, message.content, receipt, encrypt);
}
} // namespace

FriendMessageDispatcher::FriendMessageDispatcher(
        const ToxPk &f_,
        const MessageProcessor::SharedParams& p,
        ICoreIdHandler &idHandler_,
        ICoreFriendMessageSender &messageSender_)
    : f(f_),
      messageSender(messageSender_),
      offlineMsgEngine(&f_, &messageSender_),
      processor(MessageProcessor(idHandler_, f_, p ))
{
//  connect(&f, &IMFriend::onlineOfflineChanged,
//          this, &FriendMessageDispatcher::onFriendOnlineOfflineChanged);
}

FriendMessageDispatcher::~FriendMessageDispatcher()
{
    qDebug()<<__func__;
}

/**
 * @see IMessageSender::sendMessage
 */
std::pair<DispatchedMessageId, SentMessageId>
FriendMessageDispatcher::sendMessage(bool isAction, const QString &content, bool encrypt) {
  qDebug() << "FriendMessageDispatcher::sendMessage" << content;

  const auto firstId = nextMessageId;
  auto lastId = nextMessageId;

  for (const auto &message : processor.processOutgoingMessage(isAction, content)) {

    auto messageId = nextMessageId++;
    lastId = messageId;
    auto onOfflineMsgComplete = [this, messageId] {
      qDebug() << "FriendMessageDispatcher::onOfflineMsgComplete messageId:" << messageId.get();
      emit this->messageComplete(messageId);
    };

    ReceiptNum receipt;

    bool messageSent = false;

    emit this->messageSent(messageId, message);

//    if (Status::isOnline(f.getStatus())) {
      messageSent = sendMessageToCore(messageSender, f, message, receipt, encrypt);
//    }

    if (!messageSent) {
      offlineMsgEngine.addUnsentMessage(message, onOfflineMsgComplete);
    } else {
      offlineMsgEngine.addSentMessage(receipt, message, onOfflineMsgComplete);
    }

  }
  return std::make_pair(firstId, "");
}

/**
 * @brief Handles received message from toxcore
 * @param[in] isAction True if action message
 * @param[in] content Unprocessed toxcore message
 */
void FriendMessageDispatcher::onMessageReceived(FriendMessage& msg) {
  auto msg0 = processor.processIncomingMessage(msg);
  emit messageReceived(ToxPk(msg.from), msg0);
}


/**
 * @brief Handles received receipt from toxcore
 * @param[in] receipt receipt id
 */
void FriendMessageDispatcher::onReceiptReceived(ReceiptNum receipt) {
  offlineMsgEngine.onReceiptReceived(receipt);
}

/**
 * @brief Handles status change for friend
 * @note Parameters just to fit slot api
 */
void FriendMessageDispatcher::onFriendOnlineOfflineChanged(bool isOnline) {
  if (isOnline) {
    offlineMsgEngine.deliverOfflineMsgs();
  }
}

/**
 * @brief Clears all currently outgoing messages
 */
void FriendMessageDispatcher::clearOutgoingMessages() {
    offlineMsgEngine.removeAllMessages();
}

void FriendMessageDispatcher::onFileReceived( const ToxFile &file)
{
    const auto &friendId = ToxPk(file.getFriendId());
    emit fileReceived(friendId, file);
}
