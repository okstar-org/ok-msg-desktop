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

#include "JoinMeetingWidget.h"
#include <base/shadowbackground.h>
#include "MeetingOptionWidget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
namespace module::meet {

JoinMeetingWidget::JoinMeetingWidget(QWidget* parent) : QWidget(parent) {
    setContentsMargins(10, 10, 10, 10);
    ShadowBackground* shadowBack = new ShadowBackground(this);
    shadowBack->setShadowRadius(10);

    idEdit = new QLineEdit(this);
    idEdit->setAlignment(Qt::AlignCenter);
    idEdit->setPlaceholderText(tr("Meeting ID"));
    optionWidget = new MeetingOptionWidget(this);

    QPushButton* confirmButton = new QPushButton(tr("Join Meeting"), this);
    confirmButton->setObjectName("confirm");
    confirmButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    confirmButton->setCursor(Qt::PointingHandCursor);
    optionWidget->addFooterButton(confirmButton);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(idEdit);
    mainLayout->addWidget(optionWidget, 1);
    mainLayout->setAlignment(idEdit, Qt::AlignHCenter);

    connect(confirmButton, &QPushButton::clicked,
            [this]() { emit requstJoinMeeting(idEdit->text()); });

    connect(idEdit, &QLineEdit::returnPressed, confirmButton,
            [confirmButton]() { emit confirmButton->clicked(); });
}
}  // namespace module::meet