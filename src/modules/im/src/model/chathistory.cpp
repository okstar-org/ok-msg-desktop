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

#include "chathistory.h"
#include "src/persistence/settings.h"
#include "src/widget/form/chatform.h"

namespace {
/**
 * @brief Determines if the given idx needs to be loaded from history
 * @param[in] idx index to check
 * @param[in] sessionChatLog SessionChatLog containing currently loaded items
 * @return True if load is needed
 */
bool needsLoadFromHistory(ChatLogIdx idx, const SessionChatLog& sessionChatLog) {
    return idx < sessionChatLog.getFirstIdx();
}

/**
 * @brief Finds the first item in sessionChatLog that contains a message
 * @param[in] sessionChatLog
 * @return index of first message
 */
ChatLogIdx findFirstMessage(const SessionChatLog& sessionChatLog) {
    auto it = sessionChatLog.getFirstIdx();
    while (it < sessionChatLog.getNextIdx()) {
        auto item = sessionChatLog.at(it);
        if (item && item->getContentType() == ChatLogItem::ContentType::message) {
            return it;
        }
        it++;
    }
    return ChatLogIdx(-1);
}

/**
 * @brief Handles presence of aciton prefix in content
 * @param[in/out] content
 * @return True if was an action
 */
bool handleActionPrefix(QString& content) {
    // Unfortunately due to legacy reasons we have to continue
    // inserting and parsing for ACTION_PREFIX in our messages even
    // though we have the ability to something more intelligent now
    // that we aren't owned by chatform logic
    auto isAction = content.startsWith(ChatForm::ACTION_PREFIX, Qt::CaseInsensitive);
    if (isAction) {
        content.remove(0, ChatForm::ACTION_PREFIX.size());
    }

    return isAction;
}
}  // namespace

ChatHistory::ChatHistory(const ContactId& f_,                  //
                         History* history_,                    //
                         const ICoreIdHandler& coreIdHandler,  //
                         const Settings& settings_,            //
                         IMessageDispatcher& messageDispatcher)
        : f(f_)
        , history(history_)
        , settings(settings_)
        , coreIdHandler(coreIdHandler)
        , sessionChatLog(getInitialChatLogIdx(), coreIdHandler) {
    // 消息发送成功
    connect(&messageDispatcher, &IMessageDispatcher::messageComplete, this,
            &ChatHistory::onMessageComplete);
    // 送达
    connect(&messageDispatcher, &IMessageDispatcher::messageReceipt, this,
            &ChatHistory::onMessageReceipt);

    // 消息接受
    connect(&messageDispatcher, &IMessageDispatcher::messageReceived, this,
            &ChatHistory::onMessageReceived);

    if (canUseHistory()) {
        // Defer messageSent callback until we finish firing off all our unsent messages.
        // If it was connected all our unsent messages would be re-added ot history again
        dispatchUnsentMessages(messageDispatcher);
    }

    // Now that we've fired off our unsent messages we can connect the message
    connect(&messageDispatcher, &IMessageDispatcher::messageSent, this,
            &ChatHistory::onMessageSent);

    connect(&messageDispatcher, &IMessageDispatcher::fileReceived, this,
            &ChatHistory::onFileUpdated);
    connect(&messageDispatcher, &IMessageDispatcher::fileReceived, &sessionChatLog,
            &SessionChatLog::onFileUpdated);
    connect(&messageDispatcher, &IMessageDispatcher::fileCancelled, this,
            &ChatHistory::onFileCanceled);
    connect(&messageDispatcher, &IMessageDispatcher::fileCancelled, &sessionChatLog,
            &SessionChatLog::onFileCanceled);

    // NOTE: this has to be done _after_ sending all sent messages since initial
    // state of the message has to be marked according to our dispatch state
    constexpr auto defaultNumMessagesToLoad = 100;
    auto firstChatLogIdx = sessionChatLog.getFirstIdx().get() < defaultNumMessagesToLoad
                                   ? ChatLogIdx(0)
                                   : sessionChatLog.getFirstIdx() - defaultNumMessagesToLoad;

    if (canUseHistory()) {
        loadHistoryIntoSessionChatLog(firstChatLogIdx);
    }

    // We don't manage any of the item updates ourselves, we just forward along
    // the underlying sessionChatLog's updates
    connect(&sessionChatLog, &IChatLog::itemUpdated, this, &IChatLog::itemUpdated);
}

ChatHistory::~ChatHistory() { qDebug() << __func__; }

const ChatLogItem* ChatHistory::at(ChatLogIdx idx) const {
    if (canUseHistory()) {
        ensureIdxInSessionChatLog(idx);
    }

    return sessionChatLog.at(idx);
}

QList<Message> ChatHistory::getLastTextMessage(uint size) {
    QList<Message> list;
    auto selfPk = coreIdHandler.getSelfId();
    auto messages = history->getLastMessageForFriend(selfPk, FriendId{f}, size,
                                                     HistMessageContentType::message);
    for (auto& i : messages) {
        Message msg = {.isAction = false,
                       .id = QString::number(i.id.get()),
                       .from = i.sender,
                       .to = i.receiver,
                       .content = i.asMessage(),
                       .timestamp = i.timestamp};
        list.append(msg);
    }

    return list;
}

SearchResult ChatHistory::searchForward(SearchPos startIdx, const QString& phrase,
                                        const ParameterSearch& parameter) const {
    if (startIdx.logIdx >= getNextIdx()) {
        SearchResult res;
        res.found = false;
        return res;
    }

    if (canUseHistory()) {
        ensureIdxInSessionChatLog(startIdx.logIdx);
    }

    return sessionChatLog.searchForward(startIdx, phrase, parameter);
}

SearchResult ChatHistory::searchBackward(SearchPos startIdx, const QString& phrase,
                                         const ParameterSearch& parameter) const {
    auto res = sessionChatLog.searchBackward(startIdx, phrase, parameter);

    if (res.found || !canUseHistory()) {
        return res;
    }

    auto earliestMessage = findFirstMessage(sessionChatLog);
    auto earliestMsg = sessionChatLog.at(earliestMessage);

    auto earliestMessageDate = (earliestMessage == ChatLogIdx(-1) || !earliestMsg)
                                       ? QDateTime::currentDateTime()
                                       : earliestMsg->getContentAsMessage().message.timestamp;

    // Roundabout way of getting the first idx but I don't want to have to
    // deal with re-implementing so we'll just piece what we want together...
    //
    // If the double disk access is real bad we can optimize this by adding
    // another function to history
    auto dateWherePhraseFound =
            history->getDateWhereFindPhrase(f.toString(), earliestMessageDate, phrase, parameter);

    auto loadIdx = history->getNumMessagesForFriendBeforeDate(coreIdHandler.getSelfId(),
                                                              FriendId{f}, dateWherePhraseFound);
    loadHistoryIntoSessionChatLog(ChatLogIdx(loadIdx));

    // Reset search pos to the message we just loaded to avoid a double search
    startIdx.logIdx = ChatLogIdx(loadIdx);
    startIdx.numMatches = 0;
    return sessionChatLog.searchBackward(startIdx, phrase, parameter);
}

ChatLogIdx ChatHistory::getFirstIdx() const {
    if (canUseHistory()) {
        return ChatLogIdx(0);
    } else {
        return sessionChatLog.getFirstIdx();
    }
}

ChatLogIdx ChatHistory::getNextIdx() const { return sessionChatLog.getNextIdx(); }

std::vector<IChatLog::DateChatLogIdxPair>  //
ChatHistory::getDateIdxs(const QDate& startDate, size_t maxDates) const {
    if (canUseHistory()) {
        auto counts = history->getNumMessagesForFriendBeforeDateBoundaries(FriendId(f), startDate,
                                                                           maxDates);

        std::vector<IChatLog::DateChatLogIdxPair> ret;
        std::transform(counts.begin(), counts.end(), std::back_inserter(ret),
                       [&](const History::DateIdx& historyDateIdx) {
                           DateChatLogIdxPair pair;
                           pair.date = historyDateIdx.date;
                           pair.idx.get() = historyDateIdx.numMessagesIn;
                           return pair;
                       });

        // Do not re-search in the session chat log. If we have history the query to the history
        // should have been sufficient
        return ret;
    } else {
        return sessionChatLog.getDateIdxs(startDate, maxDates);
    }
}

void ChatHistory::onFileUpdated(const FriendId& sender, const ToxFile& file) {
    qDebug() << __func__ << "friendId:" << sender.toString() << file.fileName;

    if (canUseHistory()) {
        switch (file.status) {
            case FileStatus::INITIALIZING: {
                history->addNewFileMessage(f.toString(), file, sender.toString(),
                                           QDateTime::currentDateTime(), {});
                break;
            }
            case FileStatus::CANCELED:
            case FileStatus::FINISHED:
            case FileStatus::BROKEN: {
                //            const bool isSuccess = file.status == FileStatus::FINISHED;
                history->setFileMessage(file);
                break;
            }
            case FileStatus::PAUSED:
            case FileStatus::TRANSMITTING:
            default:
                break;
        }
    }

    sessionChatLog.onFileUpdated(sender, file);
}

void ChatHistory::onFileCanceled(const FriendId& sender, const QString& fileId) {
    qDebug() << __func__ << "fileId:" << fileId;
    auto files = history->getMessageByDataId(fileId);
    for (auto f : files) {
        auto ff = f.asFile();
        ff.status = FileStatus::CANCELED;
        history->setFileMessage(ToxFile(ff));
    }
}

void ChatHistory::onFileTransferRemotePausedUnpaused(const FriendId& sender, const ToxFile& file,
                                                     bool paused) {
    sessionChatLog.onFileTransferRemotePausedUnpaused(sender, file, paused);
}

void ChatHistory::onFileTransferBrokenUnbroken(const FriendId& sender, const ToxFile& file,
                                               bool broken) {
    sessionChatLog.onFileTransferBrokenUnbroken(sender, file, broken);
}

void ChatHistory::onMessageReceived(const FriendId& sender, const Message& message) {
    qDebug() << __func__ << "sender:" << sender.toString() << " from:" << message.from;
    auto selfId = coreIdHandler.getSelfPeerId().toString();

    if (selfId == message.from) {
        qWarning() << "Is self message.";
        return;
    }

    if (canUseHistory()) {
        //        auto friendPk = f.getPublicKey().toString();
        //        auto displayName = f.getDisplayedName();
        auto content = message.content;
        if (message.isAction) {
            content = ChatForm::ACTION_PREFIX + content;
        }
        history->addNewMessage(message, HistMessageContentType::message, true);
    }

    sessionChatLog.onMessageReceived(sender, message);
}

void ChatHistory::onMessageSent(DispatchedMessageId id, const Message& message) {
    if (canUseHistory()) {
        auto selfPk = coreIdHandler.getSelfId().toString();
        auto friendPk = f.toString();

        auto content = message.content;
        if (message.isAction) {
            content = ChatForm::ACTION_PREFIX + content;
        }

        auto username = coreIdHandler.getUsername();

        auto onInsertion = [this, id](RowId historyId) { handleDispatchedMessage(id, historyId); };

        history->addNewMessage(message, HistMessageContentType::message, false, onInsertion);
    }

    sessionChatLog.onMessageSent(id, message);
}

void ChatHistory::onMessageComplete(DispatchedMessageId id) {
    if (canUseHistory()) {
        completeMessage(id);
    }

    sessionChatLog.onMessageComplete(id);
}

void ChatHistory::onMessageReceipt(DispatchedMessageId id) {
    qDebug() << __func__ << id.get();
    if (canUseHistory()) {
        receiptMessage(id);
    }
    sessionChatLog.onMessageReceipt(id);
}

/**
 * @brief Forces the given index and all future indexes to be in the chatlog
 * @param[in] idx
 * @note Marked const since this doesn't change _external_ state of the class. We
     still have all the same items at all the same indexes, we've just stuckem
     in ram
 */
void ChatHistory::ensureIdxInSessionChatLog(ChatLogIdx idx) const {
    if (needsLoadFromHistory(idx, sessionChatLog)) {
        loadHistoryIntoSessionChatLog(idx);
    }
}
/**
 * @brief Unconditionally loads the given index and all future messages that
 * are not in the session chat log into the session chat log
 * @param[in] idx
 * @note Marked const since this doesn't change _external_ state of the class. We
   still have all the same items at all the same indexes, we've just stuckem
   in ram
 * @note no end idx as we always load from start -> latest. In the future we
 * could have a less contiguous history
 */
void ChatHistory::loadHistoryIntoSessionChatLog(ChatLogIdx start) const {
    if (!needsLoadFromHistory(start, sessionChatLog)) {
        return;
    }

    auto core = Core::getInstance();

    auto end = sessionChatLog.getFirstIdx();

    // We know that both history and us have a start index of 0 so the type
    // conversion should be safe
    assert(getFirstIdx() == ChatLogIdx(0));

    auto messages =
            history->getMessagesForFriend(core->getSelfId(), FriendId(f), start.get(), end.get());
    qDebug() << "load message for:" << f.toString() << "messages:" << messages.size();

    //  assert(messages.size() == end.get() - start.get());
    ChatLogIdx nextIdx = start;

    for (const auto& message : messages) {
        // Note that message.id is _not_ a valid conversion here since it is a
        // global id not a per-chat id like the ChatLogIdx
        auto currentIdx = nextIdx++;

        auto sender = ToxId(message.sender).getPublicKey();
        //        auto frnd =
        //        Nexus::getCore()->getFriendList().findFriend(ContactId{message.sender});
        auto dispName = sender.username;
        const auto date = message.timestamp;

        switch (message.type) {
            case HistMessageContentType::file: {
                auto file = message.asFile();
                auto tfile = ToxFile{file};
                tfile.receiver = message.receiver;
                tfile.sender = message.sender;
                auto chatLogFile = ChatLogFile{date, tfile};
                sessionChatLog.insertFileAtIdx(currentIdx, sender, dispName, chatLogFile);
                break;
            }
            case HistMessageContentType::message: {
                QString messageContent = message.asMessage();

                auto isAction = handleActionPrefix(messageContent);

                auto processedMessage = Message{.isAction = isAction,
                                                .from = message.sender,
                                                .to = message.receiver,
                                                .content = messageContent,
                                                .timestamp = message.timestamp};

                auto dispatchedMessageIt = std::find_if(
                        dispatchedMessageRowIdMap.begin(), dispatchedMessageRowIdMap.end(),
                        [&](RowId dispatchedId) { return dispatchedId == message.id; });

                //            assert((message.state != MessageState::pending && dispatchedMessageIt
                //            == dispatchedMessageRowIdMap.end()) ||
                //                   (message.state == MessageState::pending && dispatchedMessageIt
                //                   != dispatchedMessageRowIdMap.end()));

                auto chatLogMessage = ChatLogMessage{message.state, processedMessage};
                switch (message.state) {
                    case MessageState::complete:
                        sessionChatLog.insertCompleteMessageAtIdx(currentIdx, sender, dispName,
                                                                  chatLogMessage);
                        break;
                    case MessageState::pending:
                        sessionChatLog.insertIncompleteMessageAtIdx(currentIdx, sender, dispName,
                                                                    chatLogMessage,
                                                                    dispatchedMessageIt.key());
                        break;
                    case MessageState::broken:
                        sessionChatLog.insertBrokenMessageAtIdx(currentIdx, sender, dispName,
                                                                chatLogMessage);
                        break;
                }
                break;
            }
        }
    }

    //    assert(nextIdx == end);
}

/**
 * @brief Sends any unsent messages in history to the underlying message dispatcher
 * @param[in] messageDispatcher
 */
void ChatHistory::dispatchUnsentMessages(IMessageDispatcher& messageDispatcher) {
    auto core = Core::getInstance();
    auto unsentMessages = history->getUndeliveredMessagesForFriend(core->getSelfId(), FriendId(f));
    for (auto& message : unsentMessages) {
        if (message.type != HistMessageContentType::message) continue;

        auto messageContent = message.asMessage();
        auto isAction = handleActionPrefix(messageContent);

        // NOTE: timestamp will be generated in messageDispatcher but we haven't
        // hooked up our history callback so it will not be shown in our chatlog
        // with the new timestamp. This is intentional as everywhere else we use
        // attempted send time (which is whenever the it was initially inserted
        // into history
        auto dispatchIds = messageDispatcher.sendMessage(isAction, messageContent);

        // We should only send a single message, but in the odd case where we end
        // up having to split more than when we added the message to history we'll
        // just associate the last dispatched id with the history message
        handleDispatchedMessage(dispatchIds.first, message.id);

        // We don't add the messages to the underlying chatlog since
        // 1. We don't even know the ChatLogIdx of this message
        // 2. We only want to display the latest N messages on boot by default,
        //    even if there are more than N messages that haven't been sent
    }
}

void ChatHistory::handleDispatchedMessage(DispatchedMessageId dispatchId, RowId historyId) {
    qDebug() << "handleDispatchedMessage dispatchId:" << dispatchId.get() << "historyId"
             << historyId.get();

    auto completedMessageIt = completedMessages.find(dispatchId);
    if (completedMessageIt == completedMessages.end()) {
        dispatchedMessageRowIdMap.insert(dispatchId, historyId);
    } else {
        history->markAsDelivered(historyId);
        completedMessages.erase(completedMessageIt);
    }
}

void ChatHistory::completeMessage(DispatchedMessageId id) {
    auto dispatchedMessageIt = dispatchedMessageRowIdMap.find(id);
    if (dispatchedMessageIt == dispatchedMessageRowIdMap.end()) {
        completedMessages.insert(id);
    } else {
        history->markAsDelivered(*dispatchedMessageIt);
        //        dispatchedMessageRowIdMap.erase(dispatchedMessageIt);
    }
}

void ChatHistory::receiptMessage(DispatchedMessageId id) {
    auto dispatchedMessageIt = dispatchedMessageRowIdMap.find(id);
    if (dispatchedMessageIt != dispatchedMessageRowIdMap.end()) {
        history->markAsReceipt(*dispatchedMessageIt);
        dispatchedMessageRowIdMap.erase(dispatchedMessageIt);
    }
}

bool ChatHistory::canUseHistory() const { return history && settings.getEnableLogging(); }

/**
 * @brief Gets the initial chat log index for a sessionChatLog with 0 items loaded from history.
 * Needed to keep history indexes in sync with chat log indexes
 * @param[in] history
 * @param[in] f
 * @return Initial chat log index
 */
ChatLogIdx ChatHistory::getInitialChatLogIdx() const {
    if (canUseHistory()) {
        return ChatLogIdx(history->getNumMessagesForFriend(coreIdHandler.getSelfId(), FriendId(f)));
    }
    return ChatLogIdx(0);
}
