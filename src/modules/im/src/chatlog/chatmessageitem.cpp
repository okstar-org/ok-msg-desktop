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

#include "chatmessageitem.h"
#include <QGraphicsProxyWidget>

#include "chatlog.h"
#include "content/broken.h"
#include "content/contactavatar.h"
#include "content/simpletext.h"
#include "content/spinner.h"
#include "content/text.h"
#include "src/nexus.h"
#include "src/widget/widget.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"


namespace module::im {
ChatMessageBox::ChatMessageBox(const QPixmap& avatar,
                               const QString& contactName,
                               const QString& message,
                               const QString& id,
                               bool isSelf)
        : IChatItem{id}, _IsSelf{isSelf} {
    QFont baseFont = Nexus::getProfile()->getSettings()->getChatMessageFont();
    auto text = new Text(message, baseFont, false, message);
    text->setBoundingRadius(4.0);
    text->setContentsMargins(QMarginsF(3, 3, 3, 3));
    init(avatar, contactName, text);
}

ChatMessageBox::ChatMessageBox(const QPixmap& avatar, const QString& contactName,
                               ChatLineContent* messageItem_, const QString& id, bool isSelf)
        : IChatItem{id}, _IsSelf{isSelf} {
    init(avatar, contactName, messageItem_);
}

void ChatMessageBox::init(const QPixmap& avatar, const QString& contactName,
                          ChatLineContent* messageItem_) {
    messageItem = messageItem_;
    connect(messageItem, &ChatLineContent::reply, this, &ChatMessageBox::doReply);
    connect(messageItem, &ChatLineContent::forward, this, &ChatMessageBox::doForward);

    QFont baseFont = Nexus::getProfile()->getSettings()->getChatMessageFont();
    QFont nameFont = nicknameFont(baseFont);
    avatarItem = new ContactAvatar(const_cast<QPixmap&>(avatar));
    connect(avatarItem, &ChatLineContent::reply, this, &ChatMessageBox::doReply);
    connect(avatarItem, &ChatLineContent::copy, this, &ChatMessageBox::doCopy);
    connect(avatarItem, &ChatLineContent::forward, this, &ChatMessageBox::doForward);

    nicknameItem = new SimpleText(contactName, nameFont);
    nicknameItem->setColor(lib::settings::Style::ColorPalette::NameActive);
    connect(nicknameItem, &ChatLineContent::reply, this, &ChatMessageBox::doReply);
    connect(nicknameItem, &ChatLineContent::copy, this, &ChatMessageBox::doCopy);
    connect(nicknameItem, &ChatLineContent::forward, this, &ChatMessageBox::doForward);

    customMsg = true;

    if (_IsSelf) {
        setLayoutDirection(Qt::RightToLeft);
        setShowNickname(false);
    }
    reloadTheme();
}

void ChatMessageBox::setMessageState(MessageState state) {
    if (msgState == state) return;
    msgState = state;
    if (stateItem) {
        if (stateItem->scene()) stateItem->scene()->removeItem(stateItem);
        delete stateItem;
        stateItem = nullptr;
    }
    switch (state) {
        case MessageState::pending: {
            stateItem = new Spinner(lib::settings::Style::getImagePath("chatArea/spinner.svg"), QSize(16, 16),
                                    360.0 / 1.6);
        } break;
        case MessageState::broken: {
            stateItem = new Broken(lib::settings::Style::getImagePath("chatArea/error.svg"), QSize(16, 16));
        } break;
        default:
            break;
    }
}

void ChatMessageBox::layout(qreal width, QPointF scenePos) {
    //    qDebug() << __func__ << id << this;

    auto mirrorPos = [width, scenePos, this](QPointF& pos, qreal offset) {
        if (this->layoutDirection == Qt::RightToLeft)
            pos.rx() = width + scenePos.x() - pos.x() + scenePos.x() - offset;
    };
    qreal avatar_width = 0;
    qreal message_offset = 0;
    QSizeF msg_size;
    if (avatarItem && messageItem) {
        avatar_width = avatarItem->boundingRect().width();

        QPointF avat_pos(scenePos);
        mirrorPos(avat_pos, avatar_width);
        avatarItem->setPos(avat_pos - avatarItem->boundingRect().topLeft());

        if (showNickname && nicknameItem) {
            QPointF name_pos = scenePos + QPointF(avatar_width + 6.0, -3.0);
            mirrorPos(name_pos, nicknameItem->boundingRect().width());
            nicknameItem->setPos(name_pos - nicknameItem->boundingRect().topLeft());
            message_offset = nicknameItem->boundingRect().height() - 3.0 + 6.0;
        }

        qreal width_limit = width * 0.667;
        messageItem->setWidth(-1);
        if (messageItem->boundingRect().width() > width_limit) {
            messageItem->setWidth(width_limit);
        }
        QPointF msg_pos = scenePos + QPointF(avatar_width + 6.0, message_offset);
        mirrorPos(msg_pos, messageItem->boundingRect().width());
        messageItem->setPos(msg_pos - messageItem->boundingRect().topLeft());

        msg_size = messageItem->boundingRect().size();
    }

    if (stateItem) {
        QRectF state_rect = stateItem->boundingRect();
        QPointF state_pos = scenePos + QPointF(avatar_width + 6.0, message_offset);
        state_pos.rx() += msg_size.width() + 6.0;
        state_pos.ry() += (msg_size.height() - state_rect.height()) / 2;
        mirrorPos(state_pos, state_rect.width());
        stateItem->setPos(state_pos - state_rect.topLeft());
    }
    this->scenePos = scenePos;
}

QRectF ChatMessageBox::sceneBoundingRect() const {
    if (!avatarItem) {
        return QRectF(10, 10, 10, 10);
    }
    QRectF itemRect = avatarItem->sceneBoundingRect();
    if (showNickname)
        itemRect = itemRect.united(messageItem->sceneBoundingRect())
                           .united(messageItem->sceneBoundingRect());
    else
        itemRect = itemRect.united(messageItem->sceneBoundingRect());

    if (stateItem) itemRect = itemRect.united(stateItem->sceneBoundingRect());
    return itemRect;
}

void ChatMessageBox::setVisible(bool visible) {
    avatarItem->setVisible(visible);
    if (showNickname) {
        nicknameItem->setVisible(visible);
    }
    messageItem->setVisible(visible);
    if (stateItem) {
        stateItem->setVisible(visible);
    }
}

void ChatMessageBox::markAsDelivered(const QDateTime& time) {
    setMessageState(MessageState::complete);
}

ChatLineContent* ChatMessageBox::contentAtPos(QPointF scenePos) const {
    if (messageItem && messageItem->sceneBoundingRect().contains(scenePos)) return messageItem;
    return nullptr;
}

ChatLineContent* ChatMessageBox::centerContent() const { return messageItem; }

void ChatMessageBox::setLayoutDirection(Qt::LayoutDirection direction) {
    layoutDirection = direction;
}

void ChatMessageBox::setShowNickname(bool show) {
    showNickname = show;

    nicknameItem->setVisible(show);
    nicknameItem->visibilityChanged(show);
}

QList<ChatLineContent*> ChatMessageBox::contents() {
    QList<ChatLineContent*> result({avatarItem, nicknameItem, messageItem});
    if (stateItem) result.append(stateItem);
    return result;
}

void ChatMessageBox::reloadTheme() {
    IChatItem::reloadTheme();
    updateTextTheme();
}

QFont ChatMessageBox::nicknameFont(const QFont& baseFont) {
    QFont font = baseFont;
    font.setPixelSize(font.pixelSize() - 1);
    return font;
}

void ChatMessageBox::updateTextTheme() {
    //    if (customMsg) return;
    if (Text* text_item = dynamic_cast<Text*>(messageItem)) {
        if (_IsSelf) {
            text_item->setBackgroundColor(lib::settings::Style::getExtColor("chat.message.self.background"));
            text_item->setColor(lib::settings::Style::getExtColor("chat.message.self.color"));
        } else {
            text_item->setBackgroundColor(lib::settings::Style::getExtColor("chat.message.background"));
            text_item->setColor(lib::settings::Style::getExtColor("chat.message.color"));
        }
    }
}

IChatItemType ChatMessageBox::itemType() { return IChatItemType::TEXT; }

void ChatMessageBox::doReply() {
    qDebug() << __func__ << id;
    emit replyEvent(this);
}

void ChatMessageBox::doCopy() { messageItem->doCopySelectedText(); }

void ChatMessageBox::doForward() {
    qDebug() << __func__ << id;
    auto w = Widget::getInstance();
    emit w->toForwardMessage(id);
}

void ChatMessageBox::doSetAvatar(const QPixmap& pixmap) {
    avatarItem->setPixmap(pixmap);
}

const QString& ChatMessageBox::getNickname() { return *(QString*)nicknameItem->getContent(); }

QString ChatMessageBox::getContent() { return *(QString*)centerContent()->getContent(); }

ChatNotificationBox::ChatNotificationBox(const QString& message, const QFont& font) {
    textItem = new Text(message, font);
    textItem->setWidth(-1);
    textItem->setTextSelectable(false);
}

void ChatNotificationBox::setIcon(ChatLineContent* item) {
    if (iconItem && iconItem != item) {
        if (iconItem->scene()) iconItem->scene()->removeItem(iconItem);
        delete iconItem;
    }
    iconItem = item;
}

void ChatNotificationBox::layout(qreal width, QPointF scenePos) {
    QRectF text_rect = textItem->boundingRect();
    if (iconItem) {
        QRectF icon_rect = iconItem->boundingRect();
        qreal ctx_width = icon_rect.width() + text_rect.width();
        qreal ctx_height = std::max(icon_rect.height(), text_rect.height());
        qreal ctx_x = scenePos.x() + (width - ctx_width) / 2;
        qreal ctx_y = scenePos.y();
        QRectF temp(ctx_x, ctx_y, icon_rect.width(), ctx_height);
        icon_rect.moveCenter(temp.center() - icon_rect.topLeft());
        iconItem->setPos(icon_rect.topLeft());

        temp = QRectF(ctx_x + icon_rect.width(), ctx_y, text_rect.width(), ctx_height);
        text_rect.moveCenter(temp.center() - text_rect.topLeft());
        textItem->setPos(text_rect.topLeft());
    } else {
        qreal x = scenePos.x() + (width - text_rect.width()) / 2;
        textItem->setPos(x, scenePos.y());
    }
}

QRectF ChatNotificationBox::sceneBoundingRect() const {
    if (iconItem) {
        return iconItem->sceneBoundingRect().united(textItem->sceneBoundingRect());
    }
    return textItem->sceneBoundingRect();
}

ChatLineContent* ChatNotificationBox::contentAtPos(QPointF scenePos) const {
    if (textItem && textItem->sceneBoundingRect().contains(scenePos)) return textItem;
    return nullptr;
}

ChatLineContent* ChatNotificationBox::centerContent() const { return textItem; }

IChatItemType ChatNotificationBox::itemType() { return IChatItemType::TEXT; }

QList<ChatLineContent*> ChatNotificationBox::contents() {
    if (iconItem) return QList<ChatLineContent*>({iconItem, textItem});
    return QList<ChatLineContent*>({textItem});
}
}  // namespace module::im
