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
#include "src/video/videomode.h"
#include "ui_avform.h"

#include <memory>

class IAudioControl;
class IAudioSettings;
class IAudioSink;
class IAudioSource;
class CameraSource;
class CoreAV;
class IVideoSettings;
class VideoSurface;
class AVForm : public GenericForm, private Ui::AVForm {
    Q_OBJECT
public:
    AVForm(CameraSource& camera, IAudioSettings* audioSettings, IVideoSettings* videoSettings);
    ~AVForm() override;
    QString getFormName() final override { return tr("Audio/Video"); }

private:
    void getAudioInDevices();
    void getAudioOutDevices();
    void getVideoDevices();

    static int getModeSize(VideoMode mode);
    void selectBestModes(QVector<VideoMode>& allVideoModes);
    void fillCameraModesComboBox();
    void fillScreenModesComboBox();
    void fillAudioQualityComboBox();
    int searchPreferredIndex();

    void createVideoSurface();
    void killVideoSurface();

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
    void open(const QString& devName, const VideoMode& mode);
    int getStepsFromValue(qreal val, qreal valMin, qreal valMax);
    qreal getValueFromSteps(int steps, qreal valMin, qreal valMax);
    void trackNewScreenGeometry(QScreen* qScreen);

private:
    std::unique_ptr<IAudioControl> audio;

    IAudioSettings* audioSettings;
    IVideoSettings* videoSettings;

    bool subscribedToAudioIn;
    std::unique_ptr<IAudioSink> audioSink = nullptr;
    std::unique_ptr<IAudioSource> audioSrc = nullptr;
    VideoSurface* camVideoSurface;
    CameraSource& camera;
    QVector<QPair<QString, QString>> videoDeviceList;
    QVector<VideoMode> videoModes;
    uint alSource;
    const uint totalSliderSteps = 100;  // arbitrary number of steps to give slider a good "feel"
};

#endif
