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
// Created by gaojie on 25-1-14.
//

#include "Widget.h"
#include <QDebug>
#include <QTabWidget>
#include <QVBoxLayout>

#include "lib/storage/settings/style.h"
#include "lib/ui/web/WebWidget.h"

namespace module::doc {

Widget::Widget(QWidget* parent) : lib::ui::OFrame(parent) {
    setContentsMargins(0, 0, 0, 0);
    // setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    // 创建布局
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    tab = new QTabWidget(this);
    tab->setObjectName("mainTab");

    tab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    tab->addTab(new lib::ui::WebWidget(QUrl("http://localhost/example/"), this), tr("Office"));
    tab->addTab(new lib::ui::WebWidget(QUrl("http://localhost:9001/"), this), tr("Etherpad"));
    layout->addWidget(tab);

    setLayout(layout);

    reloadTheme();
}

Widget::~Widget() {}

void Widget::reloadTheme() {
    auto style = lib::settings::Style::getStylesheet("general.css");

    qDebug() << style;
    setStyleSheet(style);
}

}  // namespace module::doc
