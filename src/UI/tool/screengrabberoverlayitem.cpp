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

#include "screengrabberoverlayitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

#include "screenshotgrabber.h"

ScreenGrabberOverlayItem::ScreenGrabberOverlayItem(ScreenshotGrabber* grabber)
        : screnshootGrabber(grabber) {
    QBrush overlayBrush(QColor(0x00, 0x00, 0x00, 0x70));  // Translucent black

    setCursor(QCursor(Qt::CrossCursor));
    setBrush(overlayBrush);
    setPen(QPen(Qt::NoPen));
}

ScreenGrabberOverlayItem::~ScreenGrabberOverlayItem() {}

void ScreenGrabberOverlayItem::setChosenRect(QRect rect) {
    QRect oldRect = chosenRect;
    chosenRect = rect;
    update(oldRect.united(rect));
}

void ScreenGrabberOverlayItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) this->screnshootGrabber->beginRectChooser(event);
}

void ScreenGrabberOverlayItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setBrush(brush());
    painter->setPen(pen());

    QRectF self = rect();
    qreal leftX = chosenRect.x();
    qreal rightX = chosenRect.x() + chosenRect.width();
    qreal topY = chosenRect.y();
    qreal bottomY = chosenRect.y() + chosenRect.height();

    painter->drawRect(0, 0, leftX, self.height());                       // Left of chosen
    painter->drawRect(rightX, 0, self.width() - rightX, self.height());  // Right of chosen
    painter->drawRect(leftX, 0, chosenRect.width(), topY);               // Top of chosen
    painter->drawRect(leftX, bottomY, chosenRect.width(),
                      self.height() - bottomY);  // Bottom of chosen
}
