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

#include "PainterWidgetProxy.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

PainterWidgetProxy::PainterWidgetProxy(QGraphicsItem* parent)
        : QGraphicsProxyWidget(parent), popupShown(false), currentPopup(0) {
    //    timeLine = new QTimeLine(250, this);
    //    connect(timeLine, SIGNAL(valueChanged(qreal)),
    //            this, SLOT(updateStep(qreal)));
    //    connect(timeLine, SIGNAL(stateChanged(QTimeLine::State)),
    //            this, SLOT(stateChanged(QTimeLine::State)));
}

QRectF PainterWidgetProxy::boundingRect() const {
    return QGraphicsProxyWidget::boundingRect().adjusted(0, 0, 10, 10);
}

void PainterWidgetProxy::paintWindowFrame(QPainter* painter,
                                          const QStyleOptionGraphicsItem* option,
                                          QWidget* widget) {
    const QColor color(0, 0, 0, 64);

    QRectF r = windowFrameRect();
    QRectF right(r.right(), r.top() + 10, 10, r.height() - 10);
    QRectF bottom(r.left() + 10, r.bottom(), r.width(), 10);
    bool intersectsRight = right.intersects(option->exposedRect);
    bool intersectsBottom = bottom.intersects(option->exposedRect);
    if (intersectsRight && intersectsBottom) {
        QPainterPath path;
        path.addRect(right);
        path.addRect(bottom);
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        painter->drawPath(path);
    } else if (intersectsBottom) {
        painter->fillRect(bottom, color);
    } else if (intersectsRight) {
        painter->fillRect(right, color);
    }

    QGraphicsProxyWidget::paintWindowFrame(painter, option, widget);
}

void PainterWidgetProxy::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    QGraphicsProxyWidget::hoverEnterEvent(event);
    //    scene()->setActiveWindow(this);
    //    if (timeLine->currentValue() != 1)
    //        zoomIn();
}

void PainterWidgetProxy::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    QGraphicsProxyWidget::hoverLeaveEvent(event);
    //    if (!popupShown
    //            && (timeLine->direction() != QTimeLine::Backward || timeLine->currentValue() !=
    //            0)) {
    //        zoomOut();
    //    }
}

bool PainterWidgetProxy::sceneEventFilter(QGraphicsItem* watched, QEvent* event) {
    //    if (watched->isWindow()
    //            && (event->type() == QEvent::UngrabMouse || event->type() == QEvent::GrabMouse)) {
    //        popupShown = watched->isVisible();
    //        if (!popupShown && !isUnderMouse())
    //            zoomOut();
    //    }
    return QGraphicsProxyWidget::sceneEventFilter(watched, event);
}

QVariant PainterWidgetProxy::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemChildAddedChange || change == ItemChildRemovedChange) {
        if (change == ItemChildAddedChange) {
            currentPopup = qvariant_cast<QGraphicsItem*>(value);
            currentPopup->setCacheMode(ItemCoordinateCache);
            if (scene()) currentPopup->installSceneEventFilter(this);
        } else if (scene()) {
            currentPopup->removeSceneEventFilter(this);
            currentPopup = 0;
        }
    } else if (currentPopup && change == ItemSceneHasChanged) {
        currentPopup->installSceneEventFilter(this);
    }
    return QGraphicsProxyWidget::itemChange(change, value);
}

void PainterWidgetProxy::updateStep(qreal step) {
    //    QRectF r = boundingRect();
    //    setTransform(QTransform()
    //                 .translate(r.width() / 2, r.height() / 2)
    //                 .rotate(step * 30, Qt::XAxis)
    //                 .rotate(step * 10, Qt::YAxis)
    //                 .rotate(step * 5, Qt::ZAxis)
    //                 .scale(1 + 1.5 * step, 1 + 1.5 * step)
    //                 .translate(-r.width() / 2, -r.height() / 2));
}

void PainterWidgetProxy::stateChanged(QTimeLine::State state) {
    //    if (state == QTimeLine::Running) {
    //        if (timeLine->direction() == QTimeLine::Forward)
    //            setCacheMode(ItemCoordinateCache);
    //    } else if (state == QTimeLine::NotRunning) {
    //        if (timeLine->direction() == QTimeLine::Backward)
    //            setCacheMode(DeviceCoordinateCache);
    //    }
}

void PainterWidgetProxy::zoomIn() {
    if (timeLine->direction() != QTimeLine::Forward) timeLine->setDirection(QTimeLine::Forward);
    if (timeLine->state() == QTimeLine::NotRunning) timeLine->start();
}

void PainterWidgetProxy::zoomOut() {
    if (timeLine->direction() != QTimeLine::Backward) timeLine->setDirection(QTimeLine::Backward);
    if (timeLine->state() == QTimeLine::NotRunning) timeLine->start();
}

void PainterWidgetProxy::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    return QGraphicsProxyWidget::mouseMoveEvent(event);
}

void PainterWidgetProxy::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    return QGraphicsProxyWidget::mousePressEvent(event);
}

void PainterWidgetProxy::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    return QGraphicsProxyWidget::mouseReleaseEvent(event);
}
