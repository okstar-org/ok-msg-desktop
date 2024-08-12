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

#include "chatmessage.h"
#include "chatlinecontentproxy.h"
#include "content/broken.h"
#include "content/contactavatar.h"
#include "content/filetransferwidget.h"
#include "content/image.h"
#include "content/notificationicon.h"
#include "content/spinner.h"
#include "content/text.h"
#include "content/timestamp.h"
#include "src/lib/settings/style.h"
#include "textformatter.h"

#include <QCryptographicHash>
#include <QDebug>

#include "src/chatlog/chatmessageitem.h"
#include "src/nexus.h"
#include "src/persistence/history.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"

#define NAME_COL_WIDTH 140.0
#define TIME_COL_WIDTH 90.0

IChatItem::Ptr ChatMessage::createChatMessage(const ChatLogItem& item, const QString& rawMessage,
                                              MessageType type, bool isMe, MessageState state,
                                              const QDateTime& date, bool colorizeName) {
    auto avatar = Nexus::getProfile()->loadAvatar(item.getSender());
    auto* msg = new ChatMessageBox(avatar, item.getDisplayName(), rawMessage.toHtmlEscaped(), isMe);
    msg->setMessageState(state);
    msg->setTime(date);
    return IChatItem::Ptr(msg);
}

IChatItem::Ptr ChatMessage::createChatInfoMessage(const QString& rawMessage,
                                                  SystemMessageType type,
                                                  const QDateTime& date) {
    QString text = rawMessage.toHtmlEscaped();
    QString img;
    switch (type) {
        case INFO:
            img = Style::getImagePath("chatArea/info.svg");
            break;
        case ERROR:
            img = Style::getImagePath("chatArea/error.svg");
            break;
        case TYPING:
            img = Style::getImagePath("chatArea/typing.svg");
            break;
    }

    QFont baseFont = Settings::getInstance().getChatMessageFont();
    Image* imgItem = new Image(QSize(18, 18), img);
    auto* item = new ChatNotificationBox(text, baseFont);
    item->setIcon(imgItem);
    item->setTime(date);
    return IChatItem::Ptr(item);
}

IChatItem::Ptr ChatMessage::createFileTransferMessage(const ChatLogItem& item, ToxFile file,
                                                      bool isMe, const QDateTime& date) {
    qDebug() << __func__ << file.fileName;
    QPixmap avatar;
    if (isMe) {
        const auto& self = Core::getInstance()->getSelfPeerId().getPublicKey();
        avatar = Nexus::getProfile()->loadAvatar(self);
    } else {
        avatar = Nexus::getProfile()->loadAvatar(item.getSender());
    }

    auto ftw = new FileTransferWidget(nullptr, file);
    ChatLineContent* fileContent = new ChatLineContentProxy(ftw, 320, 0.6f);

    ChatMessageBox* msg = new ChatMessageBox(avatar, item.getDisplayName(), fileContent, isMe);

    msg->setTime(date);
    return IChatItem::Ptr(msg);
}

IChatItem::Ptr ChatMessage::createTypingNotification() {
    ChatMessage::Ptr msg = ChatMessage::Ptr(new ChatMessage);

    QFont baseFont = Settings::getInstance().getChatMessageFont();

    // Note: "[user]..." is just a placeholder. The actual text is set in
    // ChatForm::setFriendTyping()
    //
    // FIXME: Due to circumstances, placeholder is being used in a case where
    // user received typing notifications constantly since contact came online.
    // This causes "[user]..." to be displayed in place of user nick, as long
    // as user will keep typing. Issue #1280

    auto* item = new ChatNotificationBox("", baseFont);
    item->setIcon(new NotificationIcon(QSize(18, 18)));
    return IChatItem::Ptr(item);
}

/**
 * @brief Create message placeholder while chatform restructures text
 *
 * It can take a while for chatform to resize large amounts of text, thus
 * a message placeholder is needed to inform users about it.
 *
 * @return created message
 */
IChatItem::Ptr ChatMessage::createBusyNotification() {
    QFont baseFont = Settings::getInstance().getChatMessageFont();
    baseFont.setPixelSize(baseFont.pixelSize() + 2);
    baseFont.setBold(true);
    auto* item = new ChatNotificationBox(QObject::tr("Reformatting text in progress.."), baseFont);
    return IChatItem::Ptr(item);
}

QString ChatMessage::detectQuotes(const QString& str, MessageType type) {
    // detect text quotes
    QStringList messageLines = str.split("\n");
    QString quotedText;
    for (int i = 0; i < messageLines.size(); ++i) {
        // don't quote first line in action message. This makes co-existence of
        // quotes and action messages possible, since only first line can cause
        // problems in case where there is quote in it used.
        if (QRegExp("^(&gt;|＞).*").exactMatch(messageLines[i])) {
            if (i > 0 || type != ACTION)
                quotedText += "<span class=quote>" + messageLines[i] + " </span>";
            else
                quotedText += messageLines[i];
        } else {
            quotedText += messageLines[i];
        }

        if (i < messageLines.size() - 1) {
            quotedText += '\n';
        }
    }

    return quotedText;
}

QString ChatMessage::wrapDiv(const QString& str, const QString& div) {
    return QString("<p class=%1>%2</p>").arg(div, /*QChar(0x200E) + */ QString(str));
}
