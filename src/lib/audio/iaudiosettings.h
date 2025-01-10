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

#ifndef I_AUDIO_SETTINGS_H
#define I_AUDIO_SETTINGS_H

#include "src/base/interface.h"

#include <QString>

class IAudioSettings {
public:
    virtual ~IAudioSettings() = default;

    virtual QString getInDev() const = 0;
    virtual void setInDev(const QString& deviceSpecifier) = 0;

    virtual bool getAudioInDevEnabled() const = 0;
    virtual void setAudioInDevEnabled(bool enabled) = 0;

    virtual QString getOutDev() const = 0;
    virtual void setOutDev(const QString& deviceSpecifier) = 0;

    virtual bool getAudioOutDevEnabled() const = 0;
    virtual void setAudioOutDevEnabled(bool enabled) = 0;

    virtual qreal getAudioInGainDecibel() const = 0;
    virtual void setAudioInGainDecibel(qreal dB) = 0;

    virtual qreal getAudioThreshold() const = 0;
    virtual void setAudioThreshold(qreal percent) = 0;

    virtual int getOutVolume() const = 0;
    virtual int getOutVolumeMin() const = 0;
    virtual int getOutVolumeMax() const = 0;
    virtual void setOutVolume(int volume) = 0;

    virtual int getAudioBitrate() const = 0;
    virtual void setAudioBitrate(int bitrate) = 0;

    virtual bool getEnableTestSound() const = 0;
    virtual void setEnableTestSound(bool newValue) = 0;

    DECLARE_SIGNAL(inDevChanged, const QString& device);
    DECLARE_SIGNAL(audioInDevEnabledChanged, bool enabled);

    DECLARE_SIGNAL(outDevChanged, const QString& device);
    DECLARE_SIGNAL(audioOutDevEnabledChanged, bool enabled);

    DECLARE_SIGNAL(audioInGainDecibelChanged, qreal dB);
    DECLARE_SIGNAL(audioThresholdChanged, qreal dB);
    DECLARE_SIGNAL(outVolumeChanged, int volume);
    DECLARE_SIGNAL(audioBitrateChanged, int bitrate);
    DECLARE_SIGNAL(enableTestSoundChanged, bool newValue);
};

#endif  // I_AUDIO_SETTINGS_H
