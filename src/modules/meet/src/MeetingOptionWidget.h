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

#include <QFrame>
#include <QMenu>
#include <QPointer>


#include "Defines.h"
#include "base/compatiblerecursivemutex.h"
#include "lib/video/videomode.h"
#include "lib/ortc/ok_rtc.h"

class QPushButton;
class QSlider;
class QHBoxLayout;
class QStackedLayout;

class RoundedPixmapLabel;

namespace lib::ui {
class PopupMenuComboBox;
}

namespace lib::video {
class CameraSource;
class VideoFrame;
}  // namespace lib::video

namespace module::meet {
class CameraVideoOutputWidget;
/**
 * 选项配置控件
 */
class MeetingOptionWidget : public QWidget {
    Q_OBJECT

public:
    explicit MeetingOptionWidget(QWidget* parent = nullptr);
    void addFooterButton(QPushButton* button);

    void retranslateUi();

    [[nodiscard]] inline const lib::ortc::CtrlState& getCtrlState() const {
        return ctrlState;
    }

signals:
    // 状态改变事件
    void stateChanged();

public slots:
    void audioSelected(QAction* action);
    void videoSelected(QAction* action);

    void doOpenVideo();
    void doCloseVideo();

protected:
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

private:
    void initDeviceInfo();
    void updateAudioVideoIcon(bool audio, bool video, bool spk);

    RoundedPixmapLabel* avatarLabel = nullptr;

    lib::ui::PopupMenuComboBox* micSpeakSetting = nullptr;
    lib::ui::PopupMenuComboBox* cameraSetting = nullptr;
    lib::ui::PopupMenuComboBox* volumnSetting = nullptr;

    QSlider* volumnSlider = nullptr;

    QHBoxLayout* buttonLayout = nullptr;

    lib::ortc::CtrlState ctrlState;

    // 音频设备
    QMenu* audioMenu = nullptr;
    QActionGroup* aGroup = nullptr;
    QString selectedAudio;

    // 视频设备
    QMenu* videoMenu = nullptr;
    QActionGroup* vGroup = nullptr;
    QString selectedVideo;
    QVector<lib::video::VideoDevice> vDeviceList;

    QStackedLayout* videoOutLayout = nullptr;
    CameraVideoOutputWidget* cameraOutput = nullptr;
    CompatibleRecursiveMutex mutex;

};

}  // namespace module::meet
