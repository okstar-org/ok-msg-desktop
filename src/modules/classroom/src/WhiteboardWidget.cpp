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
#include "WhiteboardWidget.h"

#include <base/logs.h>
#include <base/utils.h>
#include <QImage>
#include <QLabel>
#include <QPalette>
#include <QPushButton>


#include "src/painter/PainterView.h"

namespace module::classroom {

WhiteboardWidget::WhiteboardWidget(QWidget* parent) : QWidget(parent) {
    qDebug() << __func__;

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    _painter = new PainterView(this);

    layout->addWidget(_painter);
    setLayout(layout);
}

WhiteboardWidget::~WhiteboardWidget() {
    qDebug() << __func__;
}

}  // namespace module::classroom
