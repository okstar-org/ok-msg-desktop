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

#include "toolboxgraphicsitem.h"

#include <QPainter>

ToolBoxGraphicsItem::ToolBoxGraphicsItem() {
    this->opacityAnimation = new QPropertyAnimation(this, QByteArrayLiteral("opacity"), this);

    this->opacityAnimation->setKeyValueAt(0, this->idleOpacity);
    this->opacityAnimation->setKeyValueAt(1, this->activeOpacity);
    this->opacityAnimation->setDuration(this->fadeTimeMs);

    setOpacity(this->activeOpacity);
}

ToolBoxGraphicsItem::~ToolBoxGraphicsItem() {}

void ToolBoxGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    startAnimation(QAbstractAnimation::Backward);
    QGraphicsItemGroup::hoverEnterEvent(event);
}

void ToolBoxGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    startAnimation(QAbstractAnimation::Forward);
    QGraphicsItemGroup::hoverLeaveEvent(event);
}

void ToolBoxGraphicsItem::startAnimation(QAbstractAnimation::Direction direction) {
    this->opacityAnimation->setDirection(direction);
    this->opacityAnimation->start();
}

void ToolBoxGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                                QWidget* widget) {
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(0xFF, 0xE2, 0x82)));
    painter->drawRect(childrenBoundingRect());
    painter->restore();

    QGraphicsItemGroup::paint(painter, option, widget);
}
