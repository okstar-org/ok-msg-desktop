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

#ifndef SIMPLETEXT_H
#define SIMPLETEXT_H

#include "../chatlinecontent.h"
#include "src/lib/settings/style.h"

#include <QFont>

class SimpleText : public ChatLineContent {
    Q_OBJECT

public:
    SimpleText(const QString& txt = "", const QFont& font = QFont());
    ~SimpleText() {}

    void setText(const QString& txt);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setWidth(qreal width) override;

    void setColor(Style::ColorPalette role);
    void setColor(const QColor& color);

protected:
    virtual void onCopyEvent() override;

private:
    void updateBoundingRect();
    void reloadTheme() override;

    QString text;
    QSizeF boundSize;
    QFont defFont;
    QColor color;
    Style::ColorPalette colorRole = Style::MainText;
    bool customColor = false;
    qreal forceWidth = -1;
};

#endif  // !SIMPLETEXT_H
