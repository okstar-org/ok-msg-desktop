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

//
// Created by gaojie on 25-1-17.
//

#pragma once

#include "Defines.h"
#include "Widget.h"
#include "lib/ortc/ok_rtc.h"
#include "lib/ui/widget/OWidget.h"

class QLineEdit;

namespace module::classroom {

class MeetingOptionWidget;

class StartRoomWidget : public UI::OWidget {
    Q_OBJECT
public:
    explicit StartRoomWidget(QWidget* parent = nullptr);
    ~StartRoomWidget() override;
    void setMeetingState(RoomState state);
    void focusInput();
    QString getName();
    void updateUi();

private:
    Widget* widget;
    QLineEdit* meetingNameEdit;
    MeetingOptionWidget* optionWidget;

    // 确定按钮
    QPushButton* confirmButton = nullptr;
    // 解散按钮
    QPushButton* disbandButton = nullptr;
    // 分享按钮
    QPushButton* shareButton = nullptr;

signals:
    void requstStartMeeting(const QString& name, const lib::ortc::CtrlState& ctrlState);
    void requstDisbandMeeting();
    void requstShareMeeting();
};
}  // namespace module::classroom
