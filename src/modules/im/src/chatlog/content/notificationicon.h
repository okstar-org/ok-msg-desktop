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

#ifndef NOTIFICATIONICON_H
#define NOTIFICATIONICON_H

#include "../chatlinecontent.h"

#include <QLinearGradient>
#include <QPixmap>

class QTimer;

class NotificationIcon : public ChatLineContent {
    Q_OBJECT
public:
    explicit NotificationIcon(QSize size);

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget) override;
    virtual void setWidth(qreal width) override;
    virtual qreal getAscent() const override;

private slots:
    void updateGradient();

private:
    QSize size;
    QPixmap pmap;
    QLinearGradient grad;
    QTimer* updateTimer = nullptr;

    qreal dotWidth = 0.2;
    qreal alpha = 0.0;
};

#endif  // NOTIFICATIONICON_H
