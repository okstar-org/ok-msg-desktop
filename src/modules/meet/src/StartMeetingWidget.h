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

#pragma once

#include <QWidget>
#include "Defines.h"
#include "lib/ortc/ok_rtc.h"

class QLineEdit;
class QPushButton;

namespace module::meet {

class Widget;
class MeetingOptionWidget;

/**
 * 新建会议界面
 */
class StartMeetingWidget : public QWidget {
    Q_OBJECT
signals:
    void requstStartMeeting(const QString& name, const lib::ortc::CtrlState& ctrlState);
    void requstDisbandMeeting();
    void requstShareMeeting();

public:
    explicit StartMeetingWidget(QWidget* parent = nullptr);
    QString getName();
    void setMeetingState(MeetingState state);
    void focusInput();
    void retranslateUi();
    void updateUi();

protected:
    void showEvent(QShowEvent* e) override;

private:
    MeetingOptionWidget* optionWidget = nullptr;
    QLineEdit* meetingNameEdit = nullptr;

    // 确定按钮
    QPushButton* confirmButton = nullptr;
    // 解散按钮
    QPushButton* disbandButton = nullptr;
    // 分享按钮
    QPushButton* shareButton = nullptr;

    /**
     * 主框架
     */
    Widget* widget;
    //    MeetingState meetingState = MeetingState::NoMeeting;
};
}  // namespace module::meet
