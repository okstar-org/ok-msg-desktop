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
#include "platformpage.h"
#include "platformpagecontainer.h"

#include <QJsonArray>
#include <QPointer>

class QWebEngineView;
class QWebChannel;
class QThread;
class QWebSocketServer;
class WebSocketClientWrapper;
class WebSocketTransport;

namespace ok::platform {

class AppCenterPage;

class AppCenterWidget : public UI::OWidget {
    Q_OBJECT
signals:
    void appPageRequest(const QUrl& url, const QString& title);

public:
    AppCenterWidget(AppCenterPage * page, QWidget* parent = nullptr);
    void start();

private:
    std::unique_ptr<QThread> thread;

    QWebEngineView* webView = nullptr;
    QWebChannel* webChannel = nullptr;
    QWebSocketServer* wss = nullptr;
    WebSocketClientWrapper* clientWrapper = nullptr;

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
    AppCenterPage* platformPage;
};

// 应用中心Page页
class AppCenterPage : public PlatformPage {
public:
    using PlatformPage::PlatformPage;
    ~AppCenterPage();
    QString getTitle() override;
    void createContent(QWidget* parent) override;
    QWidget* getWidget() override;
    QUrl getUrl() override;
    void start() override;
    void doClose() override;
    bool pageClosable() override;

private:
    void openAppPage(const QUrl& url, const QString& title);
    void onWebMessageReceived(const QJsonValue & value);

private:
    QPointer<AppCenterWidget> widget = nullptr;
    friend class AppCenterWidget;
};
}  // namespace ok::platform
