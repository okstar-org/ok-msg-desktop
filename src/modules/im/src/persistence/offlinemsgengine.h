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

class OfflineMsgEngine : public QObject
{
    Q_OBJECT
public:
    explicit OfflineMsgEngine(const Friend* f, ICoreFriendMessageSender* messageSender);

    using CompletionFn = std::function<void()>;
    void addUnsentMessage(Message const& message, CompletionFn completionCallback);
    void addSentMessage(ReceiptNum receipt, Message const& message, CompletionFn completionCallback);
    void deliverOfflineMsgs();

public slots:
    void removeAllMessages();
    void onReceiptReceived(ReceiptNum receipt);

private:
    struct OfflineMessage
    {
        Message message;
        std::chrono::time_point<std::chrono::steady_clock> authorshipTime;
        CompletionFn completionFn;
    };

private slots:
    void completeMessage(QMap<ReceiptNum, OfflineMessage>::iterator msgIt);

private:
    void checkForCompleteMessages(ReceiptNum receipt);

    CompatibleRecursiveMutex mutex;
    const Friend* f;
    ICoreFriendMessageSender* messageSender;
    QVector<ReceiptNum> receivedReceipts;
    QMap<ReceiptNum, OfflineMessage> sentMessages;
    QVector<OfflineMessage> unsentMessages;
};

#endif // OFFLINEMSGENGINE_H
