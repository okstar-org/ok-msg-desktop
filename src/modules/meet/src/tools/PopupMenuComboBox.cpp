#include "PopupMenuComboBox.h"

#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QHBoxLayout>
#include <QPainter>
#include <QLabel>
#include <QMenu>

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
    menuButton->setIcon(QIcon(":/meet/image/arrow_up.svg"));

    mainLayout->addWidget(_iconButton);
    mainLayout->addWidget(new LayoutBarSeparator(Qt::Vertical, this));
    mainLayout->addWidget(menuButton);

    connect(menuButton, &QToolButton::clicked, this, &PopupMenuComboBox::onMenuButtonClicked);
}

void PopupMenuComboBox::setLabel(const QString& text) {
    if (QLabel* label = qobject_cast<QLabel*>(content.data())) {
        label->setText(text);
        return;
    }

    setWidget(new QLabel(text, this));
}

void PopupMenuComboBox::setWidget(QWidget* widget) {
    if (content.isNull()) {
        if (widget) {
            mainLayout->insertWidget(1, widget, 1);
        }
    } else {
        if (widget) {
            mainLayout->replaceWidget(content, widget);
        }
        content->deleteLater();
    }
    content = widget;
}

QToolButton* PopupMenuComboBox::iconButton() { return _iconButton; }

void PopupMenuComboBox::setMenu(QMenu* menu) {
    popMenu = menu;
}

void PopupMenuComboBox::showMenuOnce(QMenu* menu)
{
    QSize size = menu->sizeHint();
    QPoint pos = this->mapToGlobal(QPoint(0, -5));
    pos.ry() -= size.height();
    menu->exec(pos);

    if (!rect().contains(mapFromGlobal(QCursor::pos())))
    {
        menuButton->setAttribute(Qt::WA_UnderMouse, false);
        menuButton->update();
    }
}

void PopupMenuComboBox::onMenuButtonClicked() {
    if (popMenu){
        showMenuOnce(popMenu.data());
        return;
    }
    emit menuRequest();
}
