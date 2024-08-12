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

#pragma once

#include "UI/widget/OWidget.h"

#include <QPointer>
#include <QJsonArray>

class QWebEngineView;
class QWebChannel;
class QThread;
class QWebSocketServer;
class WebSocketClientWrapper;
class WebSocketTransport;

namespace ok::platform {
class AppCenterWidget : public UI::OWidget {
    Q_OBJECT
public:
    AppCenterWidget(QWidget* parent = nullptr);
    void start();

private:
    std::unique_ptr<QThread> thread;

    QWebEngineView* webView;
    QWebChannel* webChannel;
    QWebSocketServer* wss;
    WebSocketClientWrapper* clientWrapper;

    void startWsServer();
    void startWebEngine();

    void requestAppList();

public slots:
    void clientConnected(WebSocketTransport* transport);

private:
    void sendAppListToView(const QJsonArray& appList);

private:
    QPointer<WebSocketTransport> wsTransport;
    QJsonArray cachedAppList;
    bool hasRequested = false;
};
}  // namespace ok::platform
