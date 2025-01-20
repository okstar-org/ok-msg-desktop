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

#ifndef AVFORM_H
#define AVFORM_H

#include <QList>
#include <QObject>
#include <QString>

#include "genericsettings.h"
#include "lib/video/videomode.h"
#include "ui_avform.h"

#include <memory>

#include <src/persistence/profile.h>

namespace lib::video {
class CameraSource;
class IVideoSettings;
}  // namespace lib::video

namespace lib::audio {
class IAudioControl;
class IAudioSettings;
class IAudioSink;
class IAudioSource;
}  // namespace lib::audio

namespace module::im {
class CoreAV;
class VideoSurface;

class AVForm : public GenericForm, private Ui::AVForm {
    Q_OBJECT
public:
    AVForm();
    ~AVForm() override;
    QString getFormName() final override {
        return tr("Audio/Video");
    }

private:
    void getAudioInDevices();
    void getAudioOutDevices();
    void getVideoDevices();

    static int getModeSize(lib::video::VideoMode mode);
    void selectBestModes(QVector<lib::video::VideoMode>& allVideoModes);
    void fillCameraModesComboBox();
    void fillScreenModesComboBox();
    void fillAudioQualityComboBox();
    int searchPreferredIndex();

    VideoSurface* createVideoSurface();

    void retranslateUi();

private slots:
    // audio
    void on_inDevCombobox_currentIndexChanged(int deviceIndex);
    void on_outDevCombobox_currentIndexChanged(int deviceIndex);
    void on_playbackSlider_valueChanged(int sliderSteps);
    void on_cbEnableTestSound_stateChanged();
    void on_microphoneSlider_valueChanged(int sliderSteps);
    void on_audioThresholdSlider_valueChanged(int sliderSteps);
    void on_audioQualityComboBox_currentIndexChanged(int index);

    // camera
    void on_videoDevCombobox_currentIndexChanged(int index);
    void on_videoModescomboBox_currentIndexChanged(int index);

    void rescanDevices();
    void setVolume(float value);

protected:
    void updateVideoModes(int curIndex);

private:
    void hideEvent(QHideEvent* event) final override;
    void showEvent(QShowEvent* event) final override;
    void open(const QString& devName, const lib::video::VideoMode& mode);
    int getStepsFromValue(qreal val, qreal valMin, qreal valMax);
    qreal getValueFromSteps(int steps, qreal valMin, qreal valMax);
    void trackNewScreenGeometry(QScreen* qScreen);

private:
    std::unique_ptr<lib::audio::IAudioControl> audio;

    lib::audio::IAudioSettings* audioSettings;
    lib::video::IVideoSettings* videoSettings;

    bool subscribedToAudioIn;
    std::unique_ptr<lib::audio::IAudioSink> audioSink;
    std::unique_ptr<lib::audio::IAudioSource> audioSrc;
    std::unique_ptr<VideoSurface> camVideoSurface;
    lib::video::CameraSource* camera;
    QVector<QPair<QString, QString>> videoDeviceList;
    QVector<lib::video::VideoMode> videoModes;
    uint alSource;
    // arbitrary number of steps to give slider a good "feel"

private slots:
    void onProfileChanged(Profile*);
};
}  // namespace module::im
#endif
