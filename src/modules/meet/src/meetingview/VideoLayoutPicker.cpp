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

#include "VideoLayoutPicker.h"
#include "base/shadowbackground.h"

#include <QButtonGroup>
#include <QEvent>
#include <QEventLoop>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QTimer>
namespace module::meet {

class LayoutItemButton : public QPushButton {
public:
    using QPushButton::QPushButton;
    QSize sizeHint() const;
    QSize minimumSizeHint() const {
        return sizeHint();
    }

protected:
    void paintEvent(QPaintEvent* event);
    void changeEvent(QEvent* e) {
        QWidget::changeEvent(e);
        if (e->type() != QEvent::EnabledChange) _sh = QSize();
    }

private:
    mutable QSize _sh;
    QIcon layoutIcon;
    friend class VideoLayoutPicker;
};

VideoLayoutPicker::VideoLayoutPicker(QWidget* parent)
        : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint) {
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(10, 10, 10, 10);
    (new ok::base::ShadowBackground(this))->setShadowRadius(10);

    //    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    typeGroup = new QButtonGroup(this);
    typeGroup->setExclusive(true);

    QPushButton* button = appendItem(tr("Grid View"), ":/meet/image/layout_gridview.svg");
    typeGroup->addButton(button, module::meet::GridView);
    button->setChecked(true);
    button = appendItem(tr("List on Top"), ":/meet/image/layout_toplist.svg");
    typeGroup->addButton(button, module::meet::TopList);
    button = appendItem(tr("List on Right"), ":/meet/image/layout_rightlist.svg");
    typeGroup->addButton(button, module::meet::RightList);

    connect(typeGroup, &QButtonGroup::idToggled, this, &VideoLayoutPicker::onTypeChanged);
}

void VideoLayoutPicker::setCurrentType(module::meet::VideoLayoutType type) {
    typeGroup->blockSignals(true);
    typeGroup->button(type)->setChecked(true);
    typeGroup->blockSignals(false);
}

module::meet::VideoLayoutType VideoLayoutPicker::selectedType() const {
    // id is type
    return static_cast<module::meet::VideoLayoutType>(typeGroup->checkedId());
}

void VideoLayoutPicker::exec(const QPoint& pos) {
    QEventLoop loop;
    eventLoop = &loop;
    this->adjustSize();
    this->move(pos);
    this->show();
    eventLoop->exec();
    this->close();
}

void VideoLayoutPicker::onTypeChanged(int id, bool checked) {
    if (!checked) {
        return;
    }
    if (eventLoop) {
        QTimer::singleShot(0, eventLoop, &QEventLoop::quit);
    }
}

void VideoLayoutPicker::closeEvent(QCloseEvent* e) {
    if (eventLoop) {
        eventLoop->quit();
    }
}

QPushButton* VideoLayoutPicker::appendItem(const QString& text, const QString& svgPath) {
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    LayoutItemButton* itemButton = new LayoutItemButton(this);
    itemButton->setObjectName("layoutTypeIcon");
    itemButton->layoutIcon = QIcon(svgPath);
    itemButton->setCheckable(true);

    QLabel* label = new QLabel(text, this);
    label->setObjectName("layoutName");
    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(itemButton);
    layout->setAlignment(itemButton, Qt::AlignHCenter);
    layout->addWidget(label);
    layout->addStretch(1);
    dynamic_cast<QHBoxLayout*>(this->layout())->addLayout(layout);
    return itemButton;
}

inline QSize LayoutItemButton::sizeHint() const {
    if (_sh.isValid()) {
        return _sh;
    }
    // the svg icon is 60*45
    QSize base(60, 45);
    QStyleOptionButton option;
    initStyleOption(&option);
    _sh = style()->sizeFromContents(QStyle::CT_PushButton, &option, base, this);
    return _sh;
}

void LayoutItemButton::paintEvent(QPaintEvent* event) {
    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    p.drawControl(QStyle::CE_PushButtonBevel, option);

    if (!layoutIcon.isNull()) {
        QRect cr = p.style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
        QPixmap px = layoutIcon.pixmap(cr.size());
        px.save("d:/test2.png", "png");
        px.setDevicePixelRatio(this->devicePixelRatioF());
        p.drawPixmap(cr.topLeft(), px);
    }
}
}  // namespace module::meet
