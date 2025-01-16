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
#include "OWidget.h"

#include <QFrame>
#include <QPainter>
#include <QPalette>
#include <QRgb>
#include <QWidget>

namespace UI {

OWidget::OWidget(QWidget* parent) : QWidget(parent) {
    //   QPalette palette = this->palette();

    //   QBrush brush;
    //   brush.setColor(QColor(83,83,83, 20));
    //   palette.setBrush(QPalette::Background, brush);

    //   setPalette(palette);
    //   setAutoFillBackground(true);

    //    setAttribute(Qt::WA_TranslucentBackground); //设置窗口透明

    //   QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    //   effect->setOffset(4,4);
    //   effect->setColor(QColor(0,0,0,50));
    //   effect->setBlurRadius(10);

    //   setGraphicsEffect(effect);
}

void OWidget::paintEvent(QPaintEvent* event) {}
OWidget::~OWidget() {}

}  // namespace UI
