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
#include "OTitleBar.h"
#include "ui_OTitleBar.h"

OTitleBar::OTitleBar(QWidget* parent) : QWidget(parent), ui(new Ui::OTitleBar) {
    ui->setupUi(this);
}

OTitleBar::~OTitleBar() {
    delete ui;
}

void OTitleBar::setTitle(const QString& title) {
    ui->label_title->setText(title);
}

QWidget* OTitleBar::getTitle() {
    return ui->label_title;
}
