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

#include "src/audio/backend/alsink.h"
#include "src/audio/backend/openal.h"

#include <QDebug>
#include <QMutexLocker>

/**
 * @brief Can play audio via the speakers or some other audio device. Allocates
 *        and frees the audio ressources internally.
 */

AlSink::~AlSink() {
    QMutexLocker{&killLock};

    // unsubscribe only if not already killed
    if (!killed) {
        audio.destroySink(*this);
        killed = true;
    }
}

void AlSink::playAudioBuffer(const int16_t* data, int samples, unsigned channels,
                             int sampleRate) const {
    QMutexLocker{&killLock};

    if (killed) {
        qCritical() << "Trying to play audio on an invalid sink";
    } else {
        audio.playAudioBuffer(sourceId, data, samples, channels, sampleRate);
    }
}

void AlSink::playMono16Sound(const IAudioSink::Sound& sound) {
    QMutexLocker{&killLock};

    if (killed) {
        qCritical() << "Trying to play sound on an invalid sink";
    } else {
        audio.playMono16Sound(*this, sound);
    }
}

void AlSink::startLoop() {
    QMutexLocker{&killLock};

    if (killed) {
        qCritical() << "Trying to start loop on an invalid sink";
    } else {
        audio.startLoop(sourceId);
    }
}

void AlSink::stopLoop() {
    QMutexLocker{&killLock};

    if (killed) {
        qCritical() << "Trying to stop loop on an invalid sink";
    } else {
        audio.stopLoop(sourceId);
    }
}

uint AlSink::getSourceId() const {
    uint tmp = sourceId;
    return tmp;
}

void AlSink::kill() {
    killLock.lock();
    // this flag is only set once here, afterwards the object is considered dead
    killed = true;
    killLock.unlock();
    emit invalidated();
}

AlSink::AlSink(OpenAL& al, uint sourceId) : audio(al), sourceId{sourceId} {}

AlSink::operator bool() const {
    QMutexLocker{&killLock};

    return !killed;
}
