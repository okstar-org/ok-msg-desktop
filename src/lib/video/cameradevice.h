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

#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#include <QHash>
#include <QMutex>
#include <QString>
#include <QVector>
#include <atomic>
#include <base/compatiblerecursivemutex.h>
#include "videoframe.h"
#include "videomode.h"

struct AVDeviceInfoList;
struct AVDictionary;
struct AVFormatContext;
struct AVInputFormat;
struct AVCodecContext;

namespace lib::video {


class CameraDevice {
public:
    explicit CameraDevice(const VideoDevice &dev, FrameHandler* h);
    ~CameraDevice();

    bool open(VideoMode mode);
    bool close();

    static QVector<VideoDevice> getDeviceList();

    QVector<VideoMode> getVideoModes() const;

    static QString getPixelFormatString(uint32_t pixel_format);
    static bool betterPixelFormat(uint32_t a, uint32_t b);

    static QString getDefaultDeviceName();

    static bool isScreen(const QString& devName);

    static const AVInputFormat* getDefaultInputFormat(VideoType type);

    void stream();
    void stop();


private:
    void readFrame();

    bool open(const VideoDevice& dev, AVDictionary** options, std::string &error);


    // QVector<QPair<QString, QString>> getRawDeviceListGeneric();
    static QVector<VideoMode> getScreenModes();

    mutable CompatibleRecursiveMutex openDeviceLock;

    VideoDevice videoDevice;
    int videoStreamIndex;
    VideoFrame::IDType id;

    AVDictionary* options;
    // AVInputFormat* format;
    AVFormatContext* context;
    AVCodecContext* cctx;

    FrameHandler *handler;
    std::atomic<bool> run;
};
}  // namespace lib::video
#endif  // CAMERADEVICE_H
