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
#include "videomode.h"

struct AVFormatContext;
struct AVInputFormat;
struct AVDeviceInfoList;
struct AVDictionary;

class CameraDevice {
public:
    static CameraDevice* open(QString devName, VideoMode mode = VideoMode());
    void open();
    bool close();

    static QVector<QPair<QString, QString>> getDeviceList();

    static QVector<VideoMode> getVideoModes(QString devName);
    static QString getPixelFormatString(uint32_t pixel_format);
    static bool betterPixelFormat(uint32_t a, uint32_t b);

    static QString getDefaultDeviceName();

    static bool isScreen(const QString& devName);

private:
    CameraDevice(const QString& devName, AVFormatContext* context);
    static CameraDevice* open(QString devName, AVDictionary** options);
    static bool getDefaultInputFormat();
    static QVector<QPair<QString, QString>> getRawDeviceListGeneric();
    static QVector<VideoMode> getScreenModes();

public:
    const QString devName;
    AVFormatContext* context;

private:
    std::atomic_int refcount;
    static QHash<QString, CameraDevice*> openDevices;
    static QMutex openDeviceLock, iformatLock;
};

#endif  // CAMERADEVICE_H
