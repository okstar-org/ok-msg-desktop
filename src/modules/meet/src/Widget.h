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
// Created by gaojie on 24-7-31.
//

#pragma once

#include <QMutex>
#include <QWidget>

#include "Defines.h"
#include "base/resources.h"
#include "lib/ortc/ok_rtc.h"
#include "lib/ui/widget/OPage.h"

#include <QPointer>

OK_RESOURCE_LOADER(Meet)
OK_RESOURCE_LOADER(MeetRes)

namespace Ui {
class WorkPlatform;
}

namespace module::meet {

class StartMeetingWidget;
class JoinMeetingWidget;
class MeetingVideoFrame;

/**
 * 会议主界面
 */
class Widget : public UI::window::OPage {
    Q_OBJECT
public:
    explicit Widget(QWidget* parent = nullptr);
    ~Widget() override;
    void start();
    void reloadTheme();

    [[nodiscard]] const MeetingState& getState() const {
        return state;
    }

    void setState(const MeetingState& state_);

    void activate();

protected:
    void initTranslate();
    void retranslateUi();

private:
    /**
     * 加入会议
     * @param no 会议编号
     */
    void joinMeeting(const QString& no);

    /**
     * 生成分享信息
     * @return
     */
    Share makeShare();

    /**
     * 开始会议
     * @param name 会议名称
     */
    void createMeeting(const QString& name, const lib::ortc::CtrlState& ctrlState);

    /**
     * 解散会议（销毁）
     */
    void destroyMeeting();
    /**
     * 分享会议
     */
    void shareMeeting();

private:
    OK_RESOURCE_PTR(Meet);
    OK_RESOURCE_PTR(MeetRes);

    Ui::WorkPlatform* ui;
    StartMeetingWidget* startMeetWidget = nullptr;
    JoinMeetingWidget* joinMeetWidget = nullptr;
    QPointer<MeetingVideoFrame> view;

    /**
     * 当前正在进行中的会议名称
     * - 会议开始，设置会议号
     * - 会议结束，清空会议号。
     */
    QString currentMeetingName;
    QMutex mutex;

    MeetingState state;

public slots:
    void doStart();
};

}  // namespace module::meet
