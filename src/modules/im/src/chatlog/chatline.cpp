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

#include "chatline.h"
#include "chatlinecontent.h"
#include "content/notificationicon.h"

#include <QDebug>
#include <QGraphicsScene>

void IChatItem::moveBy(qreal dx, qreal dy) {
    for (ChatLineContent* content : contents()) {
        content->moveBy(dx, dy);
    }
}

void IChatItem::addToScene(QGraphicsScene* scene) {
    for (ChatLineContent* content : contents()) {
        scene->addItem(content);
    }
}

void IChatItem::removeFromScene() {
    for (ChatLineContent* content : contents()) {
        if (content->scene()) content->scene()->removeItem(content);
    }
}

void IChatItem::setVisible(bool visible) {
    for (ChatLineContent* content : contents()) {
        content->setVisible(visible);
    }
}

ChatLineContent* IChatItem::centerContent() const { return nullptr; }

void IChatItem::visibilityChanged(bool visible) {
    for (ChatLineContent* content : contents()) {
        content->visibilityChanged(visible);
    }
}

void IChatItem::selectionFocusChanged(bool focusIn) {
    for (ChatLineContent* content : contents()) {
        content->selectionFocusChanged(focusIn);
    }
}

void IChatItem::selectionCleared() {
    for (ChatLineContent* content : contents()) {
        content->selectionCleared();
    }
}

void IChatItem::selectAll() {
    for (ChatLineContent* content : contents()) {
        content->selectAll();
    }
}

void IChatItem::setRow(int row) {
    this->row = row;
    auto content = centerContent();
    if (content) content->setIndex(row, 0);
}

void IChatItem::fontChanged(const QFont& font) {
    for (ChatLineContent* content : contents()) {
        content->fontChanged(font);
    }
}

void IChatItem::reloadTheme() {
    for (ChatLineContent* content : contents()) {
        content->reloadTheme();
    }
}
