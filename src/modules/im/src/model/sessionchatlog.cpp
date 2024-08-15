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

#include "sessionchatlog.h"

#include <QDebug>
#include <QtGlobal>
#include <mutex>

#include "Bus.h"
#include "application.h"
#include "src/friendlist.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"

#include <lib/ortc/webrtc/Instance.h>

namespace {

/**
 * lower_bound needs two way comparisons. This adaptor allows us to compare
 * between a Message and QDateTime in both directions
 */
struct MessageDateAdaptor {
    static const QDateTime invalidDateTime;
    MessageDateAdaptor(const std::pair<const ChatLogIdx, ChatLogItem>& item)
            : timestamp(item.second.getContentType() == ChatLogItem::ContentType::message
                                ? item.second.getContentAsMessage().message.timestamp
                                : invalidDateTime) {}

    MessageDateAdaptor(const QDateTime& timestamp) : timestamp(timestamp) {}

    const QDateTime& timestamp;
};

const QDateTime MessageDateAdaptor::invalidDateTime;

/**
 * @brief The search types all can be represented as some regular expression.
 * This function takes the input phrase and filter and generates the appropriate
 * regular expression
 * @return Regular expression which finds the input
 */
QRegularExpression getRegexpForPhrase(const QString& phrase, FilterSearch filter) {
    constexpr auto regexFlags = QRegularExpression::UseUnicodePropertiesOption;
    constexpr auto caseInsensitiveFlags = QRegularExpression::CaseInsensitiveOption;

    switch (filter) {
        case FilterSearch::Register:
            return QRegularExpression(QRegularExpression::escape(phrase), regexFlags);
        case FilterSearch::WordsOnly:
            return QRegularExpression(SearchExtraFunctions::generateFilterWordsOnly(phrase),
                                      caseInsensitiveFlags);
        case FilterSearch::RegisterAndWordsOnly:
            return QRegularExpression(SearchExtraFunctions::generateFilterWordsOnly(phrase),
                                      regexFlags);
        case FilterSearch::RegisterAndRegular:
            return QRegularExpression(phrase, regexFlags);
        case FilterSearch::Regular:
            return QRegularExpression(phrase, caseInsensitiveFlags);
        default:
            return QRegularExpression(QRegularExpression::escape(phrase), caseInsensitiveFlags);
    }
}

/**
 * @return True if the given status indicates no future updates will come in
 */
bool toxFileIsComplete(FileStatus status) {
    switch (status) {
        case FileStatus::INITIALIZING:
        case FileStatus::PAUSED:
        case FileStatus::TRANSMITTING:
            return false;
        case FileStatus::BROKEN:
        case FileStatus::CANCELED:
        case FileStatus::FINISHED:
        default:
            return true;
    }
}

std::map<ChatLogIdx, ChatLogItem>::const_iterator firstItemAfterDate(
        QDate date, const std::map<ChatLogIdx, ChatLogItem>& items) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    return std::lower_bound(items.begin(), items.end(), QDateTime(date.startOfDay()),
#else
    return std::lower_bound(items.begin(), items.end(), QDateTime(date),
#endif
                            [](const MessageDateAdaptor& a, MessageDateAdaptor const& b) {
                                return a.timestamp.date() < b.timestamp.date();
                            });
}
}  // namespace

SessionChatLog::SessionChatLog(const ICoreIdHandler& coreIdHandler)
        : coreIdHandler(coreIdHandler) {}

/**
 * @brief Alternate constructor that allows for an initial index to be set
 */
SessionChatLog::SessionChatLog(ChatLogIdx initialIdx, const ICoreIdHandler& coreIdHandler)
        : coreIdHandler(coreIdHandler), nextIdx(initialIdx), mProfile{nullptr} {
    connect(ok::Application::Instance()->bus(), &ok::Bus::profileChanged, this,
            [&](Profile* profile) { mProfile = profile; });
}

SessionChatLog::~SessionChatLog() { qDebug() << __func__; }

const ChatLogItem* SessionChatLog::at(ChatLogIdx idx) const {
    auto it = items.find(idx);
    if (it == items.end()) {
        qWarning() << "Unable to find ChatLogItem:" << idx.get();
        return nullptr;
    }
    return &(it->second);
}

SearchResult SessionChatLog::searchForward(SearchPos startPos, const QString& phrase,
                                           const ParameterSearch& parameter) const {
    if (startPos.logIdx >= getNextIdx()) {
        SearchResult res;
        res.found = false;
        return res;
    }

    auto currentPos = startPos;

    auto regexp = getRegexpForPhrase(phrase, parameter.filter);

    for (auto it = items.find(currentPos.logIdx); it != items.end(); ++it) {
        const auto& key = it->first;
        const auto& item = it->second;

        if (item.getContentType() != ChatLogItem::ContentType::message) {
            continue;
        }

        const auto& content = item.getContentAsMessage();

        auto match = regexp.globalMatch(content.message.content, 0);

        auto numMatches = 0;
        QRegularExpressionMatch lastMatch;
        while (match.isValid() && numMatches <= currentPos.numMatches && match.hasNext()) {
            lastMatch = match.next();
            numMatches++;
        }

        if (numMatches > currentPos.numMatches) {
            SearchResult res;
            res.found = true;
            res.pos.logIdx = key;
            res.pos.numMatches = numMatches;
            res.start = lastMatch.capturedStart();
            res.len = lastMatch.capturedLength();
            return res;
        }

        // After the first iteration we force this to 0 to search the whole
        // message
        currentPos.numMatches = 0;
    }

    // We should have returned from the above loop if we had found anything
    SearchResult ret;
    ret.found = false;
    return ret;
}

SearchResult SessionChatLog::searchBackward(SearchPos startPos, const QString& phrase,
                                            const ParameterSearch& parameter) const {
    auto currentPos = startPos;
    auto regexp = getRegexpForPhrase(phrase, parameter.filter);
    auto startIt = items.find(currentPos.logIdx);

    // If we don't have it we'll start at the end
    if (startIt == items.end()) {
        if (items.empty()) {
            SearchResult ret;
            ret.found = false;
            return ret;
        }
        startIt = std::prev(items.end());
        startPos.numMatches = 0;
    }

    // Off by 1 due to reverse_iterator api
    auto rStartIt = std::reverse_iterator<decltype(startIt)>(std::next(startIt));
    auto rEnd = std::reverse_iterator<decltype(startIt)>(items.begin());

    for (auto it = rStartIt; it != rEnd; ++it) {
        const auto& key = it->first;
        const auto& item = it->second;

        if (item.getContentType() != ChatLogItem::ContentType::message) {
            continue;
        }

        const auto& content = item.getContentAsMessage();
        auto match = regexp.globalMatch(content.message.content, 0);

        auto totalMatches = 0;
        auto numMatchesBeforePos = 0;
        QRegularExpressionMatch lastMatch;
        while (match.isValid() && match.hasNext()) {
            auto currentMatch = match.next();
            totalMatches++;
            if (currentPos.numMatches == 0 || currentPos.numMatches > numMatchesBeforePos) {
                lastMatch = currentMatch;
                numMatchesBeforePos++;
            }
        }

        if ((numMatchesBeforePos < currentPos.numMatches || currentPos.numMatches == 0) &&
            numMatchesBeforePos > 0) {
            SearchResult res;
            res.found = true;
            res.pos.logIdx = key;
            res.pos.numMatches = numMatchesBeforePos;
            res.start = lastMatch.capturedStart();
            res.len = lastMatch.capturedLength();
            return res;
        }

        // After the first iteration we force this to 0 to search the whole
        // message
        currentPos.numMatches = 0;
    }

    // We should have returned from the above loop if we had found anything
    SearchResult ret;
    ret.found = false;
    return ret;
}

ChatLogIdx SessionChatLog::getFirstIdx() const {
    if (items.empty()) {
        return nextIdx;
    }

    return items.begin()->first;
}

ChatLogIdx SessionChatLog::getNextIdx() const { return nextIdx; }

std::vector<IChatLog::DateChatLogIdxPair> SessionChatLog::getDateIdxs(const QDate& startDate,
                                                                      size_t maxDates) const {
    std::vector<DateChatLogIdxPair> ret;
    auto dateIt = startDate;

    while (true) {
        auto it = firstItemAfterDate(dateIt, items);

        if (it == items.end()) {
            break;
        }

        DateChatLogIdxPair pair;
        pair.date = dateIt;
        pair.idx = it->first;

        ret.push_back(std::move(pair));

        dateIt = dateIt.addDays(1);
        if (startDate.daysTo(dateIt) > maxDates && maxDates != 0) {
            break;
        }
    }

    return ret;
}

void SessionChatLog::insertCompleteMessageAtIdx(ChatLogIdx idx,
                                                const FriendId& sender,
                                                const QString& senderName,
                                                const ChatLogMessage& message) {
    auto item = ChatLogItem(sender, senderName, message);
    assert(message.state == MessageState::complete);
    items.emplace(idx, std::move(item));
}

void SessionChatLog::insertIncompleteMessageAtIdx(ChatLogIdx idx, const FriendId& sender,
                                                  const QString& senderName,
                                                  const ChatLogMessage& message,
                                                  DispatchedMessageId dispatchId) {
    auto item = ChatLogItem(sender, senderName, message);
    assert(message.state == MessageState::pending);

    items.emplace(idx, std::move(item));
    outgoingMessages.insert(dispatchId, idx);
}

void SessionChatLog::insertBrokenMessageAtIdx(ChatLogIdx idx,
                                              const FriendId& sender,
                                              const QString& senderName,
                                              const ChatLogMessage& message) {
    auto item = ChatLogItem(sender, senderName, message);
    assert(message.state == MessageState::broken);

    items.emplace(idx, std::move(item));
}

void SessionChatLog::insertFileAtIdx(ChatLogIdx idx, const FriendId& sender,
                                     const QString& senderName, const ChatLogFile& file) {
    auto item = ChatLogItem(sender, senderName, file);
    items.emplace(idx, std::move(item));
}

/**
 * @brief Inserts message data into the chatlog buffer
 * @note Owner of SessionChatLog is in charge of attaching this to the
 * appropriate IMessageDispatcher
 */
void SessionChatLog::onMessageReceived(const FriendId& sender, const Message& message) {
    qDebug() << __func__ << "msgId:" << message.id;
    qDebug() << "sender:" << sender.toString();
    qDebug() << "from:" << message.from;
    qDebug() << "displayName:" << message.displayName;
    qDebug() << "msg:" << message.content;

    auto messageIdx = getNextIdx(message.id);

    ChatLogMessage chatLogMessage;
    chatLogMessage.state = MessageState::complete;
    chatLogMessage.message = message;

    FriendId pk(message.from);
    items.emplace(messageIdx, ChatLogItem(pk, message.displayName, chatLogMessage));

    emit itemUpdated(messageIdx);
}

/**
 * @brief Inserts message data into the chatlog buffer
 * @note Owner of SessionChatLog is in charge of attaching this to the
 * appropriate IMessageDispatcher
 */
void SessionChatLog::onMessageSent(DispatchedMessageId dispatchedId, const Message& message) {
    qDebug() << __func__ << "dispatchedId:" << dispatchedId.get() << "msg:" << message.content;

    auto msgLogIdx = getNextIdx(message.id);

    ChatLogMessage chatLogMessage;
    chatLogMessage.state = MessageState::pending;
    chatLogMessage.message = message;

    items.emplace(msgLogIdx, ChatLogItem(coreIdHandler.getSelfId(),
                                         // 发送人名称就算自己的昵称
                                         coreIdHandler.getNick(),
                                         chatLogMessage));

    outgoingMessages.insert(dispatchedId, msgLogIdx);

    emit itemUpdated(msgLogIdx);
}

/**
 * @brief Marks the associated message as complete and notifies any listeners
 * @note Owner of SessionChatLog is in charge of attaching this to the
 * appropriate IMessageDispatcher
 */
void SessionChatLog::onMessageComplete(DispatchedMessageId id) {
    qDebug() << __func__ << "dispatchedMessageId:" << id.get();

    auto chatLogIdxIt = outgoingMessages.find(id);
    if (chatLogIdxIt == outgoingMessages.end()) {
        qWarning() << "Failed to find outgoing message";
        return;
    }

    const auto& chatLogIdx = *chatLogIdxIt;
    auto messageIt = items.find(chatLogIdx);
    if (messageIt == items.end()) {
        qWarning() << "Failed to look up message in chat log";
        return;
    }

    messageIt->second.getContentAsMessage().state = MessageState::complete;
    emit this->itemUpdated(messageIt->first);
}

void SessionChatLog::onMessageReceipt(DispatchedMessageId id) {
    qDebug() << __func__ << "dispatchedMessageId:" << id.get();
    auto chatLogIdxIt = outgoingMessages.find(id);
    if (chatLogIdxIt == outgoingMessages.end()) {
        qWarning() << "Failed to find outgoing message";
        return;
    }

    const auto& chatLogIdx = *chatLogIdxIt;
    auto messageIt = items.find(chatLogIdx);
    if (messageIt == items.end()) {
        qWarning() << "Failed to look up message in chat log";
        return;
    }
    messageIt->second.getContentAsMessage().state = MessageState::receipt;
    emit this->itemUpdated(messageIt->first);
}

/**
 * @brief Updates file state in the chatlog
 * @note The files need to be pre-filtered for the current chat since we do no
 * validation
 * @note This should be attached to any CoreFile signal that fits the signature
 */
void SessionChatLog::onFileUpdated(const FriendId& friendId, const ToxFile& file) {
    qDebug() << __func__ << "friendId:" << friendId.toString() << "file" << file.fileName;

    auto fileIt = std::find_if(
            currentFileTransfers.begin(), currentFileTransfers.end(),
            [&](const CurrentFileTransfer& transfer) { return transfer.file == file; });

    ChatLogIdx messageIdx;
    if (fileIt == currentFileTransfers.end() && file.status == FileStatus::INITIALIZING) {
        assert(file.status == FileStatus::INITIALIZING);
        CurrentFileTransfer currentTransfer;
        currentTransfer.file = file;
        currentTransfer.idx = nextIdx++;
        currentFileTransfers.push_back(currentTransfer);

        const auto chatLogFile = ChatLogFile{QDateTime::currentDateTime(), file};

        QString senderName;
        FriendId senderId{file.sender};
        auto frnd = Nexus::getCore()->getFriendList().findFriend(senderId);
        if (frnd) {
            senderName = frnd->getDisplayedName();
        }
        items.emplace(currentTransfer.idx, ChatLogItem(senderId, senderName, chatLogFile));
        messageIdx = currentTransfer.idx;
    } else if (fileIt != currentFileTransfers.end()) {
        messageIdx = fileIt->idx;
        fileIt->file = file;

        items.at(messageIdx).getContentAsFile().file = file;
    } else {
        // This may be a file unbroken message that we don't handle ATM
        return;
    }

    if (toxFileIsComplete(file.status)) {
        currentFileTransfers.erase(fileIt);
    }

    qDebug() << "file messageIdx" << messageIdx.get();
    emit this->itemUpdated(messageIdx);
}

void SessionChatLog::onFileCanceled(const FriendId& sender, const QString& fileId) {
    qDebug() << __func__ << fileId;

    ChatLogIdx messageIdx;

    if (currentFileTransfers.empty()) {
        // db
        for (auto& item : items) {
            if (item.second.getContentType() == ChatLogItem::ContentType::fileTransfer) {
                auto f = item.second.getContentAsFile();
                if (f.file.fileId == fileId) {
                    messageIdx = item.first;
                    break;
                }
            }
        }
    } else {
        // cache
        auto fileIt = std::find_if(currentFileTransfers.begin(), currentFileTransfers.end(),
                                   [&](const CurrentFileTransfer& transfer) {
                                       return transfer.file.fileId == fileId;
                                   });
        if (fileIt != currentFileTransfers.end()) {
            messageIdx = fileIt->idx;
        }
    }

    // Update status to canceled
    qDebug() << "messageIdx" << messageIdx.get();
    items.at(messageIdx).getContentAsFile().file.status = FileStatus::CANCELED;
    emit this->itemUpdated(messageIdx);
}

void SessionChatLog::onFileTransferRemotePausedUnpaused(const FriendId& sender,
                                                        const ToxFile& file,
                                                        bool /*paused*/) {
    onFileUpdated(sender, file);
}

void SessionChatLog::onFileTransferBrokenUnbroken(const FriendId& sender,
                                                  const ToxFile& file,
                                                  bool /*broken*/) {
    onFileUpdated(sender, file);
}

ChatLogIdx SessionChatLog::getNextIdx(MsgId msgId) {
    assert(!msgId.isEmpty());

    auto idx = id2IdxMap.value(msgId, ChatLogIdx(-1));
    if (idx.get() == -1) {
        idx = nextIdx++;
        id2IdxMap.insert(msgId, idx);
    }

    qDebug() << "make next msgId:" << msgId << " idx:" << idx.get();
    return idx;
}
