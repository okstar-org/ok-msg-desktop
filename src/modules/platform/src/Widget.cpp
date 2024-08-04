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

//
// Created by gaojie on 24-7-31.
//

#include "Widget.h"
#include "ui_Widget.h"

#include "AppCenterWidget.h"

namespace ok::platform {

Widget::Widget(QWidget* parent) : UI::OMenuWidget(parent), ui(new Ui::WorkPlatform) {
    OK_RESOURCE_INIT(Platform);
    ui->setupUi(this);

    ui->tabWidget->setObjectName("mainTab");

    centerWidget = new AppCenterWidget(this);
    ui->tabWidget->addTab(centerWidget, tr("App center"));

    //    thread = (std::make_unique<QThread>());
    //    thread->setObjectName("WorkPlatform");
    //    connect(thread.get(), &QThread::started, this, &Widget::doStart);
    //    moveToThread(thread.get());
    //
}

Widget::~Widget() {}

void Widget::start() { centerWidget->start(); }

void Widget::doStart() {}

}  // namespace ok::platform
