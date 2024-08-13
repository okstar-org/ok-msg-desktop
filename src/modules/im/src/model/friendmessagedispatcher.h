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

#ifndef FRIEND_MESSAGE_DISPATCHER_H
#define FRIEND_MESSAGE_DISPATCHER_H

#include "src/core/icorefriendmessagesender.h"
#include "src/model/friend.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/message.h"
#include "src/persistence/offlinemsgengine.h"

#include <QObject>
#include <QString>

#include <cstdint>

class FriendMessageDispatcher : public IMessageDispatcher {
    Q_OBJECT
public:
    FriendMessageDispatcher(const FriendId& f,
                            const MessageProcessor::SharedParams& sharedParams,
                            ICoreIdHandler& idHandler_,
                            ICoreFriendMessageSender& messageSender);
    ~FriendMessageDispatcher();

    std::pair<DispatchedMessageId, MsgId> sendMessage(bool isAction,
                                                      const QString& content,
                                                      bool encrypt = false) override;

    void onMessageReceived(FriendMessage& msg);
    void onReceiptReceived(MsgId receipt);
    void clearOutgoingMessages() override;

    void onFileReceived(const ToxFile& file);
    void onFileCancelled(const QString& fileId);

private slots:
    void onFriendOnlineOfflineChanged(bool isOnline);

private:
    const FriendId& f;
    DispatchedMessageId nextMessageId = DispatchedMessageId(0);

    ICoreFriendMessageSender& messageSender;
    OfflineMsgEngine offlineMsgEngine;
    MessageProcessor processor;
};

#endif /* IMESSAGE_DISPATCHER_H */
