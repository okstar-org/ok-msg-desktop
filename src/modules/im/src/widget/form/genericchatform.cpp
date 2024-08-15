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
#include "src/model/group.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/video/genericnetcamview.h"
#include "src/widget/chatformheader.h"
#include "src/widget/contentdialog.h"
#include "src/widget/contentdialogmanager.h"
#include "src/widget/contentlayout.h"
#include "src/widget/emoticonswidget.h"
#include "src/widget/form/chatform.h"
#include "src/widget/form/loadhistorydialog.h"
#include "src/widget/maskablepixmapwidget.h"
// #include "src/widget/searchform.h"
#include "src/lib/settings/style.h"
#include "src/widget/gui.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/tool/flyoutoverlaywidget.h"
#include "src/widget/widget.h"

#include <QClipboard>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSplitter>
#include <QStringBuilder>
#include <QtGlobal>

#ifdef SPELL_CHECKING
#include <KF5/SonnetUi/sonnet/spellcheckdecorator.h>
#endif

#ifdef OK_PLUGIN
#include "lib/plugin/pluginmanager.h"

#include <src/chatlog/chatmessageitem.h>

#include <src/nexus.h>

#include <src/core/coreav.h>
#endif

/**
 * @class GenericChatForm
 * @brief Parent class for all chatforms. It's provide the minimum required UI
 * elements and methods to work with chat messages.
 */

static const QSize FILE_FLYOUT_SIZE{24, 24};
static const short FOOT_BUTTONS_SPACING = 2;
static const short MESSAGE_EDIT_HEIGHT = 50;
static const short MAIN_FOOT_MARGIN = 8;
static const short EDIT_SEND_SPACING = 5;
static const QString FONT_STYLE[]{"normal", "italic", "oblique"};

/**
 * @brief Creates CSS style string for needed class with specified font
 * @param font Font that needs to be represented for a class
 * @param name Class name
 * @return Style string
 */
static QString fontToCss(const QFont& font, const QString& name) {
    QString result{
            "%1{"
            "font-family: \"%2\"; "
            "font-size: %3px; "
            "font-style: \"%4\"; "
            "font-weight: normal;}"};
    return result.arg(name).arg(font.family()).arg(font.pixelSize()).arg(FONT_STYLE[font.style()]);
}

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

namespace {

template <class T, class Fun>
QPushButton* createButton(const QString& name, T* self, Fun onClickSlot) {
    QPushButton* btn = new QPushButton();
    // Fix for incorrect layouts on OS X as per
    // https://bugreports.qt-project.org/browse/QTBUG-14591
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setObjectName(name);
    // btn->setProperty("state", "green");
    btn->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    btn->setCheckable(true);
    QObject::connect(btn, &QPushButton::clicked, self, onClickSlot);
    return btn;
}

IChatItem::Ptr getChatMessageForIdx(ChatLogIdx idx,
                                    const std::map<ChatLogIdx, IChatItem::Ptr>& messages) {
    auto existingMessageIt = messages.find(idx);
    if (existingMessageIt == messages.end()) {
        return IChatItem::Ptr();
    }

    return existingMessageIt->second;
}

bool shouldRenderDate(ChatLogIdx idxToRender, const IChatLog& chatLog) {
    if (idxToRender.get() == chatLog.getFirstIdx().get()) return true;

    auto prev = chatLog.at(idxToRender - 1);
    auto cur = chatLog.at(idxToRender);
    if (!prev || !cur) return false;

    return prev->getTimestamp().date() != cur->getTimestamp().date();
}

IChatItem::Ptr dateMessageForItem(const ChatLogItem& item) {
    const auto& s = Settings::getInstance();
    const auto date = item.getTimestamp().date();
    auto dateText = date.toString(s.getDateFormat());
    return ChatMessage::createChatInfoMessage(dateText, ChatMessage::INFO, QDateTime());
}

IChatItem::Ptr createMessage(const ChatLogItem& item, bool isSelf, bool colorizeNames,
                             const ChatLogMessage& chatLogMessage) {
    //  qDebug() << "createMessage displayName:" << displayName;
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

    return ChatMessage::createChatMessage(item, chatLogMessage.message.content, messageType, isSelf,
                                          chatLogMessage.state, chatLogMessage.message.timestamp,
                                          colorizeNames);
}

void renderMessage(const ChatLogItem& item, bool isSelf, bool colorizeNames,
                   const ChatLogMessage& chatLogMessage, IChatItem::Ptr& chatMessage) {
    if (chatMessage) {
        if (chatLogMessage.state == MessageState::complete) {
            chatMessage->markAsDelivered(chatLogMessage.message.timestamp);
        }
    } else {
        chatMessage = createMessage(item, isSelf, colorizeNames, chatLogMessage);
    }
}

void renderFile(const ChatLogItem& item, ToxFile file, bool isSelf, QDateTime timestamp,
                IChatItem::Ptr& chatMessage) {
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

void renderItem(const ChatLogItem& item,
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

ChatLogIdx firstItemAfterDate(QDate date, const IChatLog& chatLog) {
    auto idxs = chatLog.getDateIdxs(date, 1);
    if (idxs.size()) {
        return idxs[0].idx;
    } else {
        return chatLog.getNextIdx();
    }
}
}  // namespace

GenericChatForm::GenericChatForm(const ContactId* contact_,
                                 IChatLog& iChatLog_,
                                 IMessageDispatcher& messageDispatcher,
                                 QWidget* parent)
        : QWidget(parent, Qt::Window)
        , contactId(contact_)
        , contact(nullptr)
        , audioInputFlag(false)
        , audioOutputFlag(false)
        , isEncrypt(false)
        , iChatLog(iChatLog_)
        , emoticonsWidget{nullptr}
        , messageDispatcher(messageDispatcher) {
    setContentsMargins(0, 0, 0, 0);

    qDebug() << __func__ << "contact:" << contact_;

    curRow = 0;
    // searchForm = new SearchForm();
    // searchForm->hide();

    chatLog = new ChatLog(this);
    chatLog->setBusyNotification(ChatMessage::createBusyNotification());

    //  dateInfo = new QLabel(this);
    //  dateInfo->setAlignment(Qt::AlignHCenter);
    //  dateInfo->setVisible(false);

    // settings
    const Settings& s = Settings::getInstance();
    connect(&s, &Settings::emojiFontPointSizeChanged, chatLog, &ChatLog::forceRelayout);
    connect(&s, &Settings::chatMessageFontChanged, this,
            &GenericChatForm::onChatMessageFontChanged);

    msgEdit = new ChatTextEdit();
#ifdef SPELL_CHECKING
    if (s.getSpellCheckingEnabled()) {
        decorator = new Sonnet::SpellCheckDecorator(msgEdit);
    }
#endif
    encryptButton = createButton("encryptButton", this, &GenericChatForm::onEncryptButtonClicked);

    sendButton = createButton("sendButton", this, &GenericChatForm::onSendTriggered);

    emoteButton = createButton("emoteButton", this, &GenericChatForm::onEmoteButtonClicked);

    fileButton = createButton("fileButton", this, &GenericChatForm::onAttachClicked);
    screenshotButton =
            createButton("screenshotButton", this, &GenericChatForm::onScreenshotClicked);

    // TODO: Make updateCallButtons (see ChatForm) abstract
    //       and call here to set tooltips.

    msgEdit->setFixedHeight(MESSAGE_EDIT_HEIGHT);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    bodySplitter = new QSplitter(Qt::Vertical, this);
    connect(bodySplitter, &QSplitter::splitterMoved, this, &GenericChatForm::onSplitterMoved);
    QWidget* contentWidget = new QWidget(this);
    contentWidget->setObjectName("ChatContentContainer");
    bodySplitter->addWidget(contentWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(bodySplitter);
    mainLayout->setMargin(0);

    setLayout(mainLayout);

    QWidget* footContainer = new QFrame(contentWidget);
    footContainer->setAttribute(Qt::WA_StyledBackground);
    footContainer->setAutoFillBackground(true);
    footContainer->setObjectName("ChatFootContainer");
    QHBoxLayout* footButtonsSmall = new QHBoxLayout();
    footButtonsSmall->setSpacing(FOOT_BUTTONS_SPACING);
    footButtonsSmall->addWidget(emoteButton);
    footButtonsSmall->addWidget(fileButton);
    footButtonsSmall->addWidget(screenshotButton);
#ifdef OK_PLUGIN
    auto pm = ok::plugin::PluginManager::instance();
    connect(pm, &ok::plugin::PluginManager::pluginEnabled,  //
            this, &::GenericChatForm::onPluginEnabled);
    connect(pm, &ok::plugin::PluginManager::pluginDisabled,  //
            this, &::GenericChatForm::onPluginDisabled);
    auto omemo = pm->plugin("omemo");
    if (omemo) {
        footButtonsSmall->addWidget(encryptButton);
    }
#endif
    footButtonsSmall->addStretch(1);

    QVBoxLayout* sendButtonLyt = new QVBoxLayout();
    sendButtonLyt->addStretch(1);
    sendButtonLyt->addWidget(sendButton);

    QVBoxLayout* inputLayout = new QVBoxLayout();
    inputLayout->addLayout(footButtonsSmall);
    inputLayout->setSpacing(FOOT_BUTTONS_SPACING);
    inputLayout->addWidget(msgEdit);

    mainFootLayout = new QHBoxLayout(footContainer);
    mainFootLayout->setContentsMargins(MAIN_FOOT_MARGIN, MAIN_FOOT_MARGIN, MAIN_FOOT_MARGIN,
                                       MAIN_FOOT_MARGIN);
    mainFootLayout->setSpacing(EDIT_SEND_SPACING);
    mainFootLayout->addLayout(inputLayout);
    mainFootLayout->addLayout(sendButtonLyt);

    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    // contentLayout->addWidget(searchForm);
    //  contentLayout->addWidget(dateInfo);
    contentLayout->addWidget(chatLog, 1);
    contentLayout->addWidget(footContainer, 0);

    quoteAction = menu.addAction(QIcon(), QString(), this, SLOT(quoteSelectedText()),
                                 QKeySequence(Qt::ALT, Qt::Key_Q));
    addAction(quoteAction);
    menu.addSeparator();

    // searchAction =
    //     menu.addAction(QIcon(), QString(), this, SLOT(searchFormShow()),
    //                    QKeySequence(Qt::CTRL + Qt::Key_F));
    // addAction(searchAction);

    menu.addSeparator();

    menu.addActions(chatLog->actions());
    menu.addSeparator();

    clearAction =
            menu.addAction(QIcon::fromTheme("edit-clear"), QString(), this, SLOT(clearChatArea()),
                           QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
    addAction(clearAction);

    copyLinkAction = menu.addAction(QIcon(), QString(), this, SLOT(copyLink()));
    menu.addSeparator();

    // loadHistoryAction =
    //     menu.addAction(QIcon(), QString(), this, SLOT(onLoadHistory()));
    // exportChatAction = menu.addAction(QIcon::fromTheme("document-save"),
    //                                   QString(), this, SLOT(onExportChat()));

    connect(chatLog, &ChatLog::customContextMenuRequested, this,
            &GenericChatForm::onChatContextMenuRequested);
    connect(chatLog, &ChatLog::firstVisibleLineChanged, this, &GenericChatForm::updateShowDateInfo);
    connect(chatLog, &ChatLog::loadHistoryLower, this, &GenericChatForm::loadHistoryLower);

    connect(&iChatLog, &IChatLog::itemUpdated, this, &GenericChatForm::renderMessage);

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

    connect(msgEdit, &ChatTextEdit::enterPressed, this, &GenericChatForm::onSendTriggered);

    connect(&GUI::getInstance(), &GUI::themeApplyRequest, this, &GenericChatForm::reloadTheme);
    reloadTheme();

    settings::Translator::registerHandler([this] { retranslateUi(); }, this);
    retranslateUi();

    auto chatLogIdxRange = iChatLog.getNextIdx() - iChatLog.getFirstIdx();
    auto firstChatLogIdx =
            chatLogIdxRange < 100 ? iChatLog.getFirstIdx() : iChatLog.getNextIdx() - 100;

    renderMessages(firstChatLogIdx, iChatLog.getNextIdx());
}

GenericChatForm::~GenericChatForm() {
    qDebug() << __func__;
    settings::Translator::unregister(this);
    //  delete searchForm;
}

#ifdef OK_PLUGIN
void GenericChatForm::onPluginEnabled(const QString& shortName) {
    //  qDebug() << "Plugin is enabled" << shortName <<"for"<<contact->getDisplayedName();
    if (shortName == "omemo") {
        auto encryptButton_ = mainFootLayout->findChild<QPushButton*>("encryptButton");
        if (!encryptButton_) {
            mainFootLayout->insertWidget(0, encryptButton);
        }
        encryptButton->show();
    }
}

void GenericChatForm::onPluginDisabled(const QString& shortName) {
    qDebug() << "Plugin is disabled." << shortName;
    if (shortName == "omemo") {
        encryptButton->hide();
        //    mainFootLayout->removeWidget(encryptButton);
    }
}
#endif

QDateTime GenericChatForm::getLatestTime() const { return getTime(chatLog->getLatestLine()); }

QDateTime GenericChatForm::getFirstTime() const { return getTime(chatLog->getFirstLine()); }

void GenericChatForm::reloadTheme() {
    const Settings& s = Settings::getInstance();
    setStyleSheet(Style::getStylesheet("genericChatForm/genericChatForm.css"));

    msgEdit->setStyleSheet(Style::getStylesheet("msgEdit/msgEdit.css") +
                           fontToCss(s.getChatMessageFont(), "QTextEdit"));

    //  searchForm->reloadTheme();

    //  headWidget->setStyleSheet(Style::getStylesheet("chatArea/chatHead.css"));
    //  headWidget->reloadTheme();

    chatLog->setStyleSheet(Style::getStylesheet("chatArea/chatArea.css"));
    chatLog->reloadTheme();

    fileButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    emoteButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    screenshotButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    sendButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
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
            p->nickname()->setText(f->getDisplayedName());
        }
    }
}

void GenericChatForm::removeContact() {
    qDebug() << __func__;
    contact = nullptr;
}

void GenericChatForm::show(ContentLayout* contentLayout) {}

void GenericChatForm::showEvent(QShowEvent*) {
    msgEdit->setFocus();
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

void GenericChatForm::onChatContextMenuRequested(QPoint pos) {
    QWidget* sender = static_cast<QWidget*>(QObject::sender());
    pos = sender->mapToGlobal(pos);

    // If we right-clicked on a link, give the option to copy it
    bool clickedOnLink = false;
    Text* clickedText = qobject_cast<Text*>(chatLog->getContentFromGlobalPos(pos));
    if (clickedText) {
        QPointF scenePos = chatLog->mapToScene(chatLog->mapFromGlobal(pos));
        QString linkTarget = clickedText->getLinkAt(scenePos);
        if (!linkTarget.isEmpty()) {
            clickedOnLink = true;
            copyLinkAction->setData(linkTarget);
        }
    }
    copyLinkAction->setVisible(clickedOnLink);

    menu.exec(pos);
}

void GenericChatForm::onSendTriggered() {
    auto msg = msgEdit->toPlainText();

    bool isAction = msg.startsWith(ChatForm::ACTION_PREFIX, Qt::CaseInsensitive);
    if (isAction) {
        msg.remove(0, ChatForm::ACTION_PREFIX.length());
    }

    if (msg.isEmpty()) {
        return;
    }

    msgEdit->setLastMessage(msg);
    msgEdit->clear();

    qDebug() << "Input text:" << msg;
    auto sent = messageDispatcher.sendMessage(isAction, msg, isEncrypt);
    qDebug() << &messageDispatcher << "sendMessage=>" << sent.first.get() << sent.second;
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

void GenericChatForm::onEncryptButtonClicked() {
    auto btn = dynamic_cast<QPushButton*>(sender());
    btn->setChecked(!isEncrypt);
    isEncrypt = btn->isChecked();
    qDebug() << "isEncrypt changed=>" << isEncrypt;
}

void GenericChatForm::onEmoteButtonClicked() {
    // don't show the smiley selection widget if there are no smileys available
    if (SmileyPack::getInstance().getEmoticons().empty()) return;

    if (!emoticonsWidget) {
        emoticonsWidget = new EmoticonsWidget(this);
        emoticonsWidget->installEventFilter(this);
        connect(emoticonsWidget, SIGNAL(insertEmoticon(QString)), this,
                SLOT(onEmoteInsertRequested(QString)));
    }

    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        QPoint pos = -QPoint(emoticonsWidget->sizeHint().width() / 2,
                             emoticonsWidget->sizeHint().height()) -
                     QPoint(0, 10);
        emoticonsWidget->exec(sender->mapToGlobal(pos));
    }
}

void GenericChatForm::onEmoteInsertRequested(QString str) {
    // insert the emoticon
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) msgEdit->insertPlainText(str);

    // refocus so that we can continue typing
    msgEdit->setFocus();

    if (emoticonsWidget) {
        // close
        emoticonsWidget->close();
    }
}

void GenericChatForm::onCopyLogClicked() { chatLog->copySelectedText(); }

void GenericChatForm::focusInput() { msgEdit->setFocus(); }

void GenericChatForm::onChatMessageFontChanged(const QFont& font) {
    // chat log
    chatLog->fontChanged(font);
    chatLog->forceRelayout();
    // message editor
    msgEdit->setStyleSheet(Style::getStylesheet("msgEdit/msgEdit.css") +
                           fontToCss(font, "QTextEdit"));
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
    EmoticonsWidget* ev = qobject_cast<EmoticonsWidget*>(object);
    if (ev && event->type() == QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        msgEdit->sendKeyEvent(key);
        msgEdit->setFocus();
        return false;
    }
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

    quote.insert(0, "> ");
    quote.replace(QRegExp(QString("\r\n|[\r\n\u2028\u2029]")), QString("\n> "));
    quote.append("\n");

    msgEdit->append(quote);
}

/**
 * @brief Callback of GenericChatForm::copyLinkAction
 */
void GenericChatForm::copyLink() {
    QString linkText = copyLinkAction->data().toString();
    QApplication::clipboard()->setText(linkText);
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

void GenericChatForm::renderMessage(ChatLogIdx idx) {
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
    sendButton->setToolTip(tr("Send message"));
    emoteButton->setToolTip(tr("Smileys"));
    fileButton->setToolTip(tr("Send file(s)"));
    screenshotButton->setToolTip(tr("Send a screenshot"));
    clearAction->setText(tr("Clear displayed messages"));
    quoteAction->setText(tr("Quote"));
    copyLinkAction->setText(tr("Copy link address"));
    // searchAction->setText(tr("Search in text"));
    // loadHistoryAction->setText(tr("Load chat history..."));selected text
    // exportChatAction->setText(tr("Export to file"));
}
