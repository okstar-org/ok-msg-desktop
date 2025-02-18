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
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "Defines.h"
#include "MeetingOptionWidget.h"
#include "Widget.h"
#include "base/shadowbackground.h"

namespace module::meet {

static QPushButton* createButton(const QString& text, QWidget* parent, const QString& id) {
    auto* button = new QPushButton(text, parent);
    button->setObjectName(id);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

StartMeetingWidget::StartMeetingWidget(QWidget* parent) : QWidget(parent) {
    widget = dynamic_cast<Widget*>(parent);

    setContentsMargins(10, 10, 10, 10);

    auto shadowBack = new ok::base::ShadowBackground(this);
    shadowBack->setShadowRadius(10);

    meetingNameEdit = new QLineEdit(this);
    meetingNameEdit->setAlignment(Qt::AlignCenter);
    meetingNameEdit->setPlaceholderText(tr("Meeting Name"));

    optionWidget = new MeetingOptionWidget(this);
    confirmButton = createButton(tr("Start Meeting"), optionWidget, "confirm");
    connect(confirmButton, &QPushButton::clicked, [this]() {
        auto n = getName();
        if (n.isEmpty()) {
            return;
        }
        emit requstStartMeeting(n, optionWidget->getConf(), optionWidget->getCtrlState());
    });

    shareButton = createButton(tr("Share"), optionWidget, "share");
    connect(shareButton, &QPushButton::clicked, this, &StartMeetingWidget::requstShareMeeting);
    disbandButton = createButton(tr("Disband"), optionWidget, "disband");
    connect(disbandButton, &QPushButton::clicked, this, &StartMeetingWidget::requstDisbandMeeting);

    optionWidget->addFooterButton(disbandButton);
    optionWidget->addFooterButton(shareButton);
    optionWidget->addFooterButton(confirmButton);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(meetingNameEdit);
    mainLayout->addWidget(optionWidget, 1);
    mainLayout->setAlignment(meetingNameEdit, Qt::AlignHCenter);

    connect(meetingNameEdit, &QLineEdit::returnPressed, this,
            [this]() { emit confirmButton->clicked(); });

    updateUi();
}
QString StartMeetingWidget::getName() {
    return meetingNameEdit->text();
}

void StartMeetingWidget::setMeetingState(MeetingState state) {
    updateUi();
}

void StartMeetingWidget::focusInput() {
    meetingNameEdit->setFocus();
}

void StartMeetingWidget::retranslateUi() {
    confirmButton->setText(tr("Start Meeting"));
    shareButton->setText(tr("Share"));
    disbandButton->setText(tr("Disband"));
    meetingNameEdit->setPlaceholderText(tr("Meeting Name"));
    optionWidget->retranslateUi();
}

void StartMeetingWidget::updateUi() {
    auto meetingState = widget->getState();
    switch (meetingState) {
        case MeetingState::NoMeeting:
            shareButton->setVisible(false);
            disbandButton->setVisible(false);
            confirmButton->setVisible(true);
            confirmButton->setEnabled(true);
            meetingNameEdit->setReadOnly(false);
            break;
        case MeetingState::CreatingMeeting:
            shareButton->setVisible(false);
            disbandButton->setVisible(false);
            confirmButton->setVisible(true);
            confirmButton->setEnabled(false);
            meetingNameEdit->setReadOnly(true);
            break;
        case MeetingState::OnMeeting:
            shareButton->setVisible(true);
            disbandButton->setVisible(true);
            confirmButton->setVisible(false);
            meetingNameEdit->setReadOnly(true);
            break;
        default:
            break;
    }
}

void StartMeetingWidget::showEvent(QShowEvent* e) {}
}  // namespace module::meet
