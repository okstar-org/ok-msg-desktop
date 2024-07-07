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

#include "offlinemsgengine.h"
#include "src/core/core.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include <QCoreApplication>
#include <QMutexLocker>
#include <QTimer>
#include <chrono>

OfflineMsgEngine::OfflineMsgEngine(const FriendId *frnd,
                                   ICoreFriendMessageSender *messageSender)
    : f(frnd), messageSender(messageSender) {}

/**
 * @brief Notification that the message is now delivered.
 *
 * @param[in] receipt   Toxcore message ID which the receipt is for.
 */
void OfflineMsgEngine::onReceiptReceived(ReceiptNum receipt) {
  QMutexLocker ml(&mutex);
  if (receivedReceipts.contains(receipt)) {
    qWarning() << "Receievd duplicate receipt" << receipt << "from friend"
               << f->getId();
    return;
  }
  receivedReceipts.append(receipt);
  checkForCompleteMessages(receipt);
}

/**
 * @brief Add a message which has been saved to history, but not sent yet to the
 * peer.
 *
 * OfflineMsgEngine will send this message once the friend becomes online again,
 * then track its receipt, updating history and chatlog once received.
 *
 * @param[in] messageID   database RowId of the message, used to eventually mark
 * messages as received in history
 * @param[in] msg         chat message line in the chatlog, used to eventually
 * set the message's receieved timestamp
 */
void OfflineMsgEngine::addUnsentMessage(Message const &message,
                                        CompletionFn completionCallback) {
  qDebug() << __func__ << message.content;

  QMutexLocker ml(&mutex);
  unsentMessages.append(OfflineMessage{
      message, std::chrono::steady_clock::now(), completionCallback});
}

/**
 * @brief Add a message which has been saved to history, and which has been sent
 * to the peer.
 *
 * OfflineMsgEngine will track this message's receipt. If the friend goes
 * offline then comes back before the receipt is received, OfflineMsgEngine will
 * also resend the message, updating history and chatlog once received.
 *
 * @param[in] receipt     the toxcore message ID, corresponding to expected
 * receipt ID
 * @param[in] messageID   database RowId of the message, used to eventually mark
 * messages as received in history
 * @param[in] msg         chat message line in the chatlog, used to eventually
 * set the message's receieved timestamp
 */
void OfflineMsgEngine::addSentMessage(ReceiptNum receipt,
                                      Message const &message,
                                      CompletionFn completionCallback) {

  qDebug() << __func__ << message.content;

  QMutexLocker ml(&mutex);
  //    assert(!sentMessages.contains(receipt));
  if (sentMessages.contains(receipt)) {
    sentMessages.remove(receipt);
  }
  sentMessages.insert(receipt, {message, std::chrono::steady_clock::now(), completionCallback});
  checkForCompleteMessages(receipt);
}

/**
 * @brief Deliver all messages, used when a friend comes online.
 */
void OfflineMsgEngine::deliverOfflineMsgs() {
  QMutexLocker ml(&mutex);

//  if (!Status::isOnline(f->getStatus())) {
//    return;
//  }

  if (sentMessages.empty() && unsentMessages.empty()) {
    return;
  }

  QVector<OfflineMessage> messages =
      sentMessages.values().toVector() + unsentMessages;
  // order messages by authorship time to resend in same order as they were
  // written
  std::sort(messages.begin(), messages.end(),
            [](const OfflineMessage &lhs, const OfflineMessage &rhs) {
              return lhs.authorshipTime < rhs.authorshipTime;
            });
  removeAllMessages();

  for (const auto &message : messages) {
    QString messageText = message.message.content;
    ReceiptNum receipt;
    bool messageSent{false};
    if (message.message.isAction) {
      messageSent = messageSender->sendAction(f->getId(), messageText, receipt);
    } else {
      messageSent =
          messageSender->sendMessage(f->getId(), messageText, receipt);
    }
    if (messageSent) {
      qDebug() <<"receipt:"<<receipt;
      addSentMessage(receipt, message.message, message.completionFn);
    } else {
      qCritical() << "deliverOfflineMsgs failed to send message";
      addUnsentMessage(message.message, message.completionFn);
    }
  }
}

/**
 * @brief Removes all messages which are being tracked.
 */
void OfflineMsgEngine::removeAllMessages() {
  QMutexLocker ml(&mutex);
  receivedReceipts.clear();
  sentMessages.clear();
  unsentMessages.clear();
}

void OfflineMsgEngine::completeMessage(
    QMap<ReceiptNum, OfflineMessage>::iterator msgIt) {
  msgIt->completionFn();
  receivedReceipts.removeOne(msgIt.key());
  sentMessages.erase(msgIt);
}

void OfflineMsgEngine::checkForCompleteMessages(ReceiptNum receipt) {
  qDebug() << __func__ << receipt;
  auto msgIt = sentMessages.find(receipt);
  if (msgIt == sentMessages.end()) {
    return;
  }

  const bool receiptReceived = receivedReceipts.contains(receipt);
  if (!receiptReceived) {
    return;
  }

  completeMessage(msgIt);
}
