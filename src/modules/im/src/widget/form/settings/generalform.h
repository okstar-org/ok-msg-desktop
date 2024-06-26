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

#ifndef GENERALFORM_H
#define GENERALFORM_H

#include "genericsettings.h"

namespace Ui {
class GeneralSettings;
}

class SettingsWidget;

class GeneralForm : public GenericForm
{
    Q_OBJECT
public:
    explicit GeneralForm(SettingsWidget* parent);
    ~GeneralForm();
    virtual QString getFormName() final override
    {
        return tr("General");
    }
signals:
    void updateIcons();

private slots:
    void on_transComboBox_currentIndexChanged(int index);
    void on_cbAutorun_stateChanged();
    void on_cbSpellChecking_stateChanged();
    void on_showSystemTray_stateChanged();
    void on_startInTray_stateChanged();
    void on_closeToTray_stateChanged();
    void on_autoAwaySpinBox_editingFinished();
    void on_minimizeToTray_stateChanged();
    void on_statusChanges_stateChanged();
    void on_groupJoinLeaveMessages_stateChanged();
    void on_autoacceptFiles_stateChanged();
    void on_maxAutoAcceptSizeMB_editingFinished();
    void on_autoSaveFilesDir_clicked();
    void on_checkUpdates_stateChanged();

private:
    void retranslateUi();

private:
    Ui::GeneralSettings* bodyUI;
    SettingsWidget* parent;
};

#endif
