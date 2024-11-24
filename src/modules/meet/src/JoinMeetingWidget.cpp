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
#include "MeetingOptionWidget.h"
#include <base/shadowbackground.h>

#include <QLineEdit>
#include <QVBoxLayout>

JoinMeetingWidget::JoinMeetingWidget(QWidget* parent) : QWidget(parent) {

    setContentsMargins(10, 10, 10, 10);
    ShadowBackground* shadowBack = new ShadowBackground(this);
    shadowBack->setShadowRadius(10);

    idEdit = new QLineEdit(this);
    idEdit->setAlignment(Qt::AlignCenter);
    idEdit->setPlaceholderText(tr("Meeting ID"));
    optionWidget = new MeetingOptionWidget(this);
    optionWidget->setConfirmButtonText(tr("Join Meeting"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(idEdit);
    mainLayout->addWidget(optionWidget, 1);
    mainLayout->setAlignment(idEdit, Qt::AlignHCenter);

    connect(optionWidget, &MeetingOptionWidget::confirmed,
            [this]() { emit requstJoinMeeting(idEdit->text()); });
}
