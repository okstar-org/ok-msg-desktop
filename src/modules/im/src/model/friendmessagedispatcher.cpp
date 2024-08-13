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
#include "src/model/message.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"

namespace {

/**
 * @brief Sends message to friend using messageSender
 * @param[in] messageSender
 * @param[in] f
 * @param[in] message
 * @param[out] receipt
 */
bool sendMessageToCore(ICoreFriendMessageSender& messageSender,
                       const FriendId& f,
                       const Message& message,
                       const MsgId& msgId,
                       bool encrypt) {
    QString friendId = f.getId();

    auto sendFn = message.isAction ? std::mem_fn(&ICoreFriendMessageSender::sendAction)
                                   : std::mem_fn(&ICoreFriendMessageSender::sendMessage);

    return sendFn(messageSender, friendId, message.content, msgId, encrypt);
}
}  // namespace

FriendMessageDispatcher::FriendMessageDispatcher(const FriendId& f_,
                                                 const MessageProcessor::SharedParams& p,
                                                 ICoreIdHandler& idHandler_,
                                                 ICoreFriendMessageSender& messageSender_)
        : f(f_)
        , messageSender(messageSender_)
        , offlineMsgEngine(&f_, &messageSender_)
        , processor(MessageProcessor(idHandler_, f_, p)) {
    //  connect(&f, &IMFriend::onlineOfflineChanged,
    //          this, &FriendMessageDispatcher::onFriendOnlineOfflineChanged);
}

FriendMessageDispatcher::~FriendMessageDispatcher() { qDebug() << __func__; }

/**
 * @see IMessageSender::sendMessage
 */
std::pair<DispatchedMessageId, MsgId> FriendMessageDispatcher::sendMessage(bool isAction,
                                                                           const QString& content,
                                                                           bool encrypt) {
    qDebug() << __func__ << content;

    const auto firstId = nextMessageId;
    auto lastId = nextMessageId;

    for (const auto& message : processor.processOutgoingMessage(isAction, content)) {
        qDebug() << "Preparing to send a message:" << message.id;

        auto dispatcherId = nextMessageId++;
        qDebug() << "dispatcherId:" << dispatcherId.get();

        lastId = dispatcherId;

        auto onOfflineMsgComplete = [this, dispatcherId] { emit messageComplete(dispatcherId); };

        auto onMsgRead = [this, dispatcherId] { emit messageReceipt(dispatcherId); };

        emit messageSent(dispatcherId, message);

        bool messageSent = sendMessageToCore(messageSender, f, message, message.id, encrypt);
        qDebug() << "sendMessage=>" << messageSent
                 << QString("{msgId:%1, dispatcherId:%2}").arg(message.id).arg(dispatcherId.get());

        if (messageSent) {
            offlineMsgEngine.addSentMessage(message.id, message, onOfflineMsgComplete, onMsgRead);
        } else {
            offlineMsgEngine.addUnsentMessage(message, onOfflineMsgComplete);
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
    // 判断是否本peer发出
    if (offlineMsgEngine.isFromThis(msg)) {
        qWarning() << "Is from local msg.";
        return;
    }

    auto msg0 = processor.processIncomingMessage(msg);
    emit messageReceived(FriendId(msg.from), msg0);
}

/**
 * @brief Handles received receipt from toxcore
 * @param[in] receipt receipt id
 */
void FriendMessageDispatcher::onReceiptReceived(MsgId receipt) {
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
void FriendMessageDispatcher::clearOutgoingMessages() { offlineMsgEngine.removeAllMessages(); }

void FriendMessageDispatcher::onFileReceived(const ToxFile& file) {
    const auto& friendId = FriendId(f);
    emit fileReceived(friendId, file);
}

void FriendMessageDispatcher::onFileCancelled(const QString& fileId) {
    const auto& friendId = FriendId(f);
    emit fileCancelled(friendId, fileId);
}
