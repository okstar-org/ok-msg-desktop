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
// Created by gaojie on 24-8-4.
//

#include "AppCenterWidget.h"
#include <QUrl>
#include <QWebChannel>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QtWebEngineWidgets/QWebEngineView>

#include <QPushButton>
#include "Backend.h"
#include "Platform.h"
#include "application.h"
#include "websocketclientwrapper.h"
#include "websockettransport.h"

namespace ok::platform {

AppCenterWidget::AppCenterWidget(AppCenterPage* page, QWidget* parent)
        : UI::OWidget(parent), platformPage(page) {
    setLayout(new QGridLayout);
    layout()->setContentsMargins(0, 0, 0, 0);
}

void AppCenterWidget::startWebEngine() {
    QByteArray htmlContent;
    QFile file(":/res/Platform/platform.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        htmlContent = file.readAll();
        file.close();
    }

    webView = new QWebEngineView(this);
    webView->setContent(htmlContent, "text/html", QUrl("file:///"));

    //    auto page = webView->page();
    //    webChannel = new QWebChannel(page);
    //    webChannel->registerObject(QStringLiteral("accessToken"), &accessToken);
    //    page->setWebChannel(webChannel);

    layout()->addWidget(webView);
}

void AppCenterWidget::requestAppList() {
    hasRequested = true;
    auto session = ok::Application::Instance()->getSession();
    auto token = session->getToken();
    auto backend = new Backend(session->getSignInInfo().stackUrl);
    backend->setHeader("Authorization", token.tokenType + " " + token.accessToken);
    backend->getAppList([this, backend](QByteArray body, QString name) {
        auto arr = ok::base::Jsons::toJSON(body)
                           .object()
                           .value("data")
                           .toObject()
                           .value("list")
                           .toArray();
        sendAppListToView(arr);
        hasRequested = false;
        delete backend;
    });
}

void AppCenterWidget::sendAppListToView(const QJsonArray& appList) {
    if (this->wsTransport) {
        for (auto app : appList) {
            auto a = app.toObject();
            wsTransport->sendMessage(a);
        }
    } else {
        cachedAppList = appList;
    }
}

void AppCenterWidget::startWsServer() {
    wss = new QWebSocketServer(QStringLiteral("OkStar Websocket Server"),
                               QWebSocketServer::NonSecureMode);

    if (!wss->listen(QHostAddress::LocalHost, 65500)) {
        qWarning() << "Failed to open web socket server.";
        return;
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    clientWrapper = new WebSocketClientWrapper(wss);

    // setup the channel
    //    connect(clientWrapper,
    //            &WebSocketClientWrapper::clientConnected,
    //            webChannel,
    //            &QWebChannel::connectTo);
    connect(clientWrapper,
            &WebSocketClientWrapper::clientConnected,
            this,
            &AppCenterWidget::clientConnected);
}

void AppCenterWidget::clientConnected(WebSocketTransport* transport) {
    this->wsTransport = transport;
    if (!cachedAppList.isEmpty()) {
        sendAppListToView(cachedAppList);
        cachedAppList = QJsonArray();
    } else if (!hasRequested) {
        // 第二次加载，是否应该通过某种方式，并发处理
        requestAppList();
    }
    connect(transport,
            &WebSocketTransport::messageReceived,
            platformPage,
            &AppCenterPage::onWebMessageReceived);
}
void AppCenterWidget::start() {
    startWsServer();
    requestAppList();
    startWebEngine();
}

void AppCenterPage::createContent(QWidget* parent) {
    if (!widget) {
        widget = new AppCenterWidget(this, parent);
        connect(widget, &AppCenterWidget::appPageRequest, this, &AppCenterPage::openAppPage);
    }
}

QWidget* AppCenterPage::getWidget() { return widget.data(); }

// 指定一个唯一URL
QUrl AppCenterPage::getUrl() { return QUrl("app-center://main-page"); }

void AppCenterPage::start() {
    if (widget) {
        widget->start();
    }
}

AppCenterPage::~AppCenterPage() {
    if (widget) {
        widget->deleteLater();
        widget = nullptr;
    }
}

QString AppCenterPage::getTitle() { return AppCenterWidget::tr("App center"); }

void AppCenterPage::doClose() {}

bool AppCenterPage::pageClosable() { return false; }

// 通过PlatformContainer接口打开web链接
void AppCenterPage::openAppPage(const QUrl& url, const QString& title) {
    pageContainer->openWebPage(url, title);
}

void AppCenterPage::onWebMessageReceived(const QJsonValue& value) {
    QJsonObject object = value.toObject();
    QString command = object.value("command").toString();
    if (command == "app-center.openApp")
    {
        QUrl url = QUrl::fromEncoded(object.value("mainPage").toString().toUtf8());
        QString name = object.value("name").toString();
        if (!name.isEmpty())
        {
            pageContainer->openWebPage(url, name);
        }
    }
}

}  // namespace ok::platform
