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
#include "UI/main/src/OMenuWidget.h"
#include "base/resources.h"
#include "lib/ui/widget/OPage.h"
#include "lib/ui/widget/OWidget.h"

OK_RESOURCE_LOADER(Platform)
OK_RESOURCE_LOADER(PlatformRes)

namespace Ui {
class WorkPlatform;
}

namespace module::platform {

class AppCenterWidget;
class PlatformPage;
class PlatformPageContainer;

/**
 * 工作平台主界面
 */
class Widget : public lib::ui::OPage {
    Q_OBJECT
public:
    explicit Widget(QWidget* parent = nullptr);
    ~Widget() override;

    /**
     * 启动
     */
    void start();

    /**
     * 重载主题
     */
    void reloadTheme();

protected:
    // 工作平台页的相关操作
    PlatformPage* findPage(const QUrl& url);

    // 增加界面
    void addPage(PlatformPage* page, bool active = true);
    // 移除界面
    bool removePage(PlatformPage* page);
    void activePage(PlatformPage* page);
    void retranslateUi();

private:
    // 请求关闭tab
    void requestCloseTab();
    // 执行关闭
    void doClose(int index, PlatformPage* page);

private:
    OK_RESOURCE_PTR(Platform);
    OK_RESOURCE_PTR(PlatformRes);

    Ui::WorkPlatform* ui;

public slots:
    void doStart();

    friend class PlatformPageContainer;
};

}  // namespace module::platform
