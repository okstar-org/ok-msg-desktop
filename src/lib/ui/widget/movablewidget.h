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

#ifndef MOVABLEWIDGET_H
#define MOVABLEWIDGET_H

#include <QWidget>

class MovableWidget : public QWidget {
public:
    explicit MovableWidget(QWidget* parent);
    void resetBoundary(QRect newBoundary);
    void setBoundary(QRect newBoundary);
    float getRatio() const;
    void setRatio(float r);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

private:
    void checkBoundary(QPoint& point) const;
    void checkBoundaryLeft(int& x) const;

    typedef uint8_t Modes;

    enum Mode : Modes {
        Moving = 0x01,
        ResizeLeft = 0x02,
        ResizeRight = 0x04,
        ResizeUp = 0x08,
        ResizeDown = 0x10,
        Resize = ResizeLeft | ResizeRight | ResizeUp | ResizeDown
    };

    Modes mode = 0;
    QPoint lastPoint;
    QRect boundaryRect;
    QSizeF actualSize;
    QPointF actualPos;
    float ratio;
};

#endif  // MOVABLEWIDGET_H
