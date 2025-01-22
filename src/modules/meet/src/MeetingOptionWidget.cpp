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

#include "MeetingOptionWidget.h"

#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

#include "base/RoundedPixmapLabel.h"
#include "lib/audio/backend/openal.h"
#include "lib/ui/widget/tools/PopupMenuComboBox.h"
#include "lib/video/cameradevice.h"
#include "lib/video/camerasource.h"
#include "src/application.h"

namespace module::meet {

MeetingOptionWidget::MeetingOptionWidget(QWidget* parent)
        : QWidget(parent), ctrlState{true, false, true}, camera(nullptr) {
    auto profile = ok::Application::Instance()->getProfile();

    avatarLabel = new RoundedPixmapLabel(this);
    avatarLabel->setObjectName("avatarLabel");
    avatarLabel->setAttribute(Qt::WA_StyledBackground);
    avatarLabel->setContentsSize(QSize(120, 120));
    avatarLabel->setImage(profile->getAvatar());

    micSpeakSetting = new lib::ui::PopupMenuComboBox(this);
    // micSpeakSetting->iconButton()->setIcon(QIcon(":/meet/image/micphone.svg"));
    micSpeakSetting->setLabel(tr("Micphone"));
    micSpeakSetting->setCursor(Qt::PointingHandCursor);
    connect(micSpeakSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableMic = !ctrlState.enableMic;
        updateAudioVideoIcon(true, false, false);
    });

    cameraSetting = new lib::ui::PopupMenuComboBox(this);
    // cameraSetting->iconButton()->setIcon(QIcon(":/meet/image/videocam.svg"));
    cameraSetting->setLabel(tr("Camera"));
    cameraSetting->setCursor(Qt::PointingHandCursor);

    connect(cameraSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableCam = !ctrlState.enableCam;
        updateAudioVideoIcon(false, true, false);
    });

    volumnSetting = new lib::ui::PopupMenuComboBox(this);
    // volumnSetting->iconButton()->setIcon(QIcon(":/meet/image/speaker.svg"));
    volumnSetting->iconButton()->setCursor(Qt::PointingHandCursor);
    connect(volumnSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableSpk = !ctrlState.enableSpk;
        updateAudioVideoIcon(false, false, true);
    });

    // 默认关闭视频
    updateAudioVideoIcon(true, true, true);

    volumnSlider = new QSlider(Qt::Horizontal, volumnSetting);
    volumnSlider->setRange(0, 100);
    volumnSetting->setWidget(volumnSlider);

    buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(10);

    QHBoxLayout* footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(10);
    footerLayout->addWidget(micSpeakSetting);
    footerLayout->addWidget(cameraSetting);
    footerLayout->addWidget(volumnSetting);
    footerLayout->addStretch(1);
    footerLayout->addLayout(buttonLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(avatarLabel, 1);
    mainLayout->addLayout(footerLayout);

    // 初始化设备控件
    audioMenu = new QMenu(this);
    aGroup = new QActionGroup(this);
    aGroup->setExclusive(true);
    micSpeakSetting->setMenu(audioMenu);
    connect(audioMenu, &QMenu::triggered, this, &MeetingOptionWidget::audioSelected);

    videoMenu = new QMenu(this);
    vGroup = new QActionGroup(this);
    vGroup->setExclusive(true);

    cameraSetting->setMenu(videoMenu);
    connect(videoMenu, &QMenu::triggered, this, &MeetingOptionWidget::videoSelected);
}

void MeetingOptionWidget::addFooterButton(QPushButton* button) {
    buttonLayout->addWidget(button);
}

void MeetingOptionWidget::retranslateUi() {
    micSpeakSetting->setLabel(tr("Micphone"));
    cameraSetting->setLabel(tr("Camera"));
}

void MeetingOptionWidget::showEvent(QShowEvent* event) {
    QTimer::singleShot(100, this, [this]() {
        // 延迟执行获取设备信息
        this->initDeviceInfo();
    });
}

void MeetingOptionWidget::paintEvent(QPaintEvent* event) {
    if (!lastFrame) return;

    auto boundingRect = avatarLabel->rect();

    QPainter painter(this);
    painter.fillRect(painter.viewport(), Qt::black);
    if (lastFrame) {
        QImage frame = lastFrame->toQImage(rect().size());
        if (frame.isNull()) {
            lastFrame.reset();
            return;
        }
        painter.drawImage(boundingRect, frame, frame.rect(), Qt::NoFormatConversion);
        // avatarLabel->setVisible(false);
    }
}

/**
 * 获取设备信息
 */
void MeetingOptionWidget::initDeviceInfo() {
    for (auto a : audioMenu->actions()) {
        aGroup->removeAction(a);
        audioMenu->removeAction(a);
    }

    // audioMenu->clear();

    auto oa = std::make_unique<lib::audio::OpenAL>();
    auto alist = oa->inDeviceNames();
    for (auto& a : alist) {
        qDebug() << "audio device:" << a;
        for (auto act0 : audioMenu->actions()) {
            if (act0->text() == a) {
                continue;
            }
        }

        auto act = new QAction(a, this);
        act->setCheckable(true);
        audioMenu->addAction(act);
        aGroup->addAction(act);
    }

    for (auto a : videoMenu->actions()) {
        vGroup->removeAction(a);
        videoMenu->removeAction(a);
    }

    auto vlist = lib::video::CameraDevice::getDeviceList();
    for (auto& a : vlist) {
        if (a.first == "none") continue;
        qDebug() << "video device:" << a;
        for (auto act0 : videoMenu->actions()) {
            if (act0->text() == a.second) {
                continue;
            }
        }

        auto act = new QAction(a.second, this);
        act->setData(a.first);
        act->setCheckable(true);
        videoMenu->addAction(act);
        vGroup->addAction(act);
    }
}

void MeetingOptionWidget::updateAudioVideoIcon(bool audio, bool video, bool speaker) {
    if (audio) {
        if (ctrlState.enableMic) {
            micSpeakSetting->iconButton()->setIcon(QIcon(":/meet/image/micphone.svg"));
        } else {
            micSpeakSetting->iconButton()->setIcon(QIcon(":/meet/image/micphone_mute.svg"));
        }
    }
    if (video) {
        if (ctrlState.enableCam) {
            cameraSetting->iconButton()->setIcon(QIcon(":/meet/image/videocam.svg"));
            doOpenVideo();
        } else {
            cameraSetting->iconButton()->setIcon(QIcon(":/meet/image/videocam_stop.svg"));
            doCloseVideo();
        }
    }

    if (speaker) {
        if (ctrlState.enableSpk) {
            volumnSetting->iconButton()->setIcon(QIcon(":/meet/image/speaker.svg"));
        } else {
            volumnSetting->iconButton()->setIcon(QIcon(":/meet/image/speaker_stop.svg"));
        }
    }
}

void MeetingOptionWidget::audioSelected(QAction* action) {
    if (action->isChecked()) {
        selectedAudio = action;
    } else {
        selectedAudio = nullptr;
    }
}

void MeetingOptionWidget::videoSelected(QAction* action) {
    if (action->isChecked()) {
        selectedVideo = action;
    } else {
        selectedVideo = nullptr;
    }
}

void MeetingOptionWidget::doOpenVideo() {
    if (!selectedVideo) return;

    qDebug() << "select video device:" << selectedVideo->text();
    lib::video::VideoMode mode{200, 400, 0, 0, 30};
    camera = lib::video::CameraSource::getInstance();
    connect(camera, &lib::video::CameraSource::frameAvailable, this,
            [&](std::shared_ptr<lib::video::VideoFrame> frame) {
                lastFrame = std::move(frame);
                update();
            });
    camera->setupDevice(selectedVideo->data().toString(), mode);
    camera->subscribe();
}

void MeetingOptionWidget::doCloseVideo() {
    if (camera) {
        camera->unsubscribe();
        camera->destroyInstance();
        camera = nullptr;
    }
}

}  // namespace module::meet
