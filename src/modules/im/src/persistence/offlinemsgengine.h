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

#ifndef OFFLINEMSGENGINE_H
#define OFFLINEMSGENGINE_H

#include "src/chatlog/chatmessage.h"
#include "src/core/core.h"
#include "src/model/message.h"
#include "src/persistence/db/rawdatabase.h"

#include "base/compatiblerecursivemutex.h"

#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <chrono>

class Friend;
class ICoreFriendMessageSender;

class OfflineMsgEngine : public QObject {
    Q_OBJECT
public:
    using CompletionFn = std::function<void()>;
    using ReceiptFn = std::function<void()>;

    explicit OfflineMsgEngine(const FriendId* f, ICoreFriendMessageSender* messageSender);
    void addUnsentMessage(Message const& message, CompletionFn completionCallback);
    void addSentMessage(MsgId receipt, Message const& message, CompletionFn completionCallback,
                        ReceiptFn receiptCallback);
    void deliverOfflineMsgs();

    bool isFromThis(const Message& msg);

public slots:
    void removeAllMessages();
    void onReceiptReceived(MsgId receipt);

private:
    struct OfflineMessage {
        Message message;
        std::chrono::time_point<std::chrono::steady_clock> authorshipTime;
        CompletionFn completionFn;
        ReceiptFn receiptFn;
    };

private slots:
    void completeMessage(QMap<MsgId, OfflineMessage>::iterator msgIt);
    void receiptMessage(QMap<MsgId, OfflineMessage>::iterator msgIt);

private:
    void checkForCompleteMessages(MsgId receipt);

    CompatibleRecursiveMutex mutex;
    const FriendId* f;
    ICoreFriendMessageSender* messageSender;
    QVector<MsgId> receivedReceipts;
    QMap<MsgId, OfflineMessage> sentMessages;
    QVector<OfflineMessage> unsentMessages;
};

#endif  // OFFLINEMSGENGINE_H
