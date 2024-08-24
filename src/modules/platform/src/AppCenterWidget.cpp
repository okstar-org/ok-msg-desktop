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
#include "LoadingWidget.h"
#include "Platform.h"
#include "application.h"
#include "lib/settings/style.h"
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

    loadingWidget = new LoadingWidget(this);

    QColor color = Style::getExtColor("view.loadingAnima.color");
    if (color.isValid()) {
        QPalette pal = loadingWidget->palette();
        pal.setColor(QPalette::Normal, QPalette::WindowText, color);
        loadingWidget->setPalette(pal);
    }
}

void AppCenterWidget::requestAppList() {
    hasRequested = true;
    auto session = ok::Application::Instance()->getSession();
    auto backend = new Backend(session->getStackUrl(), session->getToken().getAuthorization());
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
        loadingWidget->hide();
        loadingWidget->deleteLater();
        loadingWidget = nullptr;
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

    QTimer::singleShot(200, this, [this]() {
        if (loadingWidget) {
            loadingWidget->setVisible(true);
        }
    });
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
    pageContainer->openWebPage(url, "d8e4dc6b-5f05-11ef-b07d-0242ac1a0004", title);
}

void AppCenterPage::onWebMessageReceived(const QJsonValue& value) {
    QJsonObject object = value.toObject();
    QString command = object.value("command").toString();
    if (command == "app-center.openApp") {
        QUrl url = QUrl::fromEncoded(object.value("homePage").toString().toUtf8());
        QString name = object.value("name").toString();
        QString uuid = object.value("uuid").toString();
        if (!name.isEmpty()) {
            pageContainer->openWebPage(url, uuid, name);
        }
    }
}

}  // namespace ok::platform
