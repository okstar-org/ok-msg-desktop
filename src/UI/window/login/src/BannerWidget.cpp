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

#include "BannerWidget.h"
#include <QWidget>

#include <QPainter>
#include <QSvgRenderer>

#include <base/logs.h>
#include <base/widgets.h>
#include "ui_BannerWidget.h"

namespace UI {

BannerWidget::BannerWidget(QWidget* parent)
        : QWidget(parent), ui(std::make_unique<Ui::BannerWidget>()) {
    ui->setupUi(this);

    ui->imgBox->setGeometry(rect());
    ui->imgBox->setFixedSize(256, 256);
    ui->imgBox->setStyleSheet("border-image: url(:/resources/logo/main.svg);");
}

BannerWidget::~BannerWidget() {}

}  // namespace UI
