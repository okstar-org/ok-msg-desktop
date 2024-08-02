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

#ifndef TOXFILEPROGRESS_H
#define TOXFILEPROGRESS_H

#include <QTime>

struct ToxFile;

class ToxFileProgress {
public:
    bool needsUpdate() const;
    void addSample(ToxFile const& file);
    void resetSpeed();

    double getProgress() const;
    double getSpeed() const;
    double getTimeLeftSeconds() const;

private:
    uint64_t lastBytesSent = 0;

    static const uint8_t TRANSFER_ROLLING_AVG_COUNT = 4;
    uint8_t meanIndex = 0;
    double meanData[TRANSFER_ROLLING_AVG_COUNT] = {0.0};

    QTime lastTick = QTime::currentTime();

    double speedBytesPerSecond;
    double timeLeftSeconds;
    double progress;
};

#endif  // TOXFILEPROGRESS_H
