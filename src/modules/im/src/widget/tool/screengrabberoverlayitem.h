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

#ifndef SCREENGRABBEROVERLAYITEM_HPP
#define SCREENGRABBEROVERLAYITEM_HPP

#include <QGraphicsRectItem>

class ScreenshotGrabber;

class ScreenGrabberOverlayItem final : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    explicit ScreenGrabberOverlayItem(ScreenshotGrabber* grabber);
    ~ScreenGrabberOverlayItem();

    void setChosenRect(QRect rect);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) final override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget) final override;

private:
    ScreenshotGrabber* screnshootGrabber;

    QRect chosenRect;
};

#endif  // SCREENGRABBEROVERLAYITEM_HPP
