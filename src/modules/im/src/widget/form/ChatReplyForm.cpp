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

#include "ChatReplyForm.h"
#include "ui_ChatReplyForm.h"
namespace module::im {

ChatReplyForm::ChatReplyForm(const ChatReplyItem& item, QWidget* parent)
        : QWidget(parent), ui(new Ui::ChatReplyForm), item{item} {
    ui->setupUi(this);

    ui->nickname->setText(item.nickname);
    ui->content->setText(item.content);
    ui->removeReplyButton->setCursor(Qt::PointingHandCursor);

    connect(ui->removeReplyButton, &QPushButton::clicked, [&](bool c) { emit removeEvent(); });
}

ChatReplyForm::~ChatReplyForm() {
    delete ui;
}
}  // namespace module::im