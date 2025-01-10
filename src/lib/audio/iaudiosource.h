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

#ifndef IAUDIOSOURCE_H
#define IAUDIOSOURCE_H

#include <QObject>

/**
 * @fn void Audio::frameAvailable(const int16_t *pcm, size_t sample_count, uint8_t channels,
 * uint32_t sampling_rate);
 *
 * When there are input subscribers, we regularly emit captured audio frames with this signal
 * Always connect with a blocking queued connection lambda, else the behaviour is undefined
 */

class IAudioSource : public QObject {
    Q_OBJECT
public:
    virtual ~IAudioSource() = default;

    virtual operator bool() const = 0;

signals:
    void frameAvailable(const int16_t* pcm,
                        size_t sample_count,
                        uint8_t channels,
                        uint32_t sampling_rate);
    void volumeAvailable(float value);
    void invalidated();
};

#endif  // IAUDIOSOURCE_H
