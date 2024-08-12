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

#include "base/OkSettings.h"
#include "base/files.h"
#include "lib/settings/translator.h"
#include "src/base/basic_types.h"
#include "src/base/widgets.h"

#if OK_PLUGIN
#include "about/src/aboutform.h"
#include "plugin/src/PluginManagerForm.h"
#include "settings/src/SettingsForm.h"

#include <settings/src/GeneralForm.h>
#endif

namespace UI {

ConfigWindow::ConfigWindow(QWidget* parent) : OMenuWidget(parent), ui(new Ui::ConfigWindow) {
    OK_RESOURCE_INIT(UIWindowConfig);

    ui->setupUi(this);

    auto qss = ok::base::Files::readStringAll(":/qss/plugin.qss");
    setStyleSheet(qss);

    QString locale = ok::base::OkSettings::getInstance().getTranslation();
    settings::Translator::translate(OK_UIWindowConfig_MODULE, locale);
    settings::Translator::registerHandler([this] { retranslateUi(); }, this);
    retranslateUi();

#if OK_PLUGIN
    ui->tabWidget->addTab(new ok::plugin::PluginManagerForm(this), tr("Plugin form"));
#endif

    auto sw = new SettingsWidget(this);
    connect(sw->general(), &GeneralForm::onLanguageChanged, [](QString locale) {
        settings::Translator::translate(OK_UIWindowConfig_MODULE, locale);
    });

    ui->tabWidget->addTab(sw, tr("Settings form"));
    ui->tabWidget->addTab(new AboutForm(this), tr("About form"));
}

ConfigWindow::~ConfigWindow() {
    settings::Translator::unregister(this);
    delete ui;
}

void ConfigWindow::retranslateUi() {
    ui->retranslateUi(this);
    ui->tabWidget->setTabText(0, tr("Plugin form"));
    ui->tabWidget->setTabText(1, tr("Settings form"));
    ui->tabWidget->setTabText(2, tr("About form"));

    for (int i = 0; i < ui->tabWidget->count(); i++) {
        auto gf = static_cast<GenericForm*>(ui->tabWidget->widget(i));
        gf->retranslateUi();
    }
}

}  // namespace UI
