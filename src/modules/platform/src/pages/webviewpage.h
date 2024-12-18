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

#pragma once

#include "../platformpage.h"

class QWebEngineView;

namespace module::platform {

class PlatformPageContainer;

class WebviewPage : public PlatformPage {
public:
    explicit WebviewPage(const QUrl& url,
                         const QString& type,
                         const QString& uuid,
                         const QString& title,
                         PlatformPageContainer* container);
    ~WebviewPage() override;
    QString getTitle() override {
        return pageTitle;
    }
    QUrl getUrl() override {
        return pageUrl;
    }
    QWidget* getWidget() override;

    void createContent(QWidget* parent) override;
    void start() override;
    void doClose() override;

private:
    QUrl pageUrl;
    QString appType;
    // App uuid
    QString appUuid;
    QString pageTitle;
    std::unique_ptr<QWebEngineView> webView;
};
}  // namespace module::platform