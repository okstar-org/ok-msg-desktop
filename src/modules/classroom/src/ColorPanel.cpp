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

#include "ColorPanel.h"

#include <memory>

#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QObject>
#include <QPainter>
#include <QVBoxLayout>

#include "src/base/logs.h"
#include "src/base/utils.h"

namespace module::classroom {

// 大小
ToolboxPanelSize::ToolboxPanelSize(QWidget* parent) : QLabel(parent) {}

ToolboxPanelSize::~ToolboxPanelSize() {}

void ToolboxPanelSize::mousePressEvent(QMouseEvent* event) {
    emit checked(this);
}

// 大小
ToolboxPanelPoint::ToolboxPanelPoint(QWidget* parent) : QWidget(parent) {}

ToolboxPanelPoint::~ToolboxPanelPoint() {}

void ToolboxPanelPoint::mousePressEvent(QMouseEvent* event) {
    emit checked(this);
}

// 颜色
ToolboxPanelColor::ToolboxPanelColor(QWidget* parent) : QWidget(parent) {}

ToolboxPanelColor::~ToolboxPanelColor() {}

void ToolboxPanelColor::mousePressEvent(QMouseEvent* event) {
    emit checked(this);
}

#define COLOR_ROW 3
#define COLOR_COL 4

// 色板
static const QString COLOR_LIST[COLOR_ROW][COLOR_COL] = {
        {"#000000", "#c0c0c0", "#7f7f7f", "#ffffff"},
        {"#eb4328", "#f77702", "#f4cd02", "#9ec55a"},
        {"#5bd4cf", "#2f8fdf", "#6f5d98", "#df8a87"}};

// static const int PEN_SIZE_LIST[4] = {4, 6, 8, 10};
static const int WEIGHT_LIST[4] = {4, 6, 8, 10};
// 2*1 2*2 2*3 2*4
// 背景图片 8 12 16 20
// 10 + 1 * 4 10 + 2 * 4 10 + 3 * 4
// static const int TEXT_SIZE_LIST[4] = { 14, 18, 24, 36 };

// ColorPanel::ColorPanel(QWidget* parent, int defaultSize,
//                        QString defaultColor /*, ToolboxType type*/)
//         : QFrame(parent) {
//     setFixedSize(QSize(TOOLBOX_PANEL_WIDTH, TOOLBOX_PANEL_HEIGHT));
//
//     QWidget* wrap = new QWidget(this);
//     wrap->setFixedSize(QSize(TOOLBOX_PANEL_WIDTH - 6, TOOLBOX_PANEL_HEIGHT));
//     wrap->setStyleSheet("background-color:#535353; border-radius: 4px;");
//
//     QVBoxLayout* vBox = new QVBoxLayout(this);
//     wrap->setLayout(vBox);
//
//     _widthLayout = new QHBoxLayout(wrap);
//
//     // 字体
//     for (int i = 0; i < ARRAY_LENGTH_OF(WEIGHT_LIST); i++) {
//         ToolboxPanelSize* p = new ToolboxPanelSize(wrap);
//         //        p->setSize(WEIGHT_LIST[i]);
//
//         QObject::connect(p, SIGNAL(checked(ToolboxPanelSize*)), this,
//                          SLOT(checkSize(ToolboxPanelSize*)));
//
//         _widthLayout->addWidget(p);
//         if (WEIGHT_LIST[i] == defaultSize) checkSize(p);
//     }
//
//     // 大小
//     vBox->addLayout(_widthLayout);
//
//     // 中间线
//     QFrame* line = new QFrame(wrap);
//     line->setFixedSize(TOOLBOX_PANEL_WIDTH * 0.8, 4);
//     line->setStyleSheet(qsl("border-top: 2px solid #383838; border-bottom: 2px solid #565656;"));
//     line->show();
//     vBox->addWidget(line);
//
//     // 颜色
//     _colorLayout = std::make_shared<QGridLayout>(wrap);
//     for (int i = 0; i < COLOR_ROW; i++) {
//         for (int j = 0; j < COLOR_COL; j++) {
//             ToolboxPanelColor* c = new ToolboxPanelColor(wrap);
//             c->setSize(20);
//             c->setColor(COLOR_LIST[i][j]);
//             QObject::connect(c, SIGNAL(checked(ToolboxPanelColor*)), this,
//                              SLOT(checkColor(ToolboxPanelColor*)));
//             _colorLayout->addWidget(c, i, j);
//
//             if (QString::compare(COLOR_LIST[i][j], defaultColor, Qt::CaseInsensitive) == 0) {
//                 checkColor(c);
//             }
//         }
//     }
//
//     vBox->addLayout(_colorLayout.get());
//
//     installEventFilter(this);
// }

// void ColorPanel::checkSize(ToolboxPanelSize* point) {
//     _video_size = point->value();
//     for (int i = 0; i < _widthLayout->count(); i++) {
//         ToolboxPanelSize* p =
//                 dynamic_cast<ToolboxPanelSize*>(_widthLayout.get()->itemAt(i)->widget());
//         if (p) p->setChecked(false);
//     }
//     point->setChecked(true);
//
//     emit checked();
// }

// void ColorPanel::checkSize(ToolboxPanelPoint* point) {
//     _video_size = point->value();
//     for (int i = 0; i < _widthLayout->count(); i++) {
//         ToolboxPanelPoint* p =
//         static_cast<ToolboxPanelPoint*>(_widthLayout->itemAt(i)->widget()); p->setChecked(false);
//     }
//     point->setChecked(true);
//     emit checked();
// }

void ColorPanel::checkColor(ToolboxPanelColor* color) {
    //    _color = color->color();
    //    for (int i = 0; i < _colorLayout->count(); i++) {
    //        ToolboxPanelColor* c =
    //        static_cast<ToolboxPanelColor*>(_colorLayout->itemAt(i)->widget());
    //        c->setChecked(false);
    //    }
    //    color->setChecked(true);
    //
    //    emit checked();
}

bool ColorPanel::eventFilter(QObject* target, QEvent* event) {
    if (event->type() == QEvent::Type::Show) {
        _hovered = true;
    }
    if (event->type() == QEvent::Type::Leave) {
        _hovered = false;
        hide(true);
    }

    return QFrame::eventFilter(target, event);
}

void ColorPanel::hide(bool force) {
    if (!force && _hovered) {
        return;
    }
    QFrame::hide();
}
void ColorPanel::checkSize(ToolboxPanelColor*) {}

ColorPanel::~ColorPanel() {}

}  // namespace module::classroom
