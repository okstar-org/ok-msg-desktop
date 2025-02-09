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

#ifndef OPENAL_H
#define OPENAL_H

#include <memory>
#include <unordered_set>

#include <QMutex>
#include <QObject>
#include <QTimer>

#include <cassert>
#include <cmath>

#include "alsink.h"
#include "alsource.h"
#include "base/compatiblerecursivemutex.h"
#include "src/lib/audio/iaudiocontrol.h"

class ALCdevice;
class ALCcontext;
typedef int ALCsizei;

namespace lib::audio {

class OpenAL : public IAudioControl {
    Q_OBJECT
public:
    OpenAL();
    virtual ~OpenAL();

    qreal maxOutputVolume() const { return 1; }
    qreal minOutputVolume() const { return 0; }
    qreal outputVolume() const;
    void setOutputVolume(qreal volume);
    void setOutputVolumeStep(int step);

    qreal minInputGain() const;
    void setMinInputGain(qreal dB);

    qreal maxInputGain() const;
    void setMaxInputGain(qreal dB);

    qreal inputGain() const;
    void setInputGain(qreal dB);

    qreal minInputThreshold() const;
    qreal maxInputThreshold() const;

    qreal getInputThreshold() const;
    void setInputThreshold(qreal normalizedThreshold);

    float getInputVol(const int16_t* buffer, int samples) override;


    void reinitInput(const QString& inDevDesc);
    bool reinitOutput(const QString& outDevDesc);

    bool isOutputReady() const;

    QStringList outDeviceNames();
    QStringList inDeviceNames();

    std::unique_ptr<IAudioSink> makeSink();
    void destroySink(AlSink& sink);

    std::unique_ptr<IAudioSource> makeSource();
    void destroySource(AlSource& source);

    void startLoop(uint sourceId);
    void stopLoop(uint sourceId);
    void playMono16Sound(AlSink& sink, const IAudioSink::Sound& sound);
    void stopActive();

    void playAudioBuffer(uint sourceId,
                         const int16_t* data,
                         int samples,
                         unsigned channels,
                         int sampleRate);

    void playFile(const QString& file);

signals:
    void startActive(qreal msec);

protected:
    static void checkAlError() noexcept;
    static void checkAlcError(ALCdevice* device) noexcept;

    qreal inputGainFactor() const;
    virtual void cleanupInput();
    virtual void cleanupOutput();

       bool autoInitInput();
       bool autoInitOutput();

    bool initInput(const QString& deviceName, uint32_t channels);

    void doAudio();

    virtual void doInput();
    virtual void doOutput();
    virtual void captureSamples(ALCdevice* device, int16_t* buffer, ALCsizei samples);

private:
    virtual bool initInput(const QString& deviceName);
    virtual bool initOutput(const QString& outDevDescr);

    void cleanupBuffers(uint sourceId);
    void cleanupSound();

    float getVolume(const int16_t *buffer, int samples);

protected:
    QThread* audioThread;
    mutable CompatibleRecursiveMutex audioLock;
    QString inDev{};
    QString outDev{};

    ALCdevice* alInDev = nullptr;
    QTimer captureTimer;
    QTimer cleanupTimer;

    ALCdevice* alOutDev = nullptr;
    ALCcontext* alOutContext = nullptr;

    bool outputInitialized = false;

    // Qt containers need copy operators, so use stdlib containers
    std::unordered_set<AlSink*> sinks;
    std::unordered_set<AlSink*> soundSinks;
    std::unordered_set<AlSource*> sources;

    int channels = 0;
    qreal gain = 0;
    qreal gainFactor = 1;
    qreal minInGain = -30;
    qreal maxInGain = 30;
    qreal inputThreshold = 0;
    qreal voiceHold = 250;
    bool isActive = false;
    QTimer voiceTimer;
    const qreal minInThreshold = 0.0;
    const qreal maxInThreshold = 0.4;
    // int16_t* inputBuffer = nullptr;
};
}  // namespace lib::audio
#endif  // OPENAL_H
