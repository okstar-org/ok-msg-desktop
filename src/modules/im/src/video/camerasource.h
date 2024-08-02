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

#ifndef CAMERA_H
#define CAMERA_H

#include <QFuture>
#include <QHash>
#include <QReadWriteLock>
#include <QString>
#include <QVector>
#include <atomic>
#include "src/video/videomode.h"
#include "src/video/videosource.h"

class CameraDevice;
struct AVCodecContext;

class CameraSource : public VideoSource {
    Q_OBJECT

public:
    static CameraSource& getInstance();
    static void destroyInstance();
    void setupDefault();
    bool isNone() const;

    // VideoSource interface
    virtual void subscribe() override;
    virtual void unsubscribe() override;

public slots:
    void setupDevice(const QString& deviceName_, const VideoMode& mode);

signals:
    void deviceOpened();
    void openFailed();

private:
    CameraSource();
    ~CameraSource();
    void stream();

private slots:
    void openDevice();
    void closeDevice();

private:
    QFuture<void> streamFuture;
    QThread* deviceThread;

    QString deviceName;
    CameraDevice* device;
    VideoMode mode;
    AVCodecContext* cctx;
    // TODO: Remove when ffmpeg version will be bumped to the 3.1.0
    AVCodecContext* cctxOrig;
    int videoStreamIndex;

    QReadWriteLock deviceMutex;
    QReadWriteLock streamMutex;

    std::atomic_bool _isNone;
    std::atomic_int subscriptions;

    static CameraSource* instance;
};

#endif  // CAMERA_H
