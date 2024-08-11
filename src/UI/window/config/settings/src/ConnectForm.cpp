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

#include "ConnectForm.h"
#include "ui_ConnectForm.h"

#include "UI/widget/GenericForm.h"

namespace UI {

ConnectForm::ConnectForm(SettingsWidget* parent)
        : GenericForm(QPixmap(":/img/settings/general.png")), ui(new Ui::ConnectForm) {
    ui->setupUi(this);
    auto pt = ui->proxyType;
    pt->addItem("HTTP");
    pt->addItem("HTTPS");
    pt->addItem("SOCKS5");
}

ConnectForm::~ConnectForm() { delete ui; }

void ConnectForm::retranslateUi() { ui->retranslateUi(this); }

}  // namespace UI
