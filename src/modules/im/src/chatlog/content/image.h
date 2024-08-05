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

#ifndef IMAGE_H
#define IMAGE_H

#include "../chatlinecontent.h"

#include <QPixmap>

class Image : public ChatLineContent {
public:
    Image(QSize size, const QString& filename);

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget) override;
    virtual void setWidth(qreal width) override;
    virtual qreal getAscent() const override;

private:
    QSize size;
    QPixmap pmap;
};

#endif  // IMAGE_H
