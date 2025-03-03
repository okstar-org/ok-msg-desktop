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

#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>

#include "lib/ui/widget/OFrame.h"

namespace Ui {
class ConfigWindow;
}

namespace module::config {

/**
 * 配置窗口
 */
class ConfigWindow : public lib::ui::OFrame {
    Q_OBJECT
public:
    explicit ConfigWindow(QWidget* parent = nullptr);
    ~ConfigWindow() override;
    void reloadTheme() override;

protected:
    void retranslateUi();

private:
    Ui::ConfigWindow* ui;

    std::unique_ptr<QVBoxLayout> bodyLayout;
    std::unique_ptr<QTabWidget> settingsWidgets;
};

}  // namespace module::config
