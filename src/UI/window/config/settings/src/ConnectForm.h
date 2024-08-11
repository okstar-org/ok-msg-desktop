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

#ifndef CONNECTFORM_H
#define CONNECTFORM_H

#include <QWidget>

#include "src/UI/widget/GenericForm.h"

namespace Ui {
class ConnectForm;
}

namespace UI {
class SettingsWidget;
class ConnectForm : public GenericForm {
    Q_OBJECT

public:
    explicit ConnectForm(SettingsWidget* parent = nullptr);
    ~ConnectForm();

    virtual QString getFormName() final override { return tr("Connection"); }
    void retranslateUi() override;

private:
    Ui::ConnectForm* ui;
};
}  // namespace UI
#endif  // CONNECTFORM_H
