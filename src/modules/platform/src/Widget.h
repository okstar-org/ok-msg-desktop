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

OK_RESOURCE_LOADER(Platform)

namespace Ui {
class WorkPlatform;
}

namespace ok::platform {

class AppCenterWidget;
class PlatformPage;
class PlatformPageContainer;

class Widget : public UI::OMenuWidget {
    Q_OBJECT
public:
    Widget(QWidget* parent = nullptr);
    ~Widget() override;
    void start();

protected:
    // 工作平台页的相关操作
    PlatformPage* findPage(const QUrl& url);
    void addPage(PlatformPage* page, bool active = true);
    bool removePage(PlatformPage* page);
    void activePage(PlatformPage* page);
    void retranslateUi();

private:
    void requestCloseTab();
    void doClose(int index, PlatformPage* page);

private:
    OK_RESOURCE_PTR(Platform);
    Ui::WorkPlatform* ui;

public slots:
    void doStart();

    friend class PlatformPageContainer;
};

}  // namespace ok::platform
