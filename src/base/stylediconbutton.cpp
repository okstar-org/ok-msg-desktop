#include "stylediconbutton.h"

#include <QStyleOptionButton>
#include <QStylePainter>
namespace ok::base {
void StyledIconButton::paintEvent(QPaintEvent* event) {
    if (icon_use_indicator) {
        QStylePainter p(this);
        QStyleOptionButton option;
        initStyleOption(&option);
        p.drawControl(QStyle::CE_PushButtonBevel, option);

        QRect r = p.style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
        option.rect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, iconSize(), r);
        p.drawPrimitive(QStyle::PE_IndicatorCheckBox, option);
    }
    QPushButton::paintEvent(event);
}
}
