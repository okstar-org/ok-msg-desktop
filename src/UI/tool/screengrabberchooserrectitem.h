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

#ifndef SCREENGRABBERCHOOSERRECTITEM_HPP
#define SCREENGRABBERCHOOSERRECTITEM_HPP

#include <QGraphicsItemGroup>

class ScreenGrabberChooserRectItem final : public QObject, public QGraphicsItemGroup {
    Q_OBJECT
public:
    explicit ScreenGrabberChooserRectItem(QGraphicsScene* scene);
    ~ScreenGrabberChooserRectItem();

    virtual QRectF boundingRect() const final override;
    void beginResize(QPointF mousePos);

    QRect chosenRect() const;

    void showHandles();
    void hideHandles();

signals:

    void doubleClicked();
    void regionChosen(QRect rect);

protected:
    virtual bool sceneEventFilter(QGraphicsItem* watched, QEvent* event) final override;

private:
    enum State {
        None,
        Resizing,
        HandleResizing,
        Moving,
    };

    State state = None;
    int rectWidth = 0;
    int rectHeight = 0;
    QPointF startPos;

    void forwardMainRectEvent(QEvent* event);
    void forwardHandleEvent(QGraphicsItem* watched, QEvent* event);

    void mousePress(QGraphicsSceneMouseEvent* event);
    void mouseMove(QGraphicsSceneMouseEvent* event);
    void mouseRelease(QGraphicsSceneMouseEvent* event);
    void mouseDoubleClick(QGraphicsSceneMouseEvent* event);

    void mousePressHandle(int x, int y, QGraphicsSceneMouseEvent* event);
    void mouseMoveHandle(int x, int y, QGraphicsSceneMouseEvent* event);
    void mouseReleaseHandle(int x, int y, QGraphicsSceneMouseEvent* event);

    QPoint getHandleMultiplier(QGraphicsItem* handle);

    void updateHandlePositions();
    QGraphicsRectItem* createHandleItem(QGraphicsScene* scene);

    QGraphicsRectItem* mainRect;
    QGraphicsRectItem* topLeft;
    QGraphicsRectItem* topCenter;
    QGraphicsRectItem* topRight;
    QGraphicsRectItem* rightCenter;
    QGraphicsRectItem* bottomRight;
    QGraphicsRectItem* bottomCenter;
    QGraphicsRectItem* bottomLeft;
    QGraphicsRectItem* leftCenter;
};

#endif  // SCREENGRABBERCHOOSERRECTITEM_HPP
