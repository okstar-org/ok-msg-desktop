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

#ifndef CALLDURATTIONFORM_H
#define CALLDURATTIONFORM_H

#include <QElapsedTimer>
#include <QLabel>
#include <QWidget>

#include <src/model/contact.h>
#include "src/video/genericnetcamview.h"

namespace Ui {
class CallDurationForm;
}
namespace module::im {

/**
 * 音视频聊天对话框
 */
class CallDurationForm : public QWidget {
    Q_OBJECT

public:
    explicit CallDurationForm(QWidget* parent = nullptr);
    ~CallDurationForm();
    void setContact(const Contact* c);
    void reloadTheme();

    GenericNetCamView* createNetcam();
    void showNetcam();
    void hideNetcam();
    void showAvatar();

    /**
     * 启动计时
     */
    void startCounter();

    /**
     * 停止计时
     */
    void stopCounter();

signals:
    void endCall();
    void muteMicrophone(bool);
    void muteSpeaker(bool);

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    Ui::CallDurationForm* ui;
    const Contact* contact;
    QTimer* callDurationTimer;
    QElapsedTimer* timeElapsed;

    bool muteOut;
    bool muteIn;
    GenericNetCamView* netcam;

private slots:
    void onUpdateTime();
    void onCallEnd();
    // 禁止麦克风
    void onMuteOut();
    // 禁止扬声器
    void onMuteIn();
};
}  // namespace module::im
#endif  // CALLDURATTIONFORM_H
