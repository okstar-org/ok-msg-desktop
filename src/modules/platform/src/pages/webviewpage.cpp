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

#include "webviewpage.h"
#include <QMessageBox>
#include <QWebEngineView>
#include "application.h"
#include "src/Backend.h"

ok::platform::WebviewPage::WebviewPage(const QUrl& url,
                                       const QString& uuid,
                                       const QString& title,
                                       PlatformPageContainer* container)
        : PlatformPage(container), pageUrl(url), appUuid(uuid), pageTitle(title) {
    webView = std::make_unique<QWebEngineView>();
}

ok::platform::WebviewPage::~WebviewPage() {}

QWidget* ok::platform::WebviewPage::getWidget() { return webView.get(); }

void ok::platform::WebviewPage::createContent(QWidget* parent) {
    auto session = ok::Application::Instance()->getSession();
    auto backend = new Backend(session->getStackUrl(), session->getToken().getAuthorization());
    backend->getInstance(
            [this, backend, session](QJsonDocument body) {
                auto obj = body.object();
                auto port = obj.value("data").toObject().value("ports").toArray()[0].toString();
                if (port.isEmpty()) return;
                QUrl baseUrl(session->getStackUrl());
                QString url = "http://" + baseUrl.host() + ":" + port;
                qDebug() << "Instance service port:" << url;
                webView->load(url);
                backend->deleteLater();
            },
            appUuid,
            [this, backend](int statusCode, const QByteArray& body) {
                auto json = base::Jsons::toJSON(body);
                auto msg = json.object().value("msg").toString();
                QMessageBox::warning(nullptr, "Warning", msg);
                backend->deleteLater();
                webView->load(pageUrl);
            });
}

void ok::platform::WebviewPage::start() {}

void ok::platform::WebviewPage::doClose() {}
