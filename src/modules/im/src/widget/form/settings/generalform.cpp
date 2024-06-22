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

#include "generalform.h"
#include "ui_generalsettings.h"

#include <QFileDialog>
#include <cmath>

#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "lib/settings/translator.h"
#include "base/OkSettings.h"

#include "src/widget/widget.h"

/**
 * @class GeneralForm
 *
 * This form contains all settings that are not suited to other forms
 */
GeneralForm::GeneralForm(SettingsWidget* myParent)
    : GenericForm(QPixmap(":/img/settings/general.png"))
    , bodyUI(new Ui::GeneralSettings)
{
    parent = myParent;

    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    Settings& s = Settings::getInstance();
    //先获取当前语言
#ifndef UPDATE_CHECK_ENABLED
    bodyUI->checkUpdates->setVisible(false);
#endif

#ifndef SPELL_CHECKING
    bodyUI->cbSpellChecking->setVisible(false);
#endif
    //获取复选框状态
    bodyUI->checkUpdates->setChecked(s.getCheckUpdates());

    auto & okSettings = ok::base::OkSettings::getInstance();

    for (int i = 0; i < okSettings.getLocales().size(); ++i) {
        QString langName;
        auto & locale = okSettings.getLocales().at(i);
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
    //当前语言下拉框状态
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

    bodyUI->statusChanges->setChecked(s.getStatusChangeNotificationEnabled());
    bodyUI->groupJoinLeaveMessages->setChecked(s.getShowGroupJoinLeaveMessages());

    bodyUI->autoAwaySpinBox->setValue(s.getAutoAwayTime());
    bodyUI->autoSaveFilesDir->setText(s.getGlobalAutoAcceptDir());
    bodyUI->maxAutoAcceptSizeMB->setValue(static_cast<double>(s.getMaxAutoAcceptSize()) / 1024 / 1024);
    bodyUI->autoacceptFiles->setChecked(okSettings.getAutoSaveEnabled());


#ifndef QTOX_PLATFORM_EXT
    bodyUI->autoAwayLabel->setEnabled(false); // these don't seem to change the appearance of the widgets,
    bodyUI->autoAwaySpinBox->setEnabled(false); // though they are unusable
#endif

    eventsInit();
    settings::Translator::registerHandler(std::bind(&GeneralForm::retranslateUi, this), this);
}

GeneralForm::~GeneralForm()
{
    settings::Translator::unregister(this);
    delete bodyUI;
}

void GeneralForm::on_transComboBox_currentIndexChanged(int index)
{
    auto & s = ok::base::OkSettings::getInstance();
    const QString& locale = s.getLocales().at(index);
    s.setTranslation(locale);
    s.saveGlobal();
    settings::Translator::translate(OK_IM_MODULE, locale);
}

void GeneralForm::on_cbAutorun_stateChanged()
{
    ok::base::OkSettings::getInstance().setAutorun(bodyUI->cbAutorun->isChecked());
}

void GeneralForm::on_cbSpellChecking_stateChanged()
{
    Settings::getInstance().setSpellCheckingEnabled(bodyUI->cbSpellChecking->isChecked());
}

void GeneralForm::on_showSystemTray_stateChanged()
{
    ok::base::OkSettings::getInstance().setShowSystemTray(bodyUI->showSystemTray->isChecked());
    Settings::getInstance().saveGlobal();
}

void GeneralForm::on_startInTray_stateChanged()
{
    ok::base::OkSettings::getInstance().setAutostartInTray(bodyUI->startInTray->isChecked());
}

void GeneralForm::on_closeToTray_stateChanged()
{
    ok::base::OkSettings::getInstance().setCloseToTray(bodyUI->closeToTray->isChecked());
}

void GeneralForm::on_minimizeToTray_stateChanged()
{
    ok::base::OkSettings::getInstance().setMinimizeToTray(bodyUI->minimizeToTray->isChecked());
}

void GeneralForm::on_statusChanges_stateChanged()
{
    Settings::getInstance().setStatusChangeNotificationEnabled(bodyUI->statusChanges->isChecked());
}

void GeneralForm::on_groupJoinLeaveMessages_stateChanged()
{
    Settings::getInstance().setShowGroupJoinLeaveMessages(bodyUI->groupJoinLeaveMessages->isChecked());
}

void GeneralForm::on_autoAwaySpinBox_editingFinished()
{
    int minutes = bodyUI->autoAwaySpinBox->value();
    Settings::getInstance().setAutoAwayTime(minutes);
}

void GeneralForm::on_autoacceptFiles_stateChanged()
{
    ok::base::OkSettings::getInstance().setAutoSaveEnabled(bodyUI->autoacceptFiles->isChecked());
}

void GeneralForm::on_autoSaveFilesDir_clicked()
{
    QString previousDir = Settings::getInstance().getGlobalAutoAcceptDir();
    QString directory =
        QFileDialog::getExistingDirectory(Q_NULLPTR,
                                          tr("Choose an auto accept directory", "popup title"),
                                          QDir::homePath());
    if (directory.isEmpty()) // cancel was pressed
        directory = previousDir;

    Settings::getInstance().setGlobalAutoAcceptDir(directory);
    bodyUI->autoSaveFilesDir->setText(directory);
}

void GeneralForm::on_maxAutoAcceptSizeMB_editingFinished()
{
    auto newMaxSizeMB = bodyUI->maxAutoAcceptSizeMB->value();
    auto newMaxSizeB = std::lround(newMaxSizeMB * 1024 * 1024);

    Settings::getInstance().setMaxAutoAcceptSize(newMaxSizeB);
}

void GeneralForm::on_checkUpdates_stateChanged()
{
    Settings::getInstance().setCheckUpdates(bodyUI->checkUpdates->isChecked());
}

/**
 * @brief Retranslate all elements in the form.
 */
void GeneralForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
}
