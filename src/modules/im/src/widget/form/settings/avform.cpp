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

#include "avform.h"

#include <cassert>
#include <map>

#include <QDesktopWidget>
#include <QScreen>
#include <QShowEvent>
#include <QVector>

#include "lib/audio/audio.h"
#include "lib/audio/iaudiosettings.h"
#include "lib/audio/iaudiosource.h"
#include "lib/storage/settings/translator.h"
#include "src/base/RecursiveSignalBlocker.h"

#include "lib/video/cameradevice.h"
#include "lib/video/camerasource.h"
#include "lib/video/ivideosettings.h"
#include "lib/video/videomode.h"
#include "src/video/videosurface.h"
#include "src/Bus.h"
#include "src/application.h"
#include "src/core/coreav.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/widget/tool/screenshotgrabber.h"

#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER ALC_DEVICE_SPECIFIER
#endif
namespace module::im {

AVForm::AVForm()
        : GenericForm(QPixmap(":/img/settings/av.png"))
        , audioSettings(nullptr)
        , videoSettings(nullptr)
        , camVideoSurface(nullptr) {
    setupUi(this);

    // block all child signals during initialization
    const ok::base::RecursiveSignalBlocker signalBlocker(this);

    eventsInit();

    QDesktopWidget* desktop = QApplication::desktop();
    for (QScreen* qScreen : QGuiApplication::screens()) {
        connect(qScreen, &QScreen::geometryChanged, this, &AVForm::rescanDevices);
    }

    auto s = &lib::settings::OkSettings::getInstance();
    audioSettings = s;
    videoSettings = s;
    audio = std::unique_ptr<lib::audio::IAudioControl>(lib::audio::Audio::makeAudio(*s));

    cbEnableTestSound->setChecked(s->getEnableTestSound());
    cbEnableTestSound->setToolTip(tr("Play a test sound while changing the output volume."));

    connect(rescanButton, &QPushButton::clicked, this, &AVForm::rescanDevices);

    playbackSlider->setTracking(false);
    playbackSlider->setMaximum(lib::audio::Audio::totalSteps);
    playbackSlider->setValue(
            getStepsFromValue(s->getOutVolume(), s->getOutVolumeMin(), s->getOutVolumeMax()));
    playbackSlider->installEventFilter(this);

    // audio settings
    microphoneSlider->setToolTip(tr("Use slider to set the gain of your input device ranging"
                                    " from %1dB to %2dB.")
                                         .arg(audio->minInputGain())
                                         .arg(audio->maxInputGain()));
    microphoneSlider->setMaximum(lib::audio::Audio::totalSteps);
    microphoneSlider->setTickPosition(QSlider::TicksBothSides);
    static const int numTicks = 4;
    microphoneSlider->setTickInterval(lib::audio::Audio::totalSteps / numTicks);
    microphoneSlider->setTracking(false);
    microphoneSlider->installEventFilter(this);

    microphoneSlider->setValue(
            getStepsFromValue(audio->inputGain(), audio->minInputGain(), audio->maxInputGain()));

    audioThresholdSlider->setToolTip(
            tr("Use slider to set the activation volume for your input device."));
    audioThresholdSlider->setMaximum(lib::audio::Audio::totalSteps);
    audioThresholdSlider->setValue(getStepsFromValue(audioSettings->getAudioThreshold(),
                                                     audio->minInputThreshold(),
                                                     audio->maxInputThreshold()));
    audioThresholdSlider->setTracking(false);
    audioThresholdSlider->installEventFilter(this);

    volumeDisplay->setMaximum(lib::audio::Audio::totalSteps);

    fillAudioQualityComboBox();

    auto* qGUIApp = qobject_cast<QGuiApplication*>(qApp);
    assert(qGUIApp);
    connect(qGUIApp, &QGuiApplication::screenAdded, this, &AVForm::trackNewScreenGeometry);
    connect(qGUIApp, &QGuiApplication::screenAdded, this, &AVForm::rescanDevices);
    connect(qGUIApp, &QGuiApplication::screenRemoved, this, &AVForm::rescanDevices);

    auto a = ok::Application::Instance();
    connect(a->bus(), &ok::Bus::languageChanged,this,
            [&](const QString& locale0) {
                retranslateUi();
            });
    connect(a->bus(), &ok::Bus::profileChanged, this, &AVForm::onProfileChanged);
}

AVForm::~AVForm() {
    
}

void AVForm::hideEvent(QHideEvent* event) {
    audioSink.reset();
    audioSrc.reset();

    if (camVideoSurface) {
        camVideoSurface->setSource(nullptr);
        camVideoSurface.reset();
        QLayoutItem* child;
        while ((child = gridLayout->takeAt(0)) != nullptr) delete child;
    }

    videoDeviceList.clear();
    camera.reset();
    GenericForm::hideEvent(event);
}

void AVForm::showEvent(QShowEvent* event) {


    getAudioOutDevices();
    getAudioInDevices();
    if (audioSrc == nullptr) {
        audioSrc = audio->makeSource();
        connect(audioSrc.get(), &lib::audio::IAudioSource::volumeAvailable, this,
                &AVForm::setVolume);
    }

    if (audioSink == nullptr) {
        audioSink = audio->makeSink();
    }

    auto source = initVideoDevices();
    auto surface = createVideoSurface(source);
    gridLayout->addWidget(surface, 0, 0, 1, 1);


    GenericForm::showEvent(event);
}

void AVForm::open(const lib::video::VideoMode& mode) {
    // qDebug() << __func__ << qstring(mode.toString());

    QRect rect = mode.toRect();
    videoSettings->setCamVideoRes(rect);
    videoSettings->setCamVideoFPS(static_cast<float>(mode.FPS));

    camera->setup(mode);
    camera->openDevice();
    if(camVideoSurface)
        camVideoSurface->setSource(camera.get());
}

void AVForm::trackNewScreenGeometry(QScreen* qScreen) {
    connect(qScreen, &QScreen::geometryChanged, this, &AVForm::rescanDevices);
}

void AVForm::onProfileChanged(Profile* profile) {}

void AVForm::rescanDevices() {
    getAudioInDevices();
    getAudioOutDevices();
    initVideoDevices();
}

void AVForm::setVolume(float value) {
    volumeDisplay->setValue(
            getStepsFromValue(value, audio->minOutputVolume(), audio->maxOutputVolume()));
}

void AVForm::on_videoModescomboBox_currentIndexChanged(int index) {
    assert(0 <= index && index < videoModes.size());
    int devIndex = videoDevCombobox->currentIndex();
    assert(0 <= devIndex && devIndex < videoDeviceList.size());

    QString devName = videoDeviceList[devIndex].name;
    auto mode = videoModes[index];
    if (lib::video::CameraDevice::isScreen(devName) && mode == lib::video::VideoMode()) {
        if (videoSettings->getScreenGrabbed()) {
            lib::video::VideoMode mode(videoSettings->getScreenRegion());
            open(mode);
            return;
        }

        auto onGrabbed = [devName, this](QRect region) {
            lib::video::VideoMode mode(region);
            mode.width = mode.width / 2 * 2;
            mode.height = mode.height / 2 * 2;
            // Needed, if the virtual screen origin is the top left corner of the primary screen
            QRect screen = QApplication::primaryScreen()->virtualGeometry();
            mode.x += screen.x();
            mode.y += screen.y();

            videoSettings->setScreenRegion(mode.toRect());
            videoSettings->setScreenGrabbed(true);

            open(mode);
        };

        // note: grabber is self-managed and will destroy itself when done
        ScreenshotGrabber* screenshotGrabber = new ScreenshotGrabber;

        connect(screenshotGrabber, &ScreenshotGrabber::regionChosen, this, onGrabbed,
                Qt::QueuedConnection);
        screenshotGrabber->showGrabber();
        return;
    }

    videoSettings->setScreenGrabbed(false);
    open(mode);
}

void AVForm::selectBestModes(QVector<lib::video::VideoMode>& allVideoModes) {
    if (allVideoModes.isEmpty()) {
        qCritical() << "Trying to select best mode from empty modes list";
        return;
    }

    // Identify the best resolutions available for the supposed XXXXp resolutions.
    std::map<int, lib::video::VideoMode> idealModes;
    idealModes[120] = lib::video::VideoMode(160, 120);
    idealModes[240] = lib::video::VideoMode(430, 240);
    idealModes[360] = lib::video::VideoMode(640, 360);
    idealModes[480] = lib::video::VideoMode(854, 480);
    idealModes[720] = lib::video::VideoMode(1280, 720);
    idealModes[1080] = lib::video::VideoMode(1920, 1080);
    idealModes[1440] = lib::video::VideoMode(2560, 1440);
    idealModes[2160] = lib::video::VideoMode(3840, 2160);
    idealModes[1920] = lib::video::VideoMode(1080, 1920);

    std::map<int, int> bestModeInds;
    for (int i = 0; i < allVideoModes.size(); ++i) {
        lib::video::VideoMode mode = allVideoModes[i];

        // PS3-Cam protection, everything above 60fps makes no sense
        if (mode.FPS > 60) continue;

        for (auto iter = idealModes.begin(); iter != idealModes.end(); ++iter) {
            int res = iter->first;
            lib::video::VideoMode idealMode = iter->second;
            // don't take approximately correct resolutions unless they really
            // are close
            qDebug("norm:%d tolerance:%d", mode.norm(idealMode), idealMode.tolerance());
            if (mode.norm(idealMode) > idealMode.tolerance()) continue;

            if (bestModeInds.find(res) == bestModeInds.end()) {
                bestModeInds[res] = i;
                continue;
            }

            int index = bestModeInds[res];
            lib::video::VideoMode best = allVideoModes[index];
            if (mode.norm(idealMode) < best.norm(idealMode)) {
                bestModeInds[res] = i;
                continue;
            }

            if (mode.norm(idealMode) == best.norm(idealMode)) {
                // prefer higher FPS and "better" pixel formats
                if (mode.FPS > best.FPS) {
                    bestModeInds[res] = i;
                    continue;
                }

                bool better = lib::video::CameraDevice::betterPixelFormat(mode.pixel_format,
                                                                          best.pixel_format);
                if (mode.FPS >= best.FPS && better) bestModeInds[res] = i;
            }
        }
    }

    QVector<lib::video::VideoMode> newVideoModes;
    for (auto it = bestModeInds.rbegin(); it != bestModeInds.rend(); ++it) {
        lib::video::VideoMode mode = allVideoModes[it->second];

        if (newVideoModes.empty()) {
            newVideoModes.push_back(mode);
        } else {
            int size = getModeSize(mode);
            auto result =
                    std::find_if(newVideoModes.cbegin(), newVideoModes.cend(),
                    [size](lib::video::VideoMode mode) { return getModeSize(mode) == size; });

            if (result == newVideoModes.end()) newVideoModes.push_back(mode);
        }
    }
    allVideoModes = newVideoModes;
}

void AVForm::fillCameraModesComboBox( const QVector<lib::video::VideoMode> &modes) {
    qDebug() << __func__ ;

    videoModes = modes;

    bool previouslyBlocked = videoModescomboBox->blockSignals(true);
    videoModescomboBox->clear();

    for (int i = 0; i < videoModes.size(); ++i) {
        auto mode = videoModes[i];
        // qDebug() << mode.toString().c_str();
        QString str;
        if (mode.height && mode.width) {
            str += QString("%1x%2 FPS:%3").arg(mode.width).arg(mode.height).arg(mode.FPS);
        } else {
            str += tr("Default resolution");
        }

        videoModescomboBox->addItem(str);
    }

    if (videoModes.isEmpty()) videoModescomboBox->addItem(tr("Default resolution"));

    videoModescomboBox->blockSignals(previouslyBlocked);
}


void AVForm::fillScreenModesComboBox(const QVector<lib::video::VideoMode> &modes) {
    videoModes = modes;

    bool previouslyBlocked = videoModescomboBox->blockSignals(true);
    videoModescomboBox->clear();

    for (int i = 0; i < videoModes.size(); ++i) {
        lib::video::VideoMode mode = videoModes[i];
        std::string pixelFormat =
                lib::video::CameraDevice::getPixelFormatString(mode.pixel_format).toStdString();
        qDebug("%dx%d+%d,%d FPS: %f, pixel format: %s\n", mode.width, mode.height, mode.x, mode.y,
               mode.FPS, pixelFormat.c_str());

        QString name;
        if (mode.width && mode.height)
            name = tr("Screen %1").arg(i + 1);
        else
            name = tr("Select region");

        videoModescomboBox->addItem(name);
    }

    videoModescomboBox->blockSignals(previouslyBlocked);
}

int AVForm::searchPreferredIndex() {
    QRect prefRes = videoSettings->getCamVideoRes();
    float prefFPS = videoSettings->getCamVideoFPS();

    for (int i = 0; i < videoModes.size(); ++i) {
        lib::video::VideoMode mode = videoModes[i];
        if (mode.width == prefRes.width() && mode.height == prefRes.height() &&
            (qAbs(mode.FPS - prefFPS) < 0.0001f)) {
            return i;
        }
    }

    return -1;
}


void AVForm::fillAudioQualityComboBox() {
    const bool previouslyBlocked = audioQualityComboBox->blockSignals(true);

    audioQualityComboBox->addItem(tr("High (64 kbps)"), 64);
    audioQualityComboBox->addItem(tr("Medium (32 kbps)"), 32);
    audioQualityComboBox->addItem(tr("Low (16 kbps)"), 16);
    audioQualityComboBox->addItem(tr("Very low (8 kbps)"), 8);

    const int currentBitrate = audioSettings->getAudioBitrate();
    const int index = audioQualityComboBox->findData(currentBitrate);

    audioQualityComboBox->setCurrentIndex(index);
    audioQualityComboBox->blockSignals(previouslyBlocked);
}

lib::video::CameraSource* AVForm::initVideoModes(int curIndex) {
    if (curIndex < 0 || curIndex >= videoDeviceList.size()) {
        qWarning() << "Invalid index:" << curIndex;
        return nullptr;
    }

    auto dev = videoDeviceList[curIndex];
    // auto devName = dev.name;
    if(!camera){
        camera = lib::video::CameraSource::CreateInstance(dev);
    }
    auto modes = camera->getVideoModes();

    // bool isScreen = lib::video::CameraDevice::isScreen(devName);
    // if (isScreen) {
        // Add extra video mode to region selection
        // allVideoModes.push_back(lib::video::VideoMode());
        // videoModes = allVideoModes;
        // fillScreenModesComboBox(modes);
    // } else {
        // selectBestModes(allVideoModes);
        // videoModes = allVideoModes;
        fillCameraModesComboBox(modes);
    // }


    // QVector<lib::video::VideoMode> allVideoModes = camera->getVideoModes();

    int preferedIndex = searchPreferredIndex();
    if (preferedIndex != -1) {
        videoSettings->setScreenGrabbed(false);
        videoModescomboBox->blockSignals(true);
        videoModescomboBox->setCurrentIndex(preferedIndex);
        videoModescomboBox->blockSignals(false);
        open(videoModes[preferedIndex]);
        return camera.get();
    }

    // if (isScreen) {
    //     QRect rect = videoSettings->getScreenRegion();
    //     lib::video::VideoMode mode(rect);
    //     videoSettings->setScreenGrabbed(true);
    //     videoModescomboBox->setCurrentIndex(videoModes.size() - 1);
    //     open(mode);
    //     return;
    // }

    // If the user hasn't set a preferred resolution yet,
    // we'll pick the resolution in the middle of the list,
    // and the best FPS for that resolution.
    // If we picked the lowest resolution, the quality would be awful
    // but if we picked the largest, FPS would be bad and thus quality bad too.
    int mid = (videoModes.size() - 1) / 2;
    videoModescomboBox->setCurrentIndex(mid);

    return camera.get();
}

void AVForm::on_videoDevCombobox_currentIndexChanged(int index) {
    assert(0 <= index && index < videoDeviceList.size());


    auto dev = videoDeviceList[index];
    videoSettings->setVideoDev(dev.name);

    videoSettings->setScreenGrabbed(false);

    // bool previouslyBlocked = videoModescomboBox->blockSignals(true);
    // updateVideoModes(index);
    // videoModescomboBox->blockSignals(previouslyBlocked);

    if (videoSettings->getScreenGrabbed()) {
        return;
    }


    camera = lib::video::CameraSource::CreateInstance(videoDeviceList.at(index));
    videoModes = camera->getVideoModes();
    fillCameraModesComboBox( camera->getVideoModes());

    int modeIndex = videoModescomboBox->currentIndex();
    if (0 <= modeIndex && modeIndex < videoModes.size()) {
        auto mode = videoModes[modeIndex];
        open(mode);
    }
}

void AVForm::on_audioQualityComboBox_currentIndexChanged(int index) {
    audioSettings->setAudioBitrate(audioQualityComboBox->currentData().toInt());
}

lib::video::VideoSource* AVForm::initVideoDevices() {
    qDebug() << __func__;

    QString settingsInDev = videoSettings->getVideoDev();

    videoDeviceList = lib::video::CameraDevice::getDeviceList();
    // prevent currentIndexChanged to be fired while adding items
    videoDevCombobox->blockSignals(true);
    videoDevCombobox->clear();

    int videoDevIndex = 0;
    for (auto& device : videoDeviceList) {
        videoDevCombobox->addItem(device.name);
        if (device.url == settingsInDev) videoDevIndex = videoDevCombobox->count() - 1;
    }
    videoDevCombobox->setCurrentIndex(videoDevIndex);
    videoDevCombobox->blockSignals(false);
    return initVideoModes(videoDevIndex);
}

int AVForm::getModeSize(lib::video::VideoMode mode) {
    return qRound(mode.height / 120.0) * 120;
}

void AVForm::getAudioInDevices() {
    QStringList deviceNames;
    deviceNames << tr("Disabled") << audio->inDeviceNames();

    inDevCombobox->blockSignals(true);
    inDevCombobox->clear();
    inDevCombobox->addItems(deviceNames);
    inDevCombobox->blockSignals(false);

    int idx = 0;
    bool enabled = audioSettings->getAudioInDevEnabled();
    if (enabled && deviceNames.size() > 1) {
        QString dev = audioSettings->getInDev();
        idx = qMax(deviceNames.indexOf(dev), 1);
    }
    inDevCombobox->setCurrentIndex(idx);
}

void AVForm::getAudioOutDevices() {
    QStringList deviceNames;
    deviceNames << tr("Disabled") << audio->outDeviceNames();

    outDevCombobox->blockSignals(true);
    outDevCombobox->clear();
    outDevCombobox->addItems(deviceNames);
    outDevCombobox->blockSignals(false);

    int idx = 0;
    bool enabled = audioSettings->getAudioOutDevEnabled();
    if (enabled && deviceNames.size() > 1) {
        QString dev = audioSettings->getOutDev();
        idx = qMax(deviceNames.indexOf(dev), 1);
    }
    outDevCombobox->setCurrentIndex(idx);
}

void AVForm::on_inDevCombobox_currentIndexChanged(int deviceIndex) {
    const bool inputEnabled = deviceIndex > 0;
    audioSettings->setAudioInDevEnabled(inputEnabled);

    QString deviceName{};
    if (inputEnabled) {
        deviceName = inDevCombobox->itemText(deviceIndex);
    }

    const QString oldName = audioSettings->getInDev();
    if (oldName != deviceName) {
        audioSettings->setInDev(deviceName);
        audio->reinitInput(deviceName);
        audioSrc = audio->makeSource();
        connect(audioSrc.get(), &lib::audio::IAudioSource::volumeAvailable, this,
                &AVForm::setVolume);
    }

    microphoneSlider->setEnabled(inputEnabled);
    if (!inputEnabled) {
        volumeDisplay->setValue(volumeDisplay->minimum());
    }
}

void AVForm::on_outDevCombobox_currentIndexChanged(int deviceIndex) {
    const bool outputEnabled = deviceIndex > 0;
    audioSettings->setAudioOutDevEnabled(outputEnabled);

    QString deviceName{};
    if (outputEnabled) {
        deviceName = outDevCombobox->itemText(deviceIndex);
    }

    const QString oldName = audioSettings->getOutDev();

    if (oldName != deviceName) {
        audioSettings->setOutDev(deviceName);
        audio->reinitOutput(deviceName);
        audioSink = audio->makeSink();
    }

    playbackSlider->setEnabled(outputEnabled);
}

void AVForm::on_playbackSlider_valueChanged(int sliderSteps) {
    const int settingsVolume = getValueFromSteps(sliderSteps, audioSettings->getOutVolumeMin(),
                                                 audioSettings->getOutVolumeMax());
    audioSettings->setOutVolume(settingsVolume);

    if (audio->isOutputReady()) {
        // const qreal volume = getValueFromSteps(sliderSteps, audio->minOutputVolume(),
        // audio->maxOutputVolume());
        audio->setOutputVolumeStep(settingsVolume);
        if (cbEnableTestSound->isChecked() && audioSink) {
            audioSink->playMono16Sound(lib::audio::IAudioSink::Sound::Test);
        }
    }
}

void AVForm::on_cbEnableTestSound_stateChanged() {
    audioSettings->setEnableTestSound(cbEnableTestSound->isChecked());

    if (!cbEnableTestSound->isChecked()) {
        return;
    }

    if (!audio->isOutputReady()) {
        return;
    }
    if (!audioSink) {
        return;
    }
    audioSink->playMono16Sound(lib::audio::IAudioSink::Sound::Test);
}

void AVForm::on_microphoneSlider_valueChanged(int sliderSteps) {
    const qreal dB = getValueFromSteps(sliderSteps, audio->minInputGain(), audio->maxInputGain());
    audioSettings->setAudioInGainDecibel(dB);
    audio->setInputGain(dB);
}

void AVForm::on_audioThresholdSlider_valueChanged(int sliderSteps) {
    const qreal normThreshold =
            getValueFromSteps(sliderSteps, audio->minInputThreshold(), audio->maxInputThreshold());
    audioSettings->setAudioThreshold(normThreshold);
    audio->setInputThreshold(normThreshold);
}

VideoSurface* AVForm::createVideoSurface(const lib::video::VideoSource* const source) {
    camVideoSurface = std::make_unique<VideoSurface>(QPixmap());
    camVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    camVideoSurface->setMinimumSize(QSize(160, 120));
    camVideoSurface->setSource(source);
    return camVideoSurface.get();
}

void AVForm::retranslateUi() {
    Ui::AVForm::retranslateUi(this);
}

int AVForm::getStepsFromValue(qreal val, qreal valMin, qreal valMax) {
    const float norm = (val - valMin) / (valMax - valMin);
    return norm * lib::audio::Audio::totalSteps;
}

qreal AVForm::getValueFromSteps(int steps, qreal valMin, qreal valMax) {
    return (static_cast<float>(steps) / lib::audio::Audio::totalSteps) * (valMax - valMin) + valMin;
}
}  // namespace module::im
