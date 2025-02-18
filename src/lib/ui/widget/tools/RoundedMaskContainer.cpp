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

//
// Created by gaojie on 25-02-18.
//

#include "RoundedMaskContainer.h"

#include <QGraphicsEffect>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>

namespace lib::ui{
class RoundMaskGraphicsEffect : public QGraphicsEffect {
public:
    RoundMaskGraphicsEffect(QWidget* target) : QGraphicsEffect(target), _target(target) {}

    QRectF boundingRectFor(const QRectF& rect) const { return rect.adjusted(-1, -1, 1, 1); }
    void setRadius(int r) {
        radius = r;
        update();
    }

protected:
    void draw(QPainter* painter) {
        if (radius <= 0) {
            drawSource(painter);
            return;
        }

        QPoint offset;
        Qt::CoordinateSystem system =
                sourceIsPixmap() ? Qt::LogicalCoordinates : Qt::DeviceCoordinates;
        QPixmap pixmap = sourcePixmap(system, &offset, QGraphicsEffect::NoPad);
        if (pixmap.isNull()) return;

        painter->save();
        QPainter pixmapPainter(&pixmap);
        pixmapPainter.setRenderHints(QPainter::Antialiasing);
        pixmapPainter.setPen(Qt::NoPen);
        pixmapPainter.setBrush(Qt::red);
        pixmapPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);

        QPainterPath path;
        QRect rect(QPoint(0, 0), _target->size());
        path.addRect(rect.adjusted(-1, -1, 1, 1));
        path.addRoundedRect(rect, radius, radius);
        if (system == Qt::DeviceCoordinates) {
            QTransform worldTransform = painter->worldTransform();
            worldTransform *= QTransform::fromTranslate(-offset.x(), -offset.y());
            pixmapPainter.setWorldTransform(worldTransform);
        } else {
            pixmapPainter.translate(-offset);
        }
        pixmapPainter.drawPath(path);
        pixmapPainter.end();
        painter->setWorldTransform(QTransform());
        painter->drawPixmap(offset, pixmap);
        painter->restore();
    }

private:
    QPointer<QWidget> _target;
    int radius = 0;
};

RoundedMaskContainer::RoundedMaskContainer(QWidget* parent) : QFrame(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    effect = new RoundMaskGraphicsEffect(this);
    setGraphicsEffect(effect);
}

void RoundedMaskContainer::setRoundRadius(int radius) {
    if (_radius != radius) {
        _radius = radius;
        effect->setRadius(radius);
    }
}
}
