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

#include <QWebEngineView>
#include "application.h"
#include "src/Backend.h"
#include "src/base/MessageBox.h"

namespace module::platform {

WebviewPage::WebviewPage(const QUrl& url,
                         const QString& type,
                         const QString& uuid,
                         const QString& title,
                         PlatformPageContainer* container)
        : PlatformPage(container), pageUrl(url), appType(type), appUuid(uuid), pageTitle(title) {
    webView = std::make_unique<QWebEngineView>();
}

WebviewPage::~WebviewPage() = default;

QWidget* WebviewPage::getWidget() {
    return webView.get();
}

void WebviewPage::createContent(QWidget* parent) {
    if (appType == "Open") {
        // 如果是开放App，直接打开链接。
        qDebug() << "Open:" << pageUrl;
        webView->load(QUrl(pageUrl));
        return;
    }

    auto session = ok::Application::Instance()->getSession();
    auto backend = new Backend(session->getStackUrl(), session->getToken().getAuthorization());
    backend->getInstance(
            [this, backend, session](const QJsonDocument& body) {
                auto obj = body.object();
                const QJsonObject& instance = obj.value("data").toObject();

                // first port as main service port.
                auto port = instance.value("ports").toArray()[0].toString();
                if (port.isEmpty()) return;

                QUrl baseUrl(session->getStackUrl());
                QString url = "https://" + baseUrl.host() + ":" + port;
                qDebug() << "Instance service url:" << url;
                webView->load(url);
                backend->deleteLater();
            },

            appUuid,
            [this, backend](int statusCode, const QByteArray& body) {
                auto json = ok::base::Jsons::toJSON(body);
                auto msg = json.object().value("msg").toString();
                ok::base::MessageBox::warning(nullptr, "Warning", msg);
                backend->deleteLater();
                webView->load(pageUrl);
            });
}

void WebviewPage::start() {}

void WebviewPage::doClose() {}

}  // namespace module::platform