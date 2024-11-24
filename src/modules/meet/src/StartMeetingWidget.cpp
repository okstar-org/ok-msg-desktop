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

#include "StartMeetingWidget.h"
#include <base/shadowbackground.h>
#include "MeetingOptionWidget.h"

#include <QLineEdit>
#include <QVBoxLayout>

StartMeetingWidget::StartMeetingWidget(QWidget* parent) : QWidget(parent) {
    setContentsMargins(10, 10, 10, 10);
    ShadowBackground* shadowBack = new ShadowBackground(this);
    shadowBack->setShadowRadius(10);

    meetingNameEdit = new QLineEdit(this);
    meetingNameEdit->setAlignment(Qt::AlignCenter);
    meetingNameEdit->setPlaceholderText(tr("Meeting Name"));
    optionWidget = new MeetingOptionWidget(this);
    optionWidget->setConfirmButtonText(tr("Start Meeting"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(meetingNameEdit);
    mainLayout->addWidget(optionWidget, 1);
    mainLayout->setAlignment(meetingNameEdit, Qt::AlignHCenter);

    connect(optionWidget, &MeetingOptionWidget::confirmed, [this]() {
        auto n = getName();
        if (n.isEmpty()) {
            return;
        }
        emit requstStartMeeting(n);
    });
}
QString StartMeetingWidget::getName() {
    return meetingNameEdit->text();
}
