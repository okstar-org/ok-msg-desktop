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

#include "friendform.h"
#include "lib/backend/UserService.h"
#include "ui_friendform.h"

FriendForm::FriendForm(const ok::backend::OrgStaff& staff_, QWidget* parent)
        : QFrame(parent), ui(new Ui::FriendForm), staff(staff_) {
    ui->setupUi(this);
    ui->no->setText(staff.no);
    ui->posts->setText(staff.posts);
    ui->name->setText(staff.name);
    ui->phone->setText(staff.phone);
    ui->email->setText(staff.email);

    connect(ui->addFriend, &QPushButton::released,
            [&]() { emit add(staff.toContactId(), staff.name); });
}

FriendForm::~FriendForm() {
    disconnect(ui->addFriend);
    delete ui;
}
