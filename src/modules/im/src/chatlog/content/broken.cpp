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

#include "broken.h"
#include <QPainter>
#include "src/chatlog/pixmapcache.h"

class QStyleOptionGraphicsItem;

Broken::Broken(const QString& img, QSize size)
        : pmap{PixmapCache::getInstance().get(img, size)}, size{size} {}

QRectF Broken::boundingRect() const {
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

void Broken::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->translate(QPointF(-size.width() / 2.0, -size.height() / 2.0));
    painter->drawPixmap(0, 0, pmap);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void Broken::setWidth(qreal width) { Q_UNUSED(width); }

void Broken::visibilityChanged(bool visible) { Q_UNUSED(visible); }

qreal Broken::getAscent() const { return 0.0; }
