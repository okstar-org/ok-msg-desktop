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
#include "ConfigWindow.h"
#include "ui_ConfigWindow.h"

#include <memory>
#include <QWidget>

#include "lib/storage/settings/OkSettings.h"
#include "lib/storage/settings/style.h"
#include "lib/storage/settings/translator.h"
#include "src/base/basic_types.h"

#include "about/src/aboutform.h"
#include "base/widgets.h"
#include "modules/im/src/widget/form/settingswidget.h"
#include "plugin/src/PluginManagerForm.h"
#include "settings/src/GeneralForm.h"
#include "settings/src/SettingsForm.h"

namespace module::config {

ConfigWindow::ConfigWindow(QWidget* parent) : UI::window::OPage(parent), ui(new Ui::ConfigWindow) {
    ui->setupUi(this);

    ui->tabWidget->setObjectName("mainTab");

    // 设置
    auto sw = new GeneralForm(this);
    connect(sw, &GeneralForm::onLanguageChanged, [](const QString& locale) {
        settings::Translator::translate(OK_Config_MODULE, locale);
    });
    ui->tabWidget->addTab(sw, tr("Settings form"));

#if OK_PLUGIN
    // 插件管理
    ui->tabWidget->addTab(new PluginManagerForm(this), tr("Plugin form"));
#endif

    // 关于
    ui->tabWidget->addTab(new AboutForm(this), tr("About form"));

    ui->tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);
    reloadTheme();

    QString locale = lib::settings::OkSettings().getTranslation();
    settings::Translator::translate(OK_Config_MODULE, locale);
    settings::Translator::registerHandler([this] { retranslateUi(); }, this);
    retranslateUi();
}

ConfigWindow::~ConfigWindow() {
    settings::Translator::unregister(this);
    delete ui;
}

void ConfigWindow::reloadTheme() {
    auto& style = lib::settings::Style::getStylesheet("general.css");
    setStyleSheet(style);
}

void ConfigWindow::retranslateUi() {
    ui->retranslateUi(this);

    ui->tabWidget->setTabText(0, tr("Settings form"));
    ui->tabWidget->setTabText(1, tr("Plugin form"));
    ui->tabWidget->setTabText(2, tr("About form"));

    for (int i = 0; i < ui->tabWidget->count(); i++) {
        auto gf = static_cast<UI::GenericForm*>(ui->tabWidget->widget(i));
        gf->retranslateUi();
    }
}

}  // namespace module::config
