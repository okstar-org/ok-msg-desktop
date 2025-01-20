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

#ifndef CONFIG_GENERALFORM_H
#define CONFIG_GENERALFORM_H

#include "lib/ui/widget/GenericForm.h"
#include "ui_GeneralForm.h"

namespace module::config {

class SettingsWidget;

class GeneralForm : public lib::ui::GenericForm {
    Q_OBJECT
public:
    explicit GeneralForm(QWidget* parent = nullptr);
    ~GeneralForm() override;

    virtual QString getFormName() final override {
        return tr("General");
    }

    void retranslateUi() override;
signals:
    void updateIcons();
    void onLanguageChanged(const QString& locale);

private slots:
    void on_transComboBox_currentIndexChanged(int index);
    void on_cbAutorun_stateChanged();
    void on_cbSpellChecking_stateChanged();
    void on_showSystemTray_stateChanged();
    void on_startInTray_stateChanged();
    void on_closeToTray_stateChanged();
    void on_minimizeToTray_stateChanged();
    void on_checkUpdates_stateChanged();
    void on_timestamp_editTextChanged(const QString& format);
    void on_dateFormats_editTextChanged(const QString& format);
    void on_themeColorCBox_currentIndexChanged(int);

private:
    Ui::GeneralForm* ui;
    const int MAX_FORMAT_LENGTH = 128;
};
}  // namespace UI
#endif
