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

class QLineEdit;
class QPushButton;

namespace module::meet {

class MeetingOptionWidget;

/**
 * 加入会议界面
 */
class JoinMeetingWidget : public QWidget {
    Q_OBJECT
signals:
    void requstJoinMeeting(const QString& no);

public:
    JoinMeetingWidget(QWidget* parent = nullptr);
    void focusInput();
    void retranslateUi();

private:
    MeetingOptionWidget* optionWidget = nullptr;
    QLineEdit* idEdit = nullptr;

    QPushButton* confirmButton = nullptr;
};
}  // namespace module::meet
