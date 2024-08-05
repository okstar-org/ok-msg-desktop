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

#include "spinner.h"
#include "../pixmapcache.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QPainter>
#include <QTime>
#include <QVariantAnimation>

Spinner::Spinner(const QString& img, QSize Size, qreal speed) : size(Size), rotSpeed(speed) {
    pmap = PixmapCache::getInstance().get(img, size);

    timer.setInterval(1000 / 30);  // 30Hz
    timer.setSingleShot(false);

    blendAnimation = new QVariantAnimation(this);
    blendAnimation->setStartValue(0.0);
    blendAnimation->setEndValue(1.0);
    blendAnimation->setDuration(350);
    blendAnimation->setEasingCurve(QEasingCurve::InCubic);
    blendAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    connect(blendAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& val) { alpha = val.toDouble(); });

    QObject::connect(&timer, &QTimer::timeout, this, &Spinner::timeout);
}

QRectF Spinner::boundingRect() const {
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

void Spinner::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setClipRect(boundingRect());

    QTransform trans =
            QTransform()
                    .rotate(QTime::currentTime().msecsSinceStartOfDay() / 1000.0 * rotSpeed)
                    .translate(-size.width() / 2.0, -size.height() / 2.0);
    painter->setOpacity(alpha);
    painter->setTransform(trans, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(0, 0, pmap);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void Spinner::setWidth(qreal width) { Q_UNUSED(width) }

void Spinner::visibilityChanged(bool visible) {
    if (visible)
        timer.start();
    else
        timer.stop();
}

qreal Spinner::getAscent() const { return 0.0; }

void Spinner::timeout() {
    if (scene()) scene()->invalidate(sceneBoundingRect());
}
