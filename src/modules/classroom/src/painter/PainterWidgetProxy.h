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

#ifndef PAINTER_WIDGETPROXY_H
#define PAINTER_WIDGETPROXY_H

#include <QGraphicsProxyWidget>
#include <QTimeLine>

class PainterWidgetProxy : public QGraphicsProxyWidget {
    Q_OBJECT

public:
    explicit PainterWidgetProxy(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paintWindowFrame(QPainter* painter, const QStyleOptionGraphicsItem* option,
                          QWidget* widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    bool sceneEventFilter(QGraphicsItem* watched, QEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private slots:
    void updateStep(qreal step);
    void stateChanged(QTimeLine::State);
    void zoomIn();
    void zoomOut();

private:
    QTimeLine* timeLine;
    bool popupShown;
    QGraphicsItem* currentPopup;
};

#endif  // CUSTOMPROXY_H
