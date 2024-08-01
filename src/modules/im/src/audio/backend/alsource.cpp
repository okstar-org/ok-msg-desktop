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

#include "src/audio/backend/alsource.h"
#include "src/audio/backend/openal.h"

/**
 * @brief Emits audio frames captured by an input device or other audio source.
 */

/**
 * @brief Reserves ressources for an audio source
 * @param audio Main audio object, must have longer lifetime than this object.
 */
AlSource::AlSource(OpenAL& al) : audio(al) {}

AlSource::~AlSource() {
    QMutexLocker{&killLock};

    // unsubscribe only if not already killed
    if (!killed) {
        audio.destroySource(*this);
        killed = true;
    }
}

AlSource::operator bool() const {
    QMutexLocker{&killLock};
    return !killed;
}

void AlSource::kill() {
    killLock.lock();
    // this flag is only set once here, afterwards the object is considered dead
    killed = true;
    killLock.unlock();
    emit invalidated();
}
