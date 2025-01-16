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

#include "OkGraphicsTextItem.h"
namespace module::painter {

OkGraphicsTextItem::OkGraphicsTextItem(QGraphicsItem* parent) : QGraphicsTextItem(parent) {
    setTextInteractionFlags(Qt::TextInteractionFlag::TextEditorInteraction);
    setFlags(QGraphicsItem::ItemIsSelectable          //
             | QGraphicsItem::ItemIsMovable           //
             | QGraphicsItem::ItemIsFocusable         //
             | QGraphicsItem::ItemAcceptsInputMethod  //
             | QGraphicsItem::ItemIsPanel);
    setEnabled(true);
    setActive(true);
    setVisible(true);
}

QVariant OkGraphicsTextItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    // emit itemChanged(this, change, value);
    if (change == QGraphicsItem::ItemSelectedHasChanged) emit itemChanged(this, change, value);
    return QGraphicsTextItem::itemChange(change, value);
}

void OkGraphicsTextItem::focusOutEvent(QFocusEvent* event) {
    setTextInteractionFlags(Qt::NoTextInteraction);
    QGraphicsTextItem::focusOutEvent(event);
    emit lostFocus(this);
    QGraphicsTextItem::focusInEvent(event);
}

void OkGraphicsTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    if (textInteractionFlags() == Qt::NoTextInteraction)
        setTextInteractionFlags(Qt::TextEditorInteraction);

    QGraphicsTextItem::mouseDoubleClickEvent(event);
}
}  // namespace module::painter