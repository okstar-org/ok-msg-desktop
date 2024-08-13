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

#include "notificationicon.h"
#include "../pixmapcache.h"
#include "src/lib/settings/style.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QTimer>

NotificationIcon::NotificationIcon(QSize Size) : size(Size) {
    pmap = PixmapCache::getInstance().get(Style::getImagePath("chatArea/typing.svg"), size);

    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000 / 30);
    updateTimer->setSingleShot(false);

    updateTimer->start();

    connect(updateTimer, &QTimer::timeout, this, &NotificationIcon::updateGradient);
}

QRectF NotificationIcon::boundingRect() const {
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

void NotificationIcon::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                             QWidget* widget) {
    painter->setClipRect(boundingRect());

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->translate(-size.width() / 2.0, -size.height() / 2.0);

    painter->fillRect(QRect(0, 0, size.width(), size.height()), grad);
    painter->drawPixmap(0, 0, size.width(), size.height(), pmap);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void NotificationIcon::setWidth(qreal width) { Q_UNUSED(width) }

qreal NotificationIcon::getAscent() const { return 3.0; }

void NotificationIcon::updateGradient() {
    alpha += 0.01;

    if (alpha + dotWidth >= 1.0) alpha = 0.0;

    grad = QLinearGradient(QPointF(-0.5 * size.width(), 0), QPointF(3.0 / 2.0 * size.width(), 0));
    grad.setColorAt(0, Qt::lightGray);
    grad.setColorAt(qMax(0.0, alpha - dotWidth), Qt::lightGray);
    grad.setColorAt(alpha, Qt::black);
    grad.setColorAt(qMin(1.0, alpha + dotWidth), Qt::lightGray);
    grad.setColorAt(1, Qt::lightGray);

    if (scene() && isVisible()) scene()->invalidate(sceneBoundingRect());
}
