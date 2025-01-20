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
// Created by gaojie on 25-1-20.
//

#include "WebWidget.h"

#include <QVBoxLayout>
#include <QWebEngineView>

namespace lib::ui {

WebWidget::WebWidget(const QUrl& url, QWidget* parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* webView = new QWebEngineView(this);
    // 加载网页
    webView->load(url);
    // 连接信号
    connect(webView, &QWebEngineView::loadFinished, this, &WebWidget::onLoadFinished);

    layout->addWidget(webView);
}

WebWidget::~WebWidget() {}

void WebWidget::onLoadFinished(bool ok) {
    qDebug() << __func__ << ok;
}

}  // namespace lib::ui
