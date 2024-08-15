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

#ifndef SESSION_CHAT_LOG_H
#define SESSION_CHAT_LOG_H

#include "ichatlog.h"
#include "imessagedispatcher.h"

#include <QList>
#include <QObject>

struct SessionChatLogMetadata;

class SessionChatLog : public IChatLog {
    Q_OBJECT
public:
    SessionChatLog(const ICoreIdHandler& coreIdHandler);
    SessionChatLog(ChatLogIdx initialIdx, const ICoreIdHandler& coreIdHandler);
    ~SessionChatLog();

    const ChatLogItem* at(ChatLogIdx idx) const override;
    SearchResult searchForward(SearchPos startIdx, const QString& phrase,
                               const ParameterSearch& parameter) const override;
    SearchResult searchBackward(SearchPos startIdx, const QString& phrase,
                                const ParameterSearch& parameter) const override;
    ChatLogIdx getFirstIdx() const override;
    ChatLogIdx getNextIdx() const override;
    std::vector<DateChatLogIdxPair> getDateIdxs(const QDate& startDate,
                                                size_t maxDates) const override;

    void insertCompleteMessageAtIdx(ChatLogIdx idx, const FriendId& sender,
                                    const QString& senderName, const ChatLogMessage& message);
    void insertIncompleteMessageAtIdx(ChatLogIdx idx, const FriendId& sender,
                                      const QString& senderName, const ChatLogMessage& message,
                                      DispatchedMessageId dispatchId);
    void insertBrokenMessageAtIdx(ChatLogIdx idx, const FriendId& sender, const QString& senderName,
                                  const ChatLogMessage& message);
    void insertFileAtIdx(ChatLogIdx idx, const FriendId& sender, const QString& senderName,
                         const ChatLogFile& file);

public slots:
    void onMessageReceived(const FriendId& sender, const Message& message);
    // 消息发送
    void onMessageSent(DispatchedMessageId id, const Message& message);
    // 消息发送成功
    void onMessageComplete(DispatchedMessageId id);
    // 消息接收
    void onMessageReceipt(DispatchedMessageId id);

    void onFileUpdated(const FriendId& sender, const ToxFile& file);
    void onFileCanceled(const FriendId& sender, const QString& fileId);

    void onFileTransferRemotePausedUnpaused(const FriendId& sender, const ToxFile& file,
                                            bool paused);
    void onFileTransferBrokenUnbroken(const FriendId& sender, const ToxFile& file, bool broken);
    ChatLogIdx getNextIdx(MsgId msgId);

private:
    const ICoreIdHandler& coreIdHandler;

    ChatLogIdx nextIdx = ChatLogIdx(-1);
    QMap<MsgId, ChatLogIdx> id2IdxMap;

    QMap<DispatchedMessageId, ChatLogIdx> outgoingMessages;
    std::map<ChatLogIdx, ChatLogItem> items;

    struct CurrentFileTransfer {
        ChatLogIdx idx;
        ToxFile file;
    };

    /**
     * Short list of active file transfers in given log. This is to make it
     * so we don't have to search through all files that have ever been transferred
     * in order to find our existing transfers
     */
    std::vector<CurrentFileTransfer> currentFileTransfers;

    Profile* mProfile;
};

#endif /*SESSION_CHAT_LOG_H*/
