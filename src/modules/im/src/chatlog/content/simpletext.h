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

private:
    void updateBoundingRect();
    void reloadTheme() override;

private:
    QString text;
    QSizeF boundSize;
    QFont defFont;
    QColor color;
    Style::ColorPalette colorRole = Style::MainText;
    bool customColor = false;
    qreal forceWidth = -1;
};

#endif  // !SIMPLETEXT_H
