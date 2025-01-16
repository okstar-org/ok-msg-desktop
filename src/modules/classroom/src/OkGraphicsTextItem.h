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

#pragma once
#include <QGraphicsTextItem>
#include <QPen>

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

namespace module::classroom {
class OkGraphicsTextItem : public QGraphicsTextItem {
    Q_OBJECT
public:
    explicit OkGraphicsTextItem(QGraphicsItem* parent = nullptr);

signals:
    void lostFocus(OkGraphicsTextItem* item);

    void itemChanged(OkGraphicsTextItem* item, GraphicsItemChange change, const QVariant& value);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    void focusOutEvent(QFocusEvent* event) override;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}  // namespace module::classroom
