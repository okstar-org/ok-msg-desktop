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

#include <lib/backend/BaseService.h>
#include <QFile>
#include <QGridLayout>
#include <QUrl>
#include <QWebChannel>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QtWebEngineWidgets/QWebEngineView>

#include "AccessToken.h"
#include "Backend.h"
#include "application.h"
#include "websocketclientwrapper.h"
#include "websockettransport.h"

namespace ok::platform {

Widget::Widget(QWidget* parent) : UI::OMenuWidget(parent) {
    OK_RESOURCE_INIT(Platform);
    setLayout(new QGridLayout());

    //    thread = (std::make_unique<QThread>());
    //    thread->setObjectName("WorkPlatform");
    //    connect(thread.get(), &QThread::started, this, &Widget::doStart);
    //    moveToThread(thread.get());
    //
}

Widget::~Widget() {}

void Widget::start() { doStart(); }

void Widget::doStart() {
    startWebEngine();
    startWsServer();
}

void Widget::startWebEngine() {
    QString htmlContent;
    QFile file(":/res/Platform/platform.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        htmlContent = in.readAll();
        file.close();
    }

    webView = new QWebEngineView(this);
    webView->setContent(htmlContent.toUtf8(), "text/html", QUrl("file:///"));

    //    auto page = webView->page();
    //    webChannel = new QWebChannel(page);
    //    webChannel->registerObject(QStringLiteral("accessToken"), &accessToken);
    //    page->setWebChannel(webChannel);

    layout()->addWidget(webView);
}

void Widget::startWsServer() {
    wss = new QWebSocketServer(QStringLiteral("QWebChannel Standalone Example Server"),
                               QWebSocketServer::NonSecureMode);

    if (!wss->listen(QHostAddress::LocalHost, 65500)) {
        qFatal("Failed to open web socket server.");
        return;
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    clientWrapper = new WebSocketClientWrapper(wss);

    // setup the channel
    connect(clientWrapper,
            &WebSocketClientWrapper::clientConnected,
            webChannel,
            &QWebChannel::connectTo);
    connect(clientWrapper,
            &WebSocketClientWrapper::clientConnected,
            this,
            &Widget::clientConnected);
}

void Widget::clientConnected(WebSocketTransport* transport) {
    auto session = ok::Application::Instance()->getSession();
    auto token = session->getToken();

    auto backend = new Backend(session->getSignInInfo().stackUrl);
    backend->setHeader("Authorization", token.tokenType + " " + token.accessToken);
    backend->getAppList([=](QByteArray body, QString name) {
        auto arr = Jsons::toJSON(body).object().value("data").toObject().value("list").toArray();
        for (auto app : arr) {
            auto a = app.toObject();
            //           qDebug() <<QString::fromUtf8( QJsonDocument(a).toJson());
            transport->sendMessage(a);
        }
    });
}

}  // namespace ok::platform
