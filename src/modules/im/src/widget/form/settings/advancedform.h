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

#ifndef ADVANCEDFORM_H
#define ADVANCEDFORM_H

#include "genericsettings.h"

class Core;

namespace Ui {
class AdvancedSettings;
}

class AdvancedForm : public GenericForm
{
    Q_OBJECT
public:
    AdvancedForm();
    ~AdvancedForm();
    virtual QString getFormName() final override
    {
        return tr("Advanced");
    }

private slots:
    // Portable

    void on_resetButton_clicked();
    // Debug
    void on_btnCopyDebug_clicked();
    void on_btnExportLog_clicked();
    // Connection
    void on_cbEnableIPv6_stateChanged();
    void on_cbEnableUDP_stateChanged();
    void on_cbEnableLanDiscovery_stateChanged();
    void on_proxyAddr_editingFinished();
    void on_proxyPort_valueChanged(int port);
    void on_proxyType_currentIndexChanged(int index);

private:
    void retranslateUi();

private:
    Ui::AdvancedSettings* bodyUI;
};

#endif // ADVANCEDFORM_H
