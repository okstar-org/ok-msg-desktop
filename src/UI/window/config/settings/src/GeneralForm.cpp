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

#include "GeneralForm.h"
#include <QFileDialog>
#include <cmath>

#include "base/OkSettings.h"
#include "lib/settings/settings.h"
#include "lib/settings/translator.h"
#include "src/base/RecursiveSignalBlocker.h"
#include "src/lib/settings/style.h"

namespace UI {
/**
 * @class GeneralForm
 *
 * This form contains all settings that are not suited to other forms
 */
GeneralForm::GeneralForm(SettingsWidget *myParent)
    : GenericForm(QPixmap(":/img/settings/general.png")),
      bodyUI(new Ui::GeneralForm) {
  parent = myParent;

  bodyUI->setupUi(this);

  // block all child signals during initialization
  const RecursiveSignalBlocker signalBlocker(this);

  Settings &s = Settings::getInstance();

  QString locale0 = ok::base::OkSettings::getInstance().getTranslation();
  settings::Translator::translate(OK_UIWindowConfig_MODULE, locale0);
  settings::Translator::registerHandler([this] { retranslateUi(); }, this);



  // 先获取当前语言
#ifndef UPDATE_CHECK_ENABLED
  bodyUI->checkUpdates->setVisible(false);
#endif

#ifndef SPELL_CHECKING
  bodyUI->cbSpellChecking->setVisible(false);
#endif
  // 获取复选框状态
  bodyUI->checkUpdates->setChecked(s.getCheckUpdates());

  auto &okSettings = ok::base::OkSettings::getInstance();

  for (int i = 0; i < okSettings.getLocales().size(); ++i) {
    QString langName;
    auto &locale = okSettings.getLocales().at(i);
    if (locale.startsWith(QLatin1String("eo"))) // QTBUG-57802
      langName = QLocale::languageToString(QLocale::Esperanto);
    else if (locale.startsWith(QLatin1String("jbo")))
      langName = QLatin1String("Lojban");
    else if (locale.startsWith(QLatin1String("pr")))
      langName = QLatin1String("Pirate");
    else if (locale == (QLatin1String("pt"))) // QTBUG-47891
      langName = QStringLiteral("português");
    else
      langName = QLocale(locale).nativeLanguageName();

    bodyUI->transComboBox->insertItem(i, langName);
  }
  // 当前语言下拉框状态
  bodyUI->transComboBox->setCurrentIndex(okSettings.getLocales().indexOf(s.getTranslation()));

  // autorun
  bodyUI->cbAutorun->setChecked(okSettings.getAutorun());

  bodyUI->cbSpellChecking->setChecked(s.getSpellCheckingEnabled());

  bool showSystemTray = okSettings.getShowSystemTray();
  bodyUI->showSystemTray->setChecked(showSystemTray);
  bodyUI->startInTray->setChecked(okSettings.getAutostartInTray());
  bodyUI->startInTray->setEnabled(showSystemTray);
  bodyUI->minimizeToTray->setChecked(okSettings.getMinimizeToTray());
  bodyUI->minimizeToTray->setEnabled(showSystemTray);
  bodyUI->closeToTray->setChecked(okSettings.getCloseToTray());
  bodyUI->closeToTray->setEnabled(showSystemTray);

  retranslateUi();
}

GeneralForm::~GeneralForm() {
//  settings::Translator::unregister(this);
  delete bodyUI;
}

void GeneralForm::on_transComboBox_currentIndexChanged(int index) {
  auto &s = ok::base::OkSettings::getInstance();
  const QString &locale = s.getLocales().at(index);
  s.setTranslation(locale);
  s.saveGlobal();
  settings::Translator::translate(OK_UIWindowConfig_MODULE, "settings_"+locale);
}

void GeneralForm::on_cbAutorun_stateChanged() { ok::base::OkSettings::getInstance().setAutorun(bodyUI->cbAutorun->isChecked()); }

void GeneralForm::on_cbSpellChecking_stateChanged() { Settings::getInstance().setSpellCheckingEnabled(bodyUI->cbSpellChecking->isChecked()); }

void GeneralForm::on_showSystemTray_stateChanged() {
  ok::base::OkSettings::getInstance().setShowSystemTray(bodyUI->showSystemTray->isChecked());
  Settings::getInstance().saveGlobal();
}

void GeneralForm::on_startInTray_stateChanged() { ok::base::OkSettings::getInstance().setAutostartInTray(bodyUI->startInTray->isChecked()); }

void GeneralForm::on_closeToTray_stateChanged() { ok::base::OkSettings::getInstance().setCloseToTray(bodyUI->closeToTray->isChecked()); }

void GeneralForm::on_minimizeToTray_stateChanged() { ok::base::OkSettings::getInstance().setMinimizeToTray(bodyUI->minimizeToTray->isChecked()); }


void GeneralForm::on_checkUpdates_stateChanged() { Settings::getInstance().setCheckUpdates(bodyUI->checkUpdates->isChecked()); }

/**
 * @brief Retranslate all elements in the form.
 */
void GeneralForm::retranslateUi() { bodyUI->retranslateUi(this); }
} // namespace UI