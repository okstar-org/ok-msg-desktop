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

#ifndef BROKEN_H
#define BROKEN_H

#include "../chatlinecontent.h"

#include <QObject>
#include <QPixmap>
namespace module::im {

class Broken : public ChatLineContent {
    Q_OBJECT
public:
    Broken(const QString& img, QSize size);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setWidth(qreal width) override;
    void visibilityChanged(bool visible) override;
    qreal getAscent() const override;
    const void* getContent() override;

protected:
    virtual void onCopyEvent() override;

private:
    QSize size;
    QPixmap pmap;
};

}  // namespace module::im
#endif  // BROKEN_H
