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
#include <QPushButton>
#include "lib/backend/UserService.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "ui_friendform.h"

FriendForm::FriendForm(const lib::backend::OrgStaff& staff_, QWidget* parent)
        : QFrame(parent), ui(new Ui::FriendForm), staff(staff_) {
    ui->setupUi(this);
    ui->nickname->setText(staff.nickname);
    ui->posts->setText(staff.postNames);
    ui->name->setText(staff.profile.getName());
    ui->phone->setText(staff.profile.phone);
    ui->email->setText(staff.profile.email);
    ui->accountId->hide();
    ui->accountId->setText(QString::number(staff.accountId));

    ui->addFriend->setCursor(Qt::PointingHandCursor);

    connect(ui->addFriend, &QPushButton::released, [&]() {
        emit add(staff.toContactId(Nexus::getProfile()->getHost()), staff.profile.getName());
    });
}

FriendForm::~FriendForm() {
    disconnect(ui->addFriend);
    delete ui;
}
