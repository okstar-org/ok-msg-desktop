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

#ifndef CHAT_HISTORY_H
#define CHAT_HISTORY_H

#include "ichatlog.h"
#include "sessionchatlog.h"
#include "src/persistence/history.h"

#include <QSet>

class Settings;

class ChatHistory : public IChatLog {
    Q_OBJECT
public:
    ChatHistory(const ContactId& f_,
                History* history_,
                const ICoreIdHandler& coreIdHandler,
                const Settings& settings,
                IMessageDispatcher& messageDispatcher);

    ~ChatHistory();

    const ChatLogItem* at(ChatLogIdx idx) const override;
    // 最后几条
    QList<Message> getLastTextMessage(uint size);

    SearchResult searchForward(SearchPos startIdx, const QString& phrase,
                               const ParameterSearch& parameter) const override;
    SearchResult searchBackward(SearchPos startIdx, const QString& phrase,
                                const ParameterSearch& parameter) const override;
    ChatLogIdx getFirstIdx() const override;
    ChatLogIdx getNextIdx() const override;
    std::vector<DateChatLogIdxPair> getDateIdxs(const QDate& startDate,
                                                size_t maxDates) const override;

public slots:
    void onFileUpdated(const FriendId& sender, const ToxFile& file);
    void onFileCanceled(const FriendId& sender, const QString& fileId);
    void onFileTransferRemotePausedUnpaused(const FriendId& sender, const ToxFile& file,
                                            bool paused);
    void onFileTransferBrokenUnbroken(const FriendId& sender, const ToxFile& file, bool broken);

private slots:
    void onMessageReceived(const FriendId& sender, const Message& message);
    void onMessageSent(DispatchedMessageId id, const Message& message);
    void onMessageComplete(DispatchedMessageId id);
    void onMessageReceipt(DispatchedMessageId id);

private:
    void ensureIdxInSessionChatLog(ChatLogIdx idx) const;
    void loadHistoryIntoSessionChatLog(ChatLogIdx start) const;
    void dispatchUnsentMessages(IMessageDispatcher& messageDispatcher);
    void handleDispatchedMessage(DispatchedMessageId dispatchId, RowId historyId);
    void completeMessage(DispatchedMessageId id);
    void receiptMessage(DispatchedMessageId id);
    bool canUseHistory() const;
    ChatLogIdx getInitialChatLogIdx() const;

    const ContactId& f;
    History* history;
    const Settings& settings;
    const ICoreIdHandler& coreIdHandler;
    mutable SessionChatLog sessionChatLog;

    // If a message completes before it's inserted into history it will end up
    // in this set
    QSet<DispatchedMessageId> completedMessages;

    // If a message is inserted into history before it gets a completion
    // callback it will end up in this map
    QMap<DispatchedMessageId, RowId> dispatchedMessageRowIdMap;
};

#endif /*CHAT_HISTORY_H*/
