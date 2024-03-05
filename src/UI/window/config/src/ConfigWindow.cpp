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

#include "base/files.h"
#include "lib/settings/OkSettings.h"
#include "lib/settings/translator.h"
#include <base/basic_types.h>
#include <base/widgets.h>

#if OK_PLUGIN
#include "UI/component/PluginManagerForm.h"
#include "lib/settings/OkSettings.h"
#include "lib/settings/translator.h"
#endif


namespace UI {

ConfigWindow::ConfigWindow(QWidget *parent): QFrame(parent),ui(new Ui::ConfigWindow){
  OK_RESOURCE_INIT(UIWindowConfig);

  ui->setupUi(this);
  setObjectName(qsl("Page:%1").arg(static_cast<int>(PageMenu::setting)));

  QString locale = ok::base::OkSettings::getInstance().getTranslation();
//  settings::Translator::registerHandler(std::bind(&ConfigWindow::retranslateUi, this), this);
  qDebug()<<"locale"<<locale;

  settings::Translator::translate(OK_UIWindowConfig_MODULE, locale);
  retranslateUi();

  auto qss = base::Files::readStringAll(":/qss/plugin.qss");
  setStyleSheet(qss);

  init();
}

ConfigWindow::~ConfigWindow() {
delete ui;
}

void ConfigWindow::init() {
#if OK_PLUGIN
  qDebug()<<tr("Plugin form");
  ui->tabWidget->addTab(new ok::plugin::PluginManagerForm(this), tr("Plugin form"));
#endif
}

void ConfigWindow::retranslateUi() {
  ui->retranslateUi(this);
}

} // namespace UI
