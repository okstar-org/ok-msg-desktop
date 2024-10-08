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

#include "chatlinecontent.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include "src/chatlog/content/text.h"

void ChatLineContent::setIndex(int r, int c) {
    row = r;
    col = c;
}

ChatLineContent::ChatLineContent(ContentType type, QObject* parent)
        : QObject(parent), contentType{type} {
    setFlag(QGraphicsItem::ItemIsSelectable);
}

int ChatLineContent::getColumn() const { return col; }

int ChatLineContent::getRow() const { return row; }

int ChatLineContent::type() const { return GraphicsItemType::ChatLineContentType; }

void ChatLineContent::selectionMouseMove(QPointF) {}

void ChatLineContent::selectionStarted(QPointF) {}

void ChatLineContent::selectionCleared() {}

void ChatLineContent::selectionDoubleClick(QPointF) {}

void ChatLineContent::selectionTripleClick(QPointF) {}

void ChatLineContent::selectionFocusChanged(bool) {}

void ChatLineContent::selectAll() {}

bool ChatLineContent::isOverSelection(QPointF) const { return false; }

QString ChatLineContent::getSelectedText() const { return QString(); }

void ChatLineContent::fontChanged(const QFont& font) { Q_UNUSED(font); }

qreal ChatLineContent::getAscent() const { return 0.0; }

void ChatLineContent::visibilityChanged(bool) {}

void ChatLineContent::reloadTheme() {}

void ChatLineContent::initMenu() {}

void ChatLineContent::doReplySelected() {
    qDebug() << __func__;
    emit reply();
}

void ChatLineContent::doCopySelectedText() {
    qDebug() << __func__;
    onCopyEvent();
    emit copy();
}

void ChatLineContent::doForwardSelectedText() {
    qDebug() << __func__;
    emit forward();
}

void ChatLineContent::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    QMenu menu;
    auto copyAction =
            menu.addAction(QIcon(), tr("Copy"), this, &ChatLineContent::doCopySelectedText);
    auto replyAction =
            menu.addAction(QIcon(), tr("Reply"), this, &ChatLineContent::doReplySelected);
    auto forwardAction =
            menu.addAction(QIcon(), tr("Forward"), this, &ChatLineContent::doForwardSelectedText);

    QAction* selectedAction = menu.exec(event->screenPos());
    if (!selectedAction) {
        return;
    }

    //     if (selectedAction == replyAction) {
    //         // 执行Action 1对应的操作
    //     } else if (selectedAction == forwardAction) {
    //         // 执行Action 2对应的操作
    //     } else if (selectedAction == copyAction) {
    //         //
    // //        doCopySelectedText();
    //     }
}

const QString& ChatLineContent::getContentAsText() { return *(QString*)getContent(); }

// void ChatLineContent::mousePressEvent(QGraphicsSceneMouseEvent *event)
// {
// if (event->button() == Qt::RightButton) {
//     qDebug() << "Right mouse button pressed on MyGraphicsItem";
//     // 在这里处理右键点击事件

// }

// return;
// event->ignore();
// event->set
// QGraphicsItem::mousePressEvent(event);  // 确保调用基类实现以进行默认处理（如果需要）

// }

