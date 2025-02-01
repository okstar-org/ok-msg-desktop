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
#include <QStackedLayout>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVector>

#include "base/RoundedPixmapLabel.h"
#include "lib/audio/backend/openal.h"
#include "lib/ui/widget/tools/PopupMenuComboBox.h"
#include "lib/video/cameradevice.h"
#include "lib/video/camerasource.h"
#include "lib/video/videoframe.h"
#include "src/application.h"

namespace module::meet {

class CameraVideoOutputWidget : public QWidget {
public:
    explicit CameraVideoOutputWidget(QWidget* parent) : QWidget(parent) {}

    void render(const lib::video::VideoDevice& device);
    void stopRender();

protected:
    void paintEvent(QPaintEvent* event) override;
uint64_t count = 0;
private:
    std::unique_ptr<lib::video::CameraSource> _camera;
    std::shared_ptr<lib::video::VideoFrame> lastFrame;
    bool is_stop = false;
};

MeetingOptionWidget::MeetingOptionWidget(QWidget* parent)
        : QWidget(parent) {
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
        if(ctrlState.enableCam){
            doOpenVideo();
        }else{
            doCloseVideo();
        }
    });

    volumnSetting = new lib::ui::PopupMenuComboBox(this);
    // volumnSetting->iconButton()->setIcon(QIcon(":/meet/image/speaker.svg"));
    volumnSetting->iconButton()->setCursor(Qt::PointingHandCursor);
    connect(volumnSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableSpk = !ctrlState.enableSpk;
        updateAudioVideoIcon(false, false, true);
    });

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

    cameraOutput = new CameraVideoOutputWidget(this);
    videoOutLayout = new QStackedLayout();
    videoOutLayout->setContentsMargins(0, 0, 0, 0);
    videoOutLayout->addWidget(avatarLabel);
    videoOutLayout->addWidget(cameraOutput);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(videoOutLayout, 1);
    mainLayout->addLayout(footerLayout);

            // 初始化设备控件
    audioMenu = new QMenu(this);
    aGroup = new QActionGroup(this);
    aGroup->setExclusive(true);
    micSpeakSetting->setMenu(audioMenu);
    connect(aGroup, &QActionGroup::triggered, this, &MeetingOptionWidget::audioSelected);

    videoMenu = new QMenu(this);
    vGroup = new QActionGroup(this);
    vGroup->setExclusive(true);

    cameraSetting->setMenu(videoMenu);
    connect(vGroup, &QActionGroup::triggered, this, &MeetingOptionWidget::videoSelected);

    ctrlState = {true, false, true};
    updateAudioVideoIcon(true, true, true);

    QTimer::singleShot(100, this, [this]() {
        // 延迟执行获取设备信息，避免界面延迟！
        this->initDeviceInfo();
    });

}

void MeetingOptionWidget::addFooterButton(QPushButton* button) {
    buttonLayout->addWidget(button);
}

void MeetingOptionWidget::retranslateUi() {
    micSpeakSetting->setLabel(tr("Micphone"));
    cameraSetting->setLabel(tr("Camera"));
}

void MeetingOptionWidget::showEvent(QShowEvent* event) {




}

void MeetingOptionWidget::hideEvent(QHideEvent* e) {
    updateAudioVideoIcon(true, true, true);
    // doCloseVideo();
}

/**
 * 获取设备信息
 */
void MeetingOptionWidget::initDeviceInfo() {
    QMutexLocker locker(&mutex);

            // 保持QAction只有QMenu作为parent，没有添加到其他QWidget，clear会自动释放
    audioMenu->clear();
    auto oa = std::make_unique<lib::audio::OpenAL>();
    auto alist = oa->inDeviceNames();
    QSet<QString> item_set;
    for (auto& a : alist) {
        qDebug() << "audio device:" << a;
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

    videoMenu->clear();
    //    item_set.clear();
    vDeviceList = lib::video::CameraDevice::getDeviceList();
    for (auto& a : vDeviceList) {
        qDebug() << "video device:" << a.name;
        auto act = new QAction(a.name, videoMenu);
        act->setData(a.name);
        act->setCheckable(true);
        // 如果存在以选择视频设备，则勾选当前的
        if (act->text() == selectedVideo) {
            act->setChecked(true);
        }
        videoMenu->addAction(act);
        vGroup->addAction(act);
    }

            // todo: 应该存在什么缓存机制，保存用户选择的视频，初始化选择
    if (!audioMenu->isEmpty()) {
        audioMenu->actions().first()->setChecked(true);
    }
    if (!videoMenu->isEmpty()) {
        videoMenu->actions().first()->setChecked(true);
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

void MeetingOptionWidget::audioSelected(QAction* action) {
    QMutexLocker locker(&mutex);

    if (action->isChecked()) {
        selectedAudio = action->text();
    }
}

void MeetingOptionWidget::videoSelected(QAction* action) {

    QMutexLocker locker(&mutex);
    if (action->isChecked()) {
        selectedVideo = action->text();
    }

    if (ctrlState.enableCam)
    {
        doOpenVideo();
    }else{
        doCloseVideo();
    }
}

void MeetingOptionWidget::doOpenVideo() {
    qDebug() << __func__;

    QAction* action = vGroup->checkedAction();
    if (!action) {
        videoOutLayout->setCurrentWidget(avatarLabel);
        return;
    }
    auto name = action->text();
    qDebug() << "select video device:" << name;

    auto it = std::find_if(vDeviceList.begin(), vDeviceList.end(), [&](auto& v) {
        return v.name == name;
    });

    if(it == vDeviceList.end()){
        return;
    }

    auto dev = vDeviceList.at(std::distance(vDeviceList.begin(), it));
    cameraOutput->render(dev);
    videoOutLayout->setCurrentWidget(cameraOutput);

}

void MeetingOptionWidget::doCloseVideo() {
    qDebug() << __func__;
    videoOutLayout->setCurrentWidget(avatarLabel);
    cameraOutput->stopRender();
}

void CameraVideoOutputWidget::render(const lib::video::VideoDevice& device) {
    is_stop = false;
    if(!_camera){
        _camera = lib::video::CameraSource::CreateInstance(device);
        auto modes = _camera->getVideoModes();
        for(auto &m : modes){
            qDebug() << "mode" << m.toString().c_str();
        }

        connect(_camera.get(), &lib::video::CameraSource::frameAvailable, this,
                [this](std::shared_ptr<lib::video::VideoFrame> frame) {
                    lastFrame = std::move(frame);
                    update();
                });
        connect(_camera.get(), &lib::video::CameraSource::sourceStopped, this, [&](){
            //Avoid dirty data affecting the next frame rendering
            lastFrame.reset();
            //destroy camera
            _camera.reset();
        });
    }
    // todo: 既然是单例，怎么保证多个订阅方选不同设备
    // lib::video::VideoMode mode{200, 400, 0, 0, 30};
    _camera->openDevice();
    // _camera->subscribe();
}

void CameraVideoOutputWidget::stopRender() {
    is_stop = true;
    if (_camera) {

        disconnect(_camera.get());
        // todo: 既然上面获取_camera是单例，其他如果用到了，是否存在释放问题，这里暂不释放
        // todo: 而且涉及到摄像头的分辨率等问题，看怎么设计的
        _camera->unsubscribe();
        _camera->closeDevice();

        // lastFrame.reset();
        update();
    }
}

void CameraVideoOutputWidget::paintEvent(QPaintEvent* event) {
    auto boundingRect = this->rect();
    QPainter painter(this);
    painter.fillRect(painter.viewport(), Qt::black);

    if (lastFrame) {

        auto img = lastFrame->toQImage(rect().size());
        if (!img.isNull()) {
            painter.drawImage(boundingRect, img, img.rect(), Qt::NoFormatConversion);
        }

        // return;
        // avatarLabel->setVisible(false);
    }
}

}  // namespace module::meet
