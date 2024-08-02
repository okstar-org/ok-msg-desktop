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

#include <QFile>
#include <QGridLayout>
#include <QUrl>
#include <QtWebEngineWidgets/QWebEngineView>

namespace platform {

Widget::Widget(QWidget* parent) : UI::OMenuWidget(parent) {
    OK_RESOURCE_INIT(Platform);

    setLayout(new QGridLayout());

    QString htmlContent;
    QFile file(":/res/Platform/app.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        htmlContent = in.readAll();
        file.close();
    }

    webView = new QWebEngineView(this);
    webView->setContent(htmlContent.toUtf8(), "text/html", QUrl("file:///"));
    layout()->addWidget(webView);

}
Widget::~Widget() {

}

}  // namespace platform
