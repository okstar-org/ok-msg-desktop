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

#pragma once

#include <QWidget>
#include "UI/widget/OMenuWidget.h"
#include "base/resources.h"

class QWebEngineView;
class QWebChannel;
class QThread;
class QWebSocketServer;

class WebSocketClientWrapper;
class WebSocketTransport;

OK_RESOURCE_LOADER(Platform)

namespace ok::platform {

class Widget : public UI::OMenuWidget {
    Q_OBJECT
public:
    Widget(QWidget* parent = nullptr);
    ~Widget() override;
    void start();

private:
    OK_RESOURCE_PTR(Platform);

    QWebEngineView* webView;
    QWebChannel* webChannel;
    QWebSocketServer* wss;
    WebSocketClientWrapper* clientWrapper;
    std::unique_ptr<QThread> thread;

    void startWsServer();
    void startWebEngine();

public slots:

    void doStart();
    void clientConnected(WebSocketTransport* transport);
};
}  // namespace ok::platform
