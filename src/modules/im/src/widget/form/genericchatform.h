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

#ifndef GENERICCHATFORM_H
#define GENERICCHATFORM_H

#include "src/chatlog/chatmessage.h"

#include "src/model/ichatlog.h"
#include "src/widget/searchtypes.h"

#include <QMenu>
#include <QWidget>

/**
 * Spacing in px inserted when the author of the last message changes
 * @note Why the hell is this a thing? surely the different font is enough?
 *        - Even a different font is not enough – TODO #1307 ~~zetok
 */

class ChatFormHeader;
class ChatLog;
class ChatTextEdit;
class Contact;
class ContentLayout;
class CroppingLabel;
class FlyoutOverlayWidget;
class GenericNetCamView;
class MaskablePixmapWidget;
class SearchForm;
class Widget;

class QLabel;
class QPushButton;
class QSplitter;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class IMessageDispatcher;
class EmoticonsWidget;
class ChatInputForm;
struct Message;

namespace Ui {
class MainWindow;
}

#ifdef SPELL_CHECKING
namespace Sonnet {
class SpellCheckDecorator;
}
#endif

class GenericChatForm : public QWidget {
    Q_OBJECT
public:
    GenericChatForm(const ContactId* contact,
                    IChatLog& chatLog,
                    IMessageDispatcher& messageDispatcher,
                    QWidget* parent = nullptr);
    ~GenericChatForm() override;

    void setContact(const Contact* contact);
    void removeContact();

    virtual void show(ContentLayout* contentLayout);
    virtual void reloadTheme();

    void addSystemInfoMessage(const QString& message,
                              ChatMessage::SystemMessageType type,
                              const QDateTime& datetime);
    static QString resolveToxPk(const FriendId& pk);
    QDateTime getLatestTime() const;
    QDateTime getFirstTime() const;

    [[__nodiscard__]] inline ChatLog* getChatLog() const { return chatLog; }

signals:
    void messageInserted();
    void messageNotFoundShow(SearchDirection direction);

public slots:
    void focusInput();
    void onChatMessageFontChanged(const QFont& font);
    void setColorizedNames(bool enable);

    void onDisplayedNameChanged(const QString& name);

protected slots:
    void onTextEditChanged(const QString& msg);
    void onTextSend(const QString& msg);
    void onFileSend(const QFile& file);
    void onImageSend(const QPixmap& pix);

    void quoteSelectedText();
    void forwardSelectedText();
    void onChatContextMenuRequested(QPoint pos);

    void onCopyLogClicked();

    void clearChatArea();
    void clearChatArea(bool confirm, bool inform);
    void onSelectAllClicked();
    void onShowMessagesClicked();
    void onSplitterMoved(int pos, int index);

    void copyLink();
    void onLoadHistory();
    void onExportChat();
    void searchFormShow();
    void onSearchTriggered();
    void updateShowDateInfo(const IChatItem::Ptr& prevLine, const IChatItem::Ptr& topLine);

    void searchInBegin(const QString& phrase, const ParameterSearch& parameter);
    void onSearchUp(const QString& phrase, const ParameterSearch& parameter);
    void onSearchDown(const QString& phrase, const ParameterSearch& parameter);
    void handleSearchResult(SearchResult result, SearchDirection direction);
    void renderMessage(ChatLogIdx idx);
    void renderMessages(ChatLogIdx begin, ChatLogIdx end,
                        std::function<void(void)> onCompletion = std::function<void(void)>());

    void loadHistoryLower();


private:
    void retranslateUi();
    void addSystemDateMessage(const QDate& date);
    QDateTime getTime(const IChatItem::Ptr& chatLine) const;

protected:
    // ChatMessage::Ptr createMessage(const ToxPk& author, const QString& message,
    //                                const QDateTime& datetime, bool isAction, bool isSent, bool
    //                                colorizeName = false);
    bool needsToHideName(ChatLogIdx idx) const;

    void disableSearchText();
    bool searchInText(const QString& phrase, const ParameterSearch& parameter,
                      SearchDirection direction);
    std::pair<int, int> indexForSearchInLine(const QString& txt, const QString& phrase,
                                             const ParameterSearch& parameter,
                                             SearchDirection direction);

    virtual void insertChatMessage(IChatItem::Ptr msg);

    virtual void showEvent(QShowEvent*) override;
    virtual bool event(QEvent*) final override;
    virtual bool eventFilter(QObject* object, QEvent* event) final override;

protected:
    const ContactId* contactId;
    const Contact* contact = nullptr;

    bool audioInputFlag;
    bool audioOutputFlag;
    int curRow;
    QSplitter* bodySplitter;
    QAction* clearAction;
    QAction* copyLinkAction;

    // QAction* loadHistoryAction;
    // QAction* exportChatAction;

    QMenu menu;


    //    SearchForm *searchForm;
    //    QLabel *dateInfo;
    ChatLog* chatLog;

#ifdef SPELL_CHECKING
    Sonnet::SpellCheckDecorator* decorator{nullptr};
#endif

    Widget* parent;

    IChatLog& iChatLog;
    IMessageDispatcher& messageDispatcher;
    SearchPos searchPos;
    std::map<ChatLogIdx, IChatItem::Ptr> messages;
    bool colorizeNames = false;

    ChatInputForm* inputForm;
    bool isTyping;
    QTimer typingTimer;
};

#endif  // GENERICCHATFORM_H
