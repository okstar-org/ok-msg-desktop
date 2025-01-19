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

#include "StorageSettingsForm.h"
#include <cmath>
#include "lib/storage/settings/OkSettings.h"
#include "ui_StorageSettingsForm.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFont>

#include <QRegularExpressionValidator>
#include <QStyleFactory>
#include <QTime>
#include <QVector>

#include "lib/storage/settings/translator.h"
#include "src/Bus.h"
#include "src/application.h"
#include "src/base/RecursiveSignalBlocker.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/lib/session/profile.h"
#include "src/lib/storage/settings/style.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/widget.h"

namespace module::im {

/**
 * @brief Constructor of StorageSettingsForm.
 * @param myParent Setting widget which will contain this form as tab.
 *
 * Restores all controls from the settings.
 */
StorageSettingsForm::StorageSettingsForm(SettingsWidget* myParent)
        : GenericForm(QPixmap(":/img/settings/general.png"), myParent)
        , bodyUI{new Ui::StorageSettingsForm} {
    bodyUI->setupUi(this);
    parent = myParent;

    // block all child signals during initialization
    const ok::base::RecursiveSignalBlocker signalBlocker(this);

    eventsInit();
    settings::Translator::registerHandler(std::bind(&StorageSettingsForm::retranslateUi, this),
                                          this);

    auto bus = ok::Application::Instance()->bus();
    connect(bus, &ok::Bus::profileChanged, this, &StorageSettingsForm::onProfileChanged);
}

StorageSettingsForm::~StorageSettingsForm() {
    settings::Translator::unregister(this);
    delete bodyUI;
}

// void StorageSettingsForm::on_statusChanges_stateChanged() {
//     Nexus::getProfile()->getSettings()->setStatusChangeNotificationEnabled(bodyUI->statusChanges->isChecked());
// }
//
// void StorageSettingsForm::on_groupJoinLeaveMessages_stateChanged() {
//     Nexus::getProfile()->getSettings()->setShowGroupJoinLeaveMessages(bodyUI->groupJoinLeaveMessages->isChecked());
// }
//
// void StorageSettingsForm::on_autoAwaySpinBox_editingFinished() {
//     int minutes = bodyUI->autoAwaySpinBox->value();
//     Nexus::getProfile()->getSettings()->setAutoAwayTime(minutes);
// }

void StorageSettingsForm::on_autoacceptFiles_stateChanged() {
    // lib::settings::OkNexus::getSettings()->setAutoSaveEnabled(bodyUI->autoacceptFiles->isChecked());
}

void StorageSettingsForm::on_autoSaveFilesDir_clicked() {
    QString previousDir = Nexus::getProfile()->getSettings()->getGlobalAutoAcceptDir();
    QString directory = QFileDialog::getExistingDirectory(
            Q_NULLPTR, tr("Choose an auto accept directory", "popup title"), QDir::homePath());
    if (directory.isEmpty())  // cancel was pressed
        directory = previousDir;

    Nexus::getProfile()->getSettings()->setGlobalAutoAcceptDir(directory);
    bodyUI->autoSaveFilesDir->setText(directory);
}

void StorageSettingsForm::on_maxAutoAcceptSizeMB_editingFinished() {
    // auto newMaxSizeMB = bodyUI->maxAutoAcceptSizeMB->value();
    // auto newMaxSizeB = std::lround(newMaxSizeMB * 1024 * 1024);

    // Nexus::getProfile()->getSettings()->setMaxAutoAcceptSize(newMaxSizeB);
}

/**
 * @brief Retranslate all elements in the form.
 */
void StorageSettingsForm::retranslateUi() {
    bodyUI->retranslateUi(this);
}

void StorageSettingsForm::onProfileChanged(Profile* profile) {
    auto s = profile->getSettings();
    bodyUI->autoSaveFilesDir->setText(s->getGlobalAutoAcceptDir());

    // 先获取当前语言
    // auto& okSettings = lib::settings::OkSettings::getInstance();
    // bodyUI->statusChanges->setChecked(s.getStatusChangeNotificationEnabled());
    // bodyUI->groupJoinLeaveMessages->setChecked(s.getShowGroupJoinLeaveMessages());

    // bodyUI->autoAwaySpinBox->setValue(s.getAutoAwayTime());
    // bodyUI->maxAutoAcceptSizeMB->setValue(static_cast<double>(s.getMaxAutoAcceptSize()) / 1024 /
    // 1024); bodyUI->autoacceptFiles->setChecked(okSettings.getAutoSaveEnabled());
}
}  // namespace module::im