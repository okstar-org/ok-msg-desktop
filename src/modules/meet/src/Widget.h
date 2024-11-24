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

#include "UI/widget/OMenuWidget.h"
#include "base/resources.h"

OK_RESOURCE_LOADER(Meet)
OK_RESOURCE_LOADER(MeetRes)

namespace Ui {
class WorkPlatform;
}

class StartMeetingWidget;
class JoinMeetingWidget;
class MeetingVideoFrame;

namespace module::meet {

/**
 * 分享信息
 */
struct Share {
    // 会议编号
    QString no;
    // 会议名称
    QString name;
};

class Widget : public UI::OMenuWidget {
    Q_OBJECT
public:
    Widget(QWidget* parent = nullptr);
    ~Widget() override;
    void start();
    void reloadTheme();

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
    void createMeeting(const QString& name);

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
    MeetingVideoFrame* view;

    /**
     * 当前正在进行中的会议名称
     * - 会议开始，设置会议号
     * - 会议结束，清空会议号。
     */
    QString currentMeetingName;
    QMutex mutex;

public slots:
    void doStart();
};

}  // namespace module::meet
