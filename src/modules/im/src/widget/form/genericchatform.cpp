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

#include "genericchatform.h"

#include <base/uuid.h>
#include <src/chatlog/chatmessageitem.h>
#include <QClipboard>
#include <QFileDialog>
#include <QKeyEvent>

#include <QSplitter>
#include <QStringBuilder>
#include <QTemporaryFile>
#include <QtGlobal>
#include "ChatInputForm.h"
#include "base/MessageBox.h"
#include "base/files.h"
#include "base/images.h"
#include "lib/settings/translator.h"
#include "src/chatlog/chatlinecontentproxy.h"
#include "src/chatlog/chatlog.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "src/chatlog/content/simpletext.h"
#include "src/chatlog/content/timestamp.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/video/genericnetcamview.h"
#include "src/widget/contentdialog.h"
#include "src/widget/contentlayout.h"
#include "src/widget/emoticonswidget.h"
#include "src/widget/form/chatform.h"
#include "src/widget/form/loadhistorydialog.h"
#include "src/widget/gui.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/tool/flyoutoverlaywidget.h"
#include "src/widget/widget.h"

/**
 * @class GenericChatForm
 * @brief Parent class for all chatforms. It's provide the minimum required UI
 * elements and methods to work with chat messages.
 */

static const QSize FILE_FLYOUT_SIZE{24, 24};
static const short MAIN_FOOT_MARGIN = 8;
static const short EDIT_SEND_SPACING = 5;
static constexpr int TYPING_NOTIFICATION_DURATION = 3000;

/**
 * @brief Searches for name (possibly alias) of someone with specified public
 * key among all of your friends or groups you are participated
 * @param pk Searched public key
 * @return Name or alias of someone with such public key, or public key string
 * representation if no one was found
 */
QString GenericChatForm::resolveToxPk(const FriendId& pk) {
    Friend* f = Nexus::getCore()->getFriendList().findFriend(pk);
    if (f) {
        return f->getDisplayedName();
    }

    //  for (Group *it : GroupList::getAllGroups()) {
    //    QString res = it->resolveToxId(pk);
    //    if (!res.isEmpty()) {
    //      return res;
    //    }
    //  }

    return pk.toString();
}

namespace {
const QString STYLE_PATH = QStringLiteral("chatForm/buttons.css");
}

IChatItem::Ptr GenericChatForm::getChatMessageForIdx(
        ChatLogIdx idx, const std::map<ChatLogIdx, IChatItem::Ptr>& messages) {
    auto existingMessageIt = messages.find(idx);
    if (existingMessageIt == messages.end()) {
        return IChatItem::Ptr();
    }
    return existingMessageIt->second;
}

bool GenericChatForm::shouldRenderDate(ChatLogIdx idxToRender, const IChatLog& chatLog) {
    if (idxToRender.get() == chatLog.getFirstIdx().get()) return true;

    auto prev = chatLog.at(idxToRender - 1);
    auto cur = chatLog.at(idxToRender);
    if (!prev || !cur) return false;

    return prev->getTimestamp().date() != cur->getTimestamp().date();
}

IChatItem::Ptr GenericChatForm::dateMessageForItem(const ChatLogItem& item) {
    const auto& s = Settings::getInstance();
    const auto date = item.getTimestamp().date();
    auto dateText = date.toString(s.getDateFormat());
    return ChatMessage::createChatInfoMessage(dateText, ChatMessage::INFO, QDateTime());
}

void GenericChatForm::renderMessage(const ChatLogItem& item, bool isSelf, bool colorizeNames,
                                    const ChatLogMessage& chatLogMessage,
                                    IChatItem::Ptr& chatMessage) {
    if (chatMessage) {
        if (chatLogMessage.state == MessageState::complete) {
            chatMessage->markAsDelivered(chatLogMessage.message.timestamp);
        }
    } else {
        chatMessage = createMessage(item, isSelf, colorizeNames, chatLogMessage);
    }
}

void GenericChatForm::renderFile(const ChatLogItem& item, ToxFile file, bool isSelf,
                                 QDateTime timestamp, IChatItem::Ptr& chatMessage) {
    qDebug() << __func__ << "file" << file.fileName;

    if (!chatMessage) {
        chatMessage = ChatMessage::createFileTransferMessage(item, file, isSelf, timestamp);
    } else {
        auto proxy = static_cast<ChatLineContentProxy*>(chatMessage->centerContent());
        if (proxy->getWidgetType() == ChatLineContentProxy::FileTransferWidgetType) {
            auto ftWidget = static_cast<FileTransferWidget*>(proxy->getWidget());
            ftWidget->onFileTransferUpdate(file);
            qDebug() << "update file" << file.fileName;
        }
    }
}

void GenericChatForm::renderItem(const ChatLogItem& item,
                                 bool hideName,
                                 bool colorizeNames,
                                 IChatItem::Ptr& chatMessage) {
    const Core* core = Core::getInstance();

    const auto& sender = item.getSender();
    const auto& selfPk = core->getSelfPeerId().getPublicKey();

    bool isSelf = sender == selfPk;  // || sender.getResource() == selfPk.getUsername();

    switch (item.getContentType()) {
        case ChatLogItem::ContentType::message: {
            const auto& chatLogMessage = item.getContentAsMessage();
            renderMessage(item, isSelf, colorizeNames, chatLogMessage, chatMessage);
            break;
        }
        case ChatLogItem::ContentType::fileTransfer: {
            const auto& file = item.getContentAsFile();
            renderFile(item, file.file, isSelf, item.getTimestamp(), chatMessage);
            break;
        }
    }
}

ChatLogIdx GenericChatForm::firstItemAfterDate(QDate date, const IChatLog& chatLog) {
    auto idxs = chatLog.getDateIdxs(date, 1);
    if (idxs.size()) {
        return idxs[0].idx;
    } else {
        return chatLog.getNextIdx();
    }
}

GenericChatForm::GenericChatForm(const ContactId* contact_,
                                 IChatLog& iChatLog_,
                                 IMessageDispatcher& messageDispatcher,
                                 QWidget* parent)
        : QWidget(parent, Qt::Window)
        , contactId(contact_)
        , contact(nullptr)
        , audioInputFlag(false)
        , audioOutputFlag(false)
        , iChatLog(iChatLog_)
        , messageDispatcher(messageDispatcher)
        , isTyping{false} {
    qDebug() << __func__ << "contact:" << contact_;

    setContentsMargins(0, 0, 0, 0);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    bodySplitter = new QSplitter(Qt::Vertical, this);
    mainLayout->addWidget(bodySplitter);

    // 聊天框
    chatLog = new ChatLog(this);
    //    chatLog->setMinimumHeight(200);
    chatLog->setBusyNotification(ChatMessage::createBusyNotification());

    connect(chatLog, &ChatLog::firstVisibleLineChanged, this, &GenericChatForm::updateShowDateInfo);
    connect(chatLog, &ChatLog::loadHistoryLower, this, &GenericChatForm::loadHistoryLower);

    bodySplitter->addWidget(chatLog);

    // 输入框
    inputForm = new ChatInputForm(this);
    connect(inputForm, &ChatInputForm::inputText, this, &GenericChatForm::onTextSend);
    connect(inputForm, &ChatInputForm::inputTextChanged, this, &GenericChatForm::onTextEditChanged);
    connect(inputForm, &ChatInputForm::inputFile, this, &GenericChatForm::onFileSend);
    connect(inputForm, &ChatInputForm::inputScreenCapture, this, &GenericChatForm::onImageSend);
    bodySplitter->addWidget(inputForm);

    bodySplitter->setSizes({120, 120});
    bodySplitter->setStretchFactor(0, 1);
    bodySplitter->setStretchFactor(1, 0);
    bodySplitter->setChildrenCollapsible(false);

    // settings
    auto& s = Settings::getInstance();

    connect(&s, &Settings::chatMessageFontChanged, this,
            &GenericChatForm::onChatMessageFontChanged);
    //  dateInfo = new QLabel(this);
    //  dateInfo->setAlignment(Qt::AlignHCenter);
    //  dateInfo->setVisible(false);

    // menu.addSeparator();

    // searchAction =
    //     menu.addAction(QIcon(), QString(), this, SLOT(searchFormShow()),
    //                    QKeySequence(Qt::CTRL + Qt::Key_F));
    // addAction(searchAction);



    connect(&iChatLog, &IChatLog::itemUpdated, this, &GenericChatForm::renderMessage0);

    // connect(searchForm, &SearchForm::searchInBegin, this,
    //         &GenericChatForm::searchInBegin);
    // connect(searchForm, &SearchForm::searchUp, this,
    //         &GenericChatForm::onSearchUp);
    // connect(searchForm, &SearchForm::searchDown, this,
    //         &GenericChatForm::onSearchDown);
    // connect(searchForm, &SearchForm::visibleChanged, this,
    //         &GenericChatForm::onSearchTriggered);
    // connect(this, &GenericChatForm::messageNotFoundShow, searchForm,
    //         &SearchForm::showMessageNotFound);

    connect(&GUI::getInstance(), &GUI::themeApplyRequest, this, &GenericChatForm::reloadTheme);

    auto chatLogIdxRange = iChatLog.getNextIdx() - iChatLog.getFirstIdx();
    auto firstChatLogIdx =
            chatLogIdxRange < 100 ? iChatLog.getFirstIdx() : iChatLog.getNextIdx() - 100;

    renderMessages(firstChatLogIdx, iChatLog.getNextIdx());

    typingTimer.setSingleShot(true);

    setLayout(mainLayout);

    reloadTheme();
    settings::Translator::registerHandler([this] { retranslateUi(); }, this);
    retranslateUi();
}

GenericChatForm::~GenericChatForm() {
    qDebug() << __func__;
    settings::Translator::unregister(this);
    //  delete searchForm;
}

IChatItem::Ptr GenericChatForm::createMessage(const ChatLogItem& item,
                                              bool isSelf,
                                              bool colorizeNames,
                                              const ChatLogMessage& chatLogMessage) {
    qDebug() << "createMessage id:" << item.getId() << chatLogMessage.message.content;
    auto messageType = chatLogMessage.message.isAction ? ChatMessage::MessageType::ACTION
                                                       : ChatMessage::MessageType::NORMAL;

    const bool bSelfMentioned =
            std::any_of(chatLogMessage.message.metadata.begin(),
                        chatLogMessage.message.metadata.end(),
                        [](const MessageMetadata& metadata) {
                            return metadata.type == MessageMetadataType::selfMention;
                        });

    if (bSelfMentioned) {
        messageType = ChatMessage::MessageType::ALERT;
    }

    auto chatItem = ChatMessage::createChatMessage(item, chatLogMessage.message.content,
                                                   messageType, isSelf, chatLogMessage.state,
                                                   chatLogMessage.message.timestamp, colorizeNames);

    connect(chatItem.get(), &IChatItem::replyEvent, this, &GenericChatForm::onReplyEvent);
    connect(chatLog, &ChatLog::itemContextMenuRequested,
            (ChatLineContent*)chatItem.get()->centerContent(), &ChatLineContent::onContextMenu);

    return chatItem;
}

QDateTime GenericChatForm::getLatestTime() const { return getTime(chatLog->getLatestLine()); }

QDateTime GenericChatForm::getFirstTime() const { return getTime(chatLog->getFirstLine()); }

void GenericChatForm::reloadTheme() {
    const Settings& s = Settings::getInstance();
    setStyleSheet(Style::getStylesheet("genericChatForm/genericChatForm.css"));

    //  searchForm->reloadTheme();

    //  headWidget->setStyleSheet(Style::getStylesheet("chatArea/chatHead.css"));
    //  headWidget->reloadTheme();

    chatLog->setStyleSheet(Style::getStylesheet("chatArea/chatArea.css"));
    chatLog->reloadTheme();
}

void GenericChatForm::setContact(const Contact* contact_) {
    //    qDebug()<<__func__<<contact_;
    contact = contact_;
    connect(contact, &Contact::displayedNameChanged, this,
            &GenericChatForm::onDisplayedNameChanged);

    if (contact->isGroup()) {
    } else {
        const Friend* f = static_cast<const Friend*>(contact);
        for (auto msg : messages) {
            auto p = (ChatMessageBox*)msg.second.get();
            if (p->nickname()) p->nickname()->setText(f->getDisplayedName());
        }
    }
}

void GenericChatForm::removeContact() {
    qDebug() << __func__;
    contact = nullptr;
}

void GenericChatForm::show(ContentLayout* contentLayout) {}

void GenericChatForm::showEvent(QShowEvent*) {
    inputForm->setFocus();
    if (contact) {
        if (!contact->isGroup()) {
            auto f = Nexus::getCore()->getFriendList().findFriend(*contactId);
            if (f) {
                setContact(f);
            }
        }
    }
}

bool GenericChatForm::event(QEvent* e) {
    // If the user accidentally starts typing outside of the msgEdit, focus it
    // automatically
    /* if (e->type() == QEvent::KeyPress) {
       QKeyEvent *ke = static_cast<QKeyEvent *>(e);
       if ((ke->modifiers() == Qt::NoModifier ||
            ke->modifiers() == Qt::ShiftModifier) &&
           !ke->text().isEmpty()) {
         if (searchForm->isHidden()) {
           msgEdit->sendKeyEvent(ke);
           msgEdit->setFocus();
         } else {
           searchForm->insertEditor(ke->text());
           searchForm->setFocusEditor();
         }
       }
     }*/
    return QWidget::event(e);
}

void GenericChatForm::onDisplayedNameChanged(const QString& name) {
    qDebug() << __func__ << contactId->toString() << name;
    //    headWidget->setName(name);
    for (auto msg : messages) {
        auto it = msg.second;
        auto p = (ChatMessageBox*)it.get();
        p->nickname()->setText(name);
    }
}

void GenericChatForm::onReplyEvent(IChatItem* item) { emit replyEvent(item); }

void GenericChatForm::onChatContextMenuRequested(QPoint pos) {

}

/**
 * @brief Show, is it needed to hide message author name or not
 * @param idx ChatLogIdx of the message
 * @return True if the name should be hidden, false otherwise
 */
bool GenericChatForm::needsToHideName(ChatLogIdx idx) const {
    // If the previous message is not rendered we should show the name
    // regardless of other constraints
    auto itemBefore = messages.find(idx - 1);
    if (itemBefore == messages.end()) {
        return false;
    }

    const auto prevItem = iChatLog.at(idx - 1);
    const auto currentItem = iChatLog.at(idx);
    if (!prevItem || !currentItem) {
        return false;
    }
    // Always show the * in the name field for action messages
    if (currentItem->getContentType() == ChatLogItem::ContentType::message &&
        currentItem->getContentAsMessage().message.isAction) {
        return false;
    }

    qint64 messagesTimeDiff = prevItem->getTimestamp().secsTo(currentItem->getTimestamp());
    return currentItem->getSender() == prevItem->getSender() &&
           messagesTimeDiff < chatLog->repNameAfter;
}

void GenericChatForm::onCopyLogClicked() { chatLog->copySelectedText(); }

void GenericChatForm::focusInput() { inputForm->setFocus(); }

void GenericChatForm::onChatMessageFontChanged(const QFont& font) {
    // chat log
    chatLog->fontChanged(font);
    chatLog->forceRelayout();
}

void GenericChatForm::setColorizedNames(bool enable) { colorizeNames = enable; }

void GenericChatForm::addSystemInfoMessage(const QString& message,
                                           ChatMessage::SystemMessageType type,
                                           const QDateTime& datetime) {
    insertChatMessage(ChatMessage::createChatInfoMessage(message, type, datetime));
}

void GenericChatForm::addSystemDateMessage(const QDate& date) {
    const Settings& s = Settings::getInstance();
    QString dateText = date.toString(s.getDateFormat());

    insertChatMessage(ChatMessage::createChatInfoMessage(dateText, ChatMessage::INFO, QDateTime()));
}

QDateTime GenericChatForm::getTime(const IChatItem::Ptr& chatLine) const {
    if (chatLine) {
        return chatLine->getTime();
    }
    return QDateTime();
}

void GenericChatForm::disableSearchText() {
    auto msgIt = messages.find(searchPos.logIdx);
    if (msgIt != messages.end()) {
        auto text = qobject_cast<Text*>(msgIt->second->centerContent());
        text->deselectText();
    }
}

void GenericChatForm::clearChatArea() { clearChatArea(/* confirm = */ true, /* inform = */ true); }

void GenericChatForm::clearChatArea(bool confirm, bool inform) {
    if (confirm) {
        QMessageBox::StandardButton mboxResult = QMessageBox::question(
                this, tr("Confirmation"),
                tr("You are sure that you want to clear all displayed messages?"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (mboxResult == QMessageBox::No) {
            return;
        }
    }

    chatLog->clear();

    if (inform)
        addSystemInfoMessage(tr("Cleared"), ChatMessage::INFO, QDateTime::currentDateTime());

    messages.clear();
}

void GenericChatForm::onSelectAllClicked() { chatLog->selectAll(); }

void GenericChatForm::insertChatMessage(IChatItem::Ptr msg) {
    chatLog->insertChatlineAtBottom(msg);
    emit messageInserted();
}

bool GenericChatForm::eventFilter(QObject* object, QEvent* event) {
    // EmoticonsWidget* ev = qobject_cast<EmoticonsWidget*>(object);
    // if (ev && event->type() == QEvent::KeyPress) {
    //     QKeyEvent* key = static_cast<QKeyEvent*>(event);
    //     msgEdit->sendKeyEvent(key);
    //     msgEdit->setFocus();
    //     return false;
    // }
    return false;
}

void GenericChatForm::onSplitterMoved(int, int) {
    //  if (netcam)
    //    netcam->setShowMessages(bodySplitter->sizes()[1] == 0);
}

void GenericChatForm::onShowMessagesClicked() {
    //  if (netcam) {
    //    if (bodySplitter->sizes()[1] == 0)
    //      bodySplitter->setSizes({1, 1});
    //    else
    //      bodySplitter->setSizes({1, 0});
    //
    //    onSplitterMoved(0, 0);
    //  }
}

void GenericChatForm::quoteSelectedText() {
    QString selectedText = chatLog->getSelectedText();
    if (selectedText.isEmpty()) return;

    // forming pretty quote text
    // 1. insert "> " to the begining of quote;
    // 2. replace all possible line terminators with "\n> ";
    // 3. append new line to the end of quote.
    QString quote = selectedText;

    // quote.insert(0, "> ");
    // quote.replace(QRegExp(QString("\r\n|[\r\n\u2028\u2029]")), QString("\n> "));
    // quote.append("\n");

    // inputForm->setQuote(quote);
}

void GenericChatForm::forwardSelectedText() {
    QString selectedText = chatLog->getSelectedText();
    if (selectedText.isEmpty()) return;
}

void GenericChatForm::copyLink() {
    //    QString linkText = copyLinkAction->data().toString();
    //    QApplication::clipboard()->setText(linkText);
}

void GenericChatForm::searchFormShow() {
    /* if (searchForm->isHidden()) {
       searchForm->show();
       searchForm->setFocusEditor();
     }*/
}

void GenericChatForm::onLoadHistory() {
    LoadHistoryDialog dlg(&iChatLog);
    if (dlg.exec()) {
        QDateTime time = dlg.getFromDate();
        auto idx = firstItemAfterDate(dlg.getFromDate().date(), iChatLog);
        renderMessages(idx, iChatLog.getNextIdx());
    }
}

void GenericChatForm::onExportChat() {
    QString path = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save chat log"));
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QString buffer;
    for (auto i = iChatLog.getFirstIdx(); i < iChatLog.getNextIdx(); ++i) {
        const auto& item = iChatLog.at(i);
        if (!item || item->getContentType() != ChatLogItem::ContentType::message) {
            continue;
        }

        QString timestamp = item->getTimestamp().time().toString("hh:mm:ss");
        QString datestamp = item->getTimestamp().date().toString("yyyy-MM-dd");
        QString author = item->getDisplayName();

        buffer = buffer % QString{datestamp % '\t' % timestamp % '\t' % author % '\t' %
                                  item->getContentAsMessage().message.content % '\n'};
    }
    file.write(buffer.toUtf8());
    file.close();
}

void GenericChatForm::onSearchTriggered() {
    // if (searchForm->isHidden()) {
    //   searchForm->removeSearchPhrase();
    // }
    disableSearchText();
}

void GenericChatForm::searchInBegin(const QString& phrase, const ParameterSearch& parameter) {
    disableSearchText();

    bool bForwardSearch = false;
    switch (parameter.period) {
        case PeriodSearch::WithTheFirst: {
            bForwardSearch = true;
            searchPos.logIdx = iChatLog.getFirstIdx();
            searchPos.numMatches = 0;
            break;
        }
        case PeriodSearch::WithTheEnd:
        case PeriodSearch::None: {
            bForwardSearch = false;
            searchPos.logIdx = iChatLog.getNextIdx();
            searchPos.numMatches = 0;
            break;
        }
        case PeriodSearch::AfterDate: {
            bForwardSearch = true;
            searchPos.logIdx = firstItemAfterDate(parameter.date, iChatLog);
            searchPos.numMatches = 0;
            break;
        }
        case PeriodSearch::BeforeDate: {
            bForwardSearch = false;
            searchPos.logIdx = firstItemAfterDate(parameter.date, iChatLog);
            searchPos.numMatches = 0;
            break;
        }
    }

    if (bForwardSearch) {
        onSearchDown(phrase, parameter);
    } else {
        onSearchUp(phrase, parameter);
    }
}

void GenericChatForm::onSearchUp(const QString& phrase, const ParameterSearch& parameter) {
    auto result = iChatLog.searchBackward(searchPos, phrase, parameter);
    handleSearchResult(result, SearchDirection::Up);
}

void GenericChatForm::onSearchDown(const QString& phrase, const ParameterSearch& parameter) {
    auto result = iChatLog.searchForward(searchPos, phrase, parameter);
    handleSearchResult(result, SearchDirection::Down);
}

void GenericChatForm::handleSearchResult(SearchResult result, SearchDirection direction) {
    if (!result.found) {
        emit messageNotFoundShow(direction);
        return;
    }

    disableSearchText();

    searchPos = result.pos;

    auto const firstRenderedIdx =
            (messages.empty()) ? iChatLog.getNextIdx() : messages.begin()->first;

    renderMessages(searchPos.logIdx, firstRenderedIdx, [this, result] {
        auto msg = messages.at(searchPos.logIdx);
        chatLog->scrollToLine(msg);

        auto text = qobject_cast<Text*>(msg->centerContent());
        text->selectText(result.exp, std::make_pair(result.start, result.len));
    });
}

void GenericChatForm::renderMessage0(ChatLogIdx idx) {
    qDebug() << __func__ << "for contact:" << contactId->toString()
             << "message log index:" << idx.get();
    renderMessages(idx, idx + 1);
}

void GenericChatForm::renderMessages(ChatLogIdx begin, ChatLogIdx end,
                                     std::function<void(void)> onCompletion) {
    QList<IChatItem::Ptr> beforeLines;
    QList<IChatItem::Ptr> afterLines;

    for (auto i = begin; i < end; ++i) {
        auto item = iChatLog.at(i);
        if (!item) {
            qWarning() << "chatLog have no msg idx is:" << i.get();
            continue;
        }

        auto chatMessage = getChatMessageForIdx(i, messages);
        //        if(!chatMessage || !chatMessage->isValid()){
        //            continue;
        //        }
        renderItem(*item, needsToHideName(i), colorizeNames, chatMessage);

        if (messages.find(i) == messages.end()) {
            QList<IChatItem::Ptr>* lines =
                    (messages.empty() || i > messages.rbegin()->first) ? &afterLines : &beforeLines;

            messages.insert({i, chatMessage});

            if (shouldRenderDate(i, iChatLog)) {
                auto msg = iChatLog.at(i);
                if (msg) lines->push_back(dateMessageForItem(*msg));
            }
            lines->push_back(chatMessage);
        }
    }

    for (auto const& line : afterLines) {
        chatLog->insertChatlineAtBottom(line);
    }

    if (!beforeLines.empty()) {
        // Rendering upwards is expensive and has async behavior for chatLog.
        // Once rendering completes we call our completion callback once and
        // then disconnect the signal
        if (onCompletion) {
            auto connection = std::make_shared<QMetaObject::Connection>();
            *connection =
                    connect(chatLog, &ChatLog::workerTimeoutFinished, [onCompletion, connection] {
                        onCompletion();
                        disconnect(*connection);
                    });
        }

        chatLog->insertChatlinesOnTop(beforeLines);
    } else if (onCompletion) {
        onCompletion();
    }
}

void GenericChatForm::loadHistoryLower() {
    auto begin = messages.begin()->first;
    if (begin.get() > 100) {
        begin = ChatLogIdx(begin.get() - 100);
    } else {
        begin = ChatLogIdx(0);
    }
    renderMessages(begin, iChatLog.getNextIdx());
}

void GenericChatForm::updateShowDateInfo(const IChatItem::Ptr& prevLine,
                                         const IChatItem::Ptr& topLine) {
    // If the dateInfo is visible we need to pretend the top line is the one
    // covered by the date to prevent oscillations
    //  const auto effectiveTopLine =
    //      (dateInfo->isVisible() && prevLine) ? prevLine : topLine;
    //
    //  const auto date = getTime(effectiveTopLine);
    //
    //  if (date.isValid() && date.date() != QDate::currentDate()) {
    //    const auto dateText =
    //        QStringLiteral("<b>%1<\b>")
    //            .arg(date.toString(Settings::getInstance().getDateFormat()));
    //    dateInfo->setText(dateText);
    //    dateInfo->setVisible(true);
    //  } else {
    //    dateInfo->setVisible(false);
    //  }
}

void GenericChatForm::retranslateUi() {
    //    copyLinkAction->setText(tr("Copy link address"));
    // searchAction->setText(tr("Search in text"));
    // loadHistoryAction->setText(tr("Load chat history..."));selected text
    // exportChatAction->setText(tr("Export to file"));
}

void GenericChatForm::onTextEditChanged(const QString& text) {
    if (!Settings::getInstance().getTypingNotification()) {
        if (isTyping) {
            isTyping = false;
            Core::getInstance()->sendTyping(contactId->getId(), false);
        }
        return;
    }

    bool isTypingNow = !text.isEmpty();
    if (isTyping != isTypingNow) {
        Core::getInstance()->sendTyping(contactId->getId(), isTypingNow);
        if (isTypingNow) {
            typingTimer.start(TYPING_NOTIFICATION_DURATION);
        }

        isTyping = isTypingNow;
    }
}

void GenericChatForm::onTextSend(const QString& msg) {
    if (msg.isEmpty()) return;
    if (msg.trimmed().isEmpty()) {
        return;
    }
    auto sent = messageDispatcher.sendMessage(false, msg, false);
    qDebug() << &messageDispatcher << "sendMessage=>" << sent.first.get() << sent.second;
}

void GenericChatForm::onFileSend(const QFile& file) {
    if (!file.exists()) {
        qWarning() << "File is no existing!";
        return;
    }
    sendFile(file);
}

void GenericChatForm::onImageSend(const QPixmap& pix) {
    qDebug() << __func__ << pix;

    if (pix.isNull()) {
        qWarning() << "Empty image!";
        return;
    }

    QFile file("./" + ok::base::UUID::make() + ".png");
    bool saved = ok::base::Images::SaveToFile(pix, file, "png");
    if (!saved) {
        qWarning() << "Unable to save to temp file" << file.fileName();
        return;
    }
    file.remove();
    sendFile(file);
}

void GenericChatForm::sendFile(const QFile& file) {
    qDebug() << "Sending image:" << file.fileName();
    auto sent = Nexus::getProfile()->getCoreFile()->sendFile(contact->getId(), file);
    if (!sent) {
        ok::base::MessageBox::warning(
                this, "", tr("The community version cannot send files to offline contacts!"));
    }
}
