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

#ifndef TOOLBOXGRAPHICSITEM_HPP
#define TOOLBOXGRAPHICSITEM_HPP

#include <QGraphicsItemGroup>
#include <QObject>
#include <QPropertyAnimation>

class ToolBoxGraphicsItem final : public QObject, public QGraphicsItemGroup {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    ToolBoxGraphicsItem();
    ~ToolBoxGraphicsItem();

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget) final override;

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) final override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) final override;

private:
    void startAnimation(QAbstractAnimation::Direction direction);

    QPropertyAnimation* opacityAnimation;
    qreal idleOpacity = 0.0f;
    qreal activeOpacity = 1.0f;
    int fadeTimeMs = 300;
};

#endif  // TOOLBOXGRAPHICSITEM_HPP
