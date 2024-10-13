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
    setCursor(Qt::PointingHandCursor);
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

void ChatLineContent::doReplySelected() { emit reply(); }

void ChatLineContent::doCopySelectedText() {
    onCopyEvent();
    emit copy();
}

void ChatLineContent::doForwardSelectedText() { emit forward(); }

void ChatLineContent::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    showContextMenu(event->screenPos());
}

const QString& ChatLineContent::getContentAsText() { return *(QString*)getContent(); }

void ChatLineContent::onContextMenu(QGraphicsItem* item, QPoint pos) {
    if (this == item) {
        qDebug() << "Selected" << getSelectedText();
        showContextMenu(pos);
    }
}

void ChatLineContent::showContextMenu(const QPoint& pos) const {
    QMenu menu;
    menu.addAction(QIcon(), tr("Copy"), this, &ChatLineContent::doCopySelectedText);
    menu.addAction(QIcon(), tr("Reply"), this, &ChatLineContent::doReplySelected);
    menu.addAction(QIcon(), tr("Forward"), this, &ChatLineContent::doForwardSelectedText);

    QAction* selectedAction = menu.exec(pos);
    qDebug() << "Selected action:" << selectedAction;
}
