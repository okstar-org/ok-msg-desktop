#ifndef SIMPLETEXT_H
#define SIMPLETEXT_H

#include "../chatlinecontent.h"
#include "src/widget/style.h"

#include <QFont>

class SimpleText : public ChatLineContent {
    Q_OBJECT

public:
    SimpleText(const QString &txt = "", const QFont &font = QFont(),
               const QColor &custom = Style::getColor(Style::MainText));
    ~SimpleText() {
    }

    void setText(const QString &txt);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void setWidth(qreal width) override;

private:
    void updateBoundingRect();

private:
    QString text;
    QSizeF boundSize;
    QFont defFont;
    QColor color;
    qreal forceWidth = -1;
};

#endif // !SIMPLETEXT_H

