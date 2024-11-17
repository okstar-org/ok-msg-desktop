#include "PopupMenuComboBox.h"

#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QHBoxLayout>
#include <QPainter>
#include <QLabel>

class ToolButtonRemoveSpace : public QToolButton {
public:
    using QToolButton::QToolButton;
    QSize sizeHint() const {
        QSize size = QToolButton::sizeHint();
        if (this->toolButtonStyle() == Qt::ToolButtonTextBesideIcon) {
            size.rwidth() -= fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 2;
        }
        return size;
    }
};

class LayoutBarSeparator : public QWidget {
public:
    LayoutBarSeparator(Qt::Orientation orientation, QWidget* parent)
            : QWidget(parent), orient(orientation) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    }
    QSize sizeHint() const override {
        QStyleOption opt;
        initStyleOption(&opt);
        const int extent =
                style()->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, &opt, parentWidget());
        return QSize(extent, extent);
    }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        QStyleOption opt;
        initStyleOption(&opt);
        style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, &p, parentWidget());
    }
    void initStyleOption(QStyleOption* option) const {
        option->initFrom(this);
        // 与实际相反
        if (orient == Qt::Vertical) {
            option->state |= QStyle::State_Horizontal;
        }
    }

private:
    Qt::Orientation orient;
};

PopupMenuComboBox::PopupMenuComboBox(QWidget* parent) : QFrame(parent) {
    setAttribute(Qt::WA_StyledBackground);
    mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(2, 2, 2, 2);

    _iconButton = new ToolButtonRemoveSpace(this);

    menuButton = new QToolButton(this);
    menuButton->setObjectName("menuButton");
    menuButton->setIconSize(QSize(12, 12));
    menuButton->setIcon(QIcon(":/meet/image/up_arrow.svg"));

    mainLayout->addWidget(_iconButton);
    mainLayout->addWidget(new LayoutBarSeparator(Qt::Vertical, this));
    mainLayout->addWidget(menuButton);
}

void PopupMenuComboBox::setLabel(const QString& text) { setWidget(new QLabel(text, this)); }

void PopupMenuComboBox::setWidget(QWidget* widget) {
    if (content.isNull()) {
        if (widget) {
            mainLayout->insertWidget(1, widget, 1);
        }
    } else {
        if (widget) {
            mainLayout->replaceWidget(content, widget);
        } else {
            content->deleteLater();
        }
    }
    content = widget;
}

QToolButton* PopupMenuComboBox::iconButton() { return _iconButton; }
