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

#include "OMediaConfigWidget.h"

#include "application.h"
#include "lib/ui/widget/tools/RoundedPixmapLabel.h"
#include "lib/ui/widget/tools/PopupMenuComboBox.h"
#include "AudioVolumnSlider.h"
#include "lib/video/cameradevice.h"
#include <QMenu>
#include <QPushButton>
#include <QSlider>
#include <QStackedLayout>
#include <QToolButton>

namespace lib::ui {
OMediaConfigWidget::OMediaConfigWidget(QWidget* parent) : QWidget{parent} {
    auto profile = ok::Application::Instance()->getProfile();

    avatarLabel = new lib::ui::RoundedPixmapLabel(this);
    avatarLabel->setObjectName("avatarLabel");
    avatarLabel->setAttribute(Qt::WA_StyledBackground);
    avatarLabel->setContentsSize(QSize(120, 120));
    avatarLabel->setImage(profile->getAvatar());

    micSpeakSetting = new lib::ui::PopupMenuComboBox(this);
    micSpeakSetting->setLabel(tr("Micphone"));
    micSpeakSetting->setCursor(Qt::PointingHandCursor);
    connect(micSpeakSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableMic = !ctrlState.enableMic;
        updateAudioVideoIcon(true, false, false);
        if(ctrlState.enableMic){
            doOpenAudio();
        }else{
            doCloseAudio();
        }
    });

    cameraSetting = new lib::ui::PopupMenuComboBox(this);
    cameraSetting->setLabel(tr("Camera"));
    cameraSetting->setCursor(Qt::PointingHandCursor);

    connect(cameraSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableCam = !ctrlState.enableCam;
        updateAudioVideoIcon(false, true, false);
        if (ctrlState.enableCam) {
            doOpenVideo();
        } else {
            doCloseVideo();
        }
    });

    volumnSetting = new lib::ui::PopupMenuComboBox(this);
    volumnSetting->iconButton()->setCursor(Qt::PointingHandCursor);
    connect(volumnSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableSpk = !ctrlState.enableSpk;
        updateAudioVideoIcon(false, false, true);
    });

    volumnSlider = new AudioVolumnSlider(volumnSetting);
    volumnSlider->setRange(0, 100);
    volumnSetting->setWidget(volumnSlider);

    buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(10);

    auto* footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(10);
    footerLayout->addWidget(micSpeakSetting);
    footerLayout->addWidget(cameraSetting);
    footerLayout->addWidget(volumnSetting);
    footerLayout->addStretch(1);
    footerLayout->addLayout(buttonLayout);

    cameraOutput = new OVideoOutputWidget(this);
    videoOutLayout = new QStackedLayout();
    videoOutLayout->setContentsMargins(0, 0, 0, 0);
    videoOutLayout->addWidget(avatarLabel);
    videoOutLayout->addWidget(cameraOutput);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(videoOutLayout, 1);
    mainLayout->addLayout(footerLayout);

    // 初始化设备控件
    audioMenu = new QMenu(this);
    micSpeakSetting->setMenu(audioMenu);

    aGroup = new QActionGroup(this);
    aGroup->setExclusive(true);
    connect(aGroup, &QActionGroup::triggered, this, &OMediaConfigWidget::audioSelected);

    videoMenu = new QMenu(this);
    cameraSetting->setMenu(videoMenu);
    vGroup = new QActionGroup(this);
    vGroup->setExclusive(true);
    connect(vGroup, &QActionGroup::triggered, this, &OMediaConfigWidget::videoSelected);

    ctrlState = {false, false, true};
    updateAudioVideoIcon(true, true, true);

    QTimer::singleShot(100, this, [this]() {
        // 延迟执行获取设备信息，避免界面延迟！
        this->initDeviceInfo();
    });
}



void OMediaConfigWidget::addFooterButton(QPushButton *button)
{
    buttonLayout->addWidget(button);
}

/**
 * 获取设备信息
 */
void OMediaConfigWidget::initDeviceInfo() {
    QMutexLocker locker(&mutex);

    // 保持QAction只有QMenu作为parent，没有添加到其他QWidget，clear会自动释放
    audioMenu->clear();

    aDeviceList = ok::Application::Instance()->getAudioControl()->inDeviceNames();
    QSet<QString> item_set;
    for (auto& a : aDeviceList) {
        if (item_set.contains(a)) {
            continue;
        }
        item_set.insert(a);
        auto act = new QAction(a, audioMenu);
        act->setCheckable(true);
        // 如果存在以选择音频设备，则勾选当前的
        if (act->text() == selectedAudio) {
            act->setChecked(true);
        }
        audioMenu->addAction(act);
        aGroup->addAction(act);
    }
    if (!audioMenu->isEmpty()) {
        auto f = audioMenu->actions().first();
        f->setChecked(true);
        selectedAudio = f->text();
    }


    videoMenu->clear();
    vDeviceList = lib::video::CameraDevice::getDeviceList();
    for (auto& a : vDeviceList) {
        auto act = new QAction(a.name, videoMenu);
        act->setData(a.name);
        act->setCheckable(true);
        act->setProperty("type", QVariant::fromValue<lib::video::VideoType>(a.type));
        // 如果存在以选择视频设备，则勾选当前的
        if (act->text() == selectedVideo) {
            act->setChecked(true);
        }
        videoMenu->addAction(act);
        vGroup->addAction(act);
    }


    if (!videoMenu->isEmpty()) {
        auto f = videoMenu->actions().first();
        f->setChecked(true);
        selectedVideo = f->text();
        selectedVideoType = f->property("type").value<lib::video::VideoType>();
    }
}

void OMediaConfigWidget::updateAudioVideoIcon(bool audio, bool video, bool speaker) {
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
        } else {
            cameraSetting->iconButton()->setIcon(QIcon(":/meet/image/videocam_stop.svg"));
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


void OMediaConfigWidget::audioSelected(QAction* action) {
    QMutexLocker locker(&mutex);

    if (action->isChecked()) {
        selectedAudio = action->text();
    }

    if(ctrlState.enableMic){
        doOpenAudio();
    }else{
        doCloseAudio();
    }
}

void OMediaConfigWidget::videoSelected(QAction* action) {
    QMutexLocker locker(&mutex);

    if (action->isChecked()) {
        auto newSelectedVideo = action->text();
        if(selectedVideo == newSelectedVideo){
            return;
        }

        if (!selectedVideo.isEmpty()) {
            doCloseVideo();
        }
        selectedVideo = newSelectedVideo;
    }

    if (ctrlState.enableCam) {
        doOpenVideo();
    } else {
        doCloseVideo();
    }
}

void OMediaConfigWidget::doOpenVideo() {
    qDebug() << __func__;

    QAction* action = vGroup->checkedAction();
    if (!action) {
        videoOutLayout->setCurrentWidget(avatarLabel);
        return;
    }
    auto name = action->text();
    qDebug() << "select video device:" << name;

    auto it = std::find_if(vDeviceList.begin(), vDeviceList.end(),
                           [&](auto& v) { return v.name == name; });

    if (it == vDeviceList.end()) {
        return;
    }

    auto& dev = vDeviceList.at(std::distance(vDeviceList.begin(), it));
    selectedVideo = dev.name;
    cameraOutput->render(dev);
    videoOutLayout->setCurrentWidget(cameraOutput);
}

void OMediaConfigWidget::doCloseVideo() {
    qDebug() << __func__;
    videoOutLayout->setCurrentWidget(avatarLabel);
    cameraOutput->stopRender();
}

void OMediaConfigWidget::closeVideo()
{
    ctrlState.enableCam=false;
    updateAudioVideoIcon(false, true, false);
    doCloseVideo();
}

void OMediaConfigWidget::doOpenAudio()
{
    QMutexLocker locker(&mutex);

    auto ac = ok::Application::Instance()->getAudioControl();
    if(!selectedAudio.isEmpty()){
        ac->reinitInput(selectedAudio);
    }

    audioSink = ac->makeSink();

    audioSource = ac->makeSource();
    connect(audioSource.get(), &lib::audio::IAudioSource::frameAvailable, this,
            [&,ac](const int16_t* pcm, size_t samples, uint8_t chans, uint32_t rate) {
                // 音频帧 pcm
                auto v = ac->getInputVol(pcm, samples);
                // 显示音频跳动
                volumnSlider->setRealVolume(v*100);
            });
}

void OMediaConfigWidget::doCloseAudio()
{
    QMutexLocker locker(&mutex);
    audioSink.reset();
    audioSource.reset();
}

void OMediaConfigWidget::closeAudio()
{
    updateAudioVideoIcon(true, false, false);
    doCloseAudio();
}

const lib::ortc::DeviceConfig OMediaConfigWidget::getConf() {

    auto a = lib::ortc::DeviceConfig{.audioName = selectedAudio.toStdString(),
                                     .videoName = selectedVideo.toStdString()};
    switch (selectedVideoType) {
        case lib::video::VideoType::Camera:
            a.videoType = lib::ortc::VideoType::Camera;
        case lib::video::VideoType::Desktop:
            a.videoType = lib::ortc::VideoType::Desktop;
        case video::VideoType::File:
        case video::VideoType::Stream:
            break;
    }
    return a;
}
}  // namespace lib::ui
