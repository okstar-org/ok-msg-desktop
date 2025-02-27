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

#include "simpletext.h"
#include <QPainter>

namespace module::im {

SimpleText::SimpleText(const QString& txt, const QFont& font)
        : ChatLineContent(ContentType::CHAT_TEXT), text(txt), defFont(font) {
    color = lib::settings::Style::getColor(colorRole);
    updateBoundingRect();
}

void SimpleText::setText(const QString& txt) {
    if (text != txt) {
        text = txt;
        updateBoundingRect();
    }
}

QRectF SimpleText::boundingRect() const { return QRectF(QPointF(0, 0), boundSize); }

void SimpleText::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    if (!text.isEmpty()) {
        painter->setFont(defFont);
        painter->setPen(QPen(color));
        int y_offset = QFontMetricsF(defFont).leading() / 2.0;
        painter->drawText(QRectF(QPointF(0, y_offset), boundSize), Qt::AlignCenter, text);
    }
}

void SimpleText::setWidth(qreal width) {
    if (width <= 0) {
        forceWidth = -1;
        updateBoundingRect();
    } else {
        forceWidth = width;
        boundSize.rwidth() = width;
    }
}

void SimpleText::setColor(lib::settings::Style::ColorPalette role) {
    customColor = false;
    colorRole = role;
    color = lib::settings::Style::getColor(colorRole);
}

void SimpleText::setColor(const QColor& color) {
    customColor = true;
    this->color = color;
}

void SimpleText::onCopyEvent() {}

void SimpleText::updateBoundingRect() {
    QFontMetricsF fm(defFont);
    if (!text.isEmpty())
        boundSize = fm.boundingRect(text).size();
    else
        boundSize = QSizeF(fm.height(), fm.height());
    if (forceWidth > 0) boundSize.rwidth() = forceWidth;
}

void SimpleText::reloadTheme() {
    if (!customColor) color = lib::settings::Style::getColor(colorRole);
}
const void* SimpleText::getContent() { return &text; }
}  // namespace module::im