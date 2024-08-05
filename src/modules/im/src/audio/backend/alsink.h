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

#ifndef ALSINK_H
#define ALSINK_H

#include <QMutex>
#include <QObject>

#include "base/compatiblerecursivemutex.h"
#include "src/audio/iaudiosink.h"
#include "src/base/interface.h"

class OpenAL;
class QMutex;
class AlSink : public QObject, public IAudioSink {
    Q_OBJECT
public:
    AlSink(OpenAL& al, uint sourceId);
    AlSink(const AlSink& src) = delete;
    AlSink& operator=(const AlSink&) = delete;
    AlSink(AlSink&& other) = delete;
    AlSink& operator=(AlSink&& other) = delete;
    ~AlSink();

    void playAudioBuffer(const int16_t* data, int samples, unsigned channels,
                         int sampleRate) const override;
    void playMono16Sound(const IAudioSink::Sound& sound) override;
    void startLoop() override;
    void stopLoop() override;

    operator bool() const override;

    uint getSourceId() const;
    void kill();

    SIGNAL_IMPL(AlSink, finishedPlaying)
    SIGNAL_IMPL(AlSink, invalidated)

private:
    OpenAL& audio;
    uint sourceId;
    bool killed = false;
    mutable CompatibleRecursiveMutex killLock;
};

#endif  // ALSINK_H
