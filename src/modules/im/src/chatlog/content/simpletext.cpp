#include "simpletext.h"
#include <QPainter>

SimpleText::SimpleText(const QString &txt, const QFont &font,
                       const QColor &custom)
    : text(txt), defFont(font) {
    updateBoundingRect();
}

void SimpleText::setText(const QString &txt) {
    if (text != txt) {
        text = txt;
        updateBoundingRect();
    }
}

QRectF SimpleText::boundingRect() const {
    return QRectF(QPointF(0, 0), boundSize);
}

void SimpleText::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) {
    if (!text.isEmpty()) {
        painter->setFont(defFont);
        int y_offset = QFontMetricsF(defFont).leading() / 2.0;
        painter->drawText(QRectF(QPointF(0, y_offset), boundSize),
                          Qt::AlignCenter, text);
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

void SimpleText::updateBoundingRect() {
    QFontMetricsF fm(defFont);
    if (!text.isEmpty())
        boundSize = fm.boundingRect(text).size();
    else
        boundSize = QSizeF(fm.height(), fm.height());
    if (forceWidth > 0)
        boundSize.rwidth() = forceWidth;
}
