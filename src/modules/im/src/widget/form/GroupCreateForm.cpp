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

#include "GroupCreateForm.h"
#include "ui_GroupCreateForm.h"
namespace module::im {

GroupCreateForm::GroupCreateForm(QWidget* parent) : QWidget(parent), ui(new Ui::GroupCreateForm) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(tr("Create new group"));

    connect(ui->confirm, &QPushButton::released, [&]() {
        auto name = ui->group->text();
        if (name.trimmed().isEmpty()) {
            return;
        }
        emit confirmed(name);
    });
}

GroupCreateForm::~GroupCreateForm() {
    disconnect(ui->confirm);
    delete ui;
}
}  // namespace module::im
