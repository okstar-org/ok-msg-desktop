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
#include <QRecursiveMutex>
#include <QString>
#include <QVector>
#include <atomic>
#include "lib/video/videomode.h"
#include "lib/video/videosource.h"

class CameraDevice;


namespace lib::video {

class CameraSource : public VideoSource, public FrameHandler {
    Q_OBJECT
public:
    static std::unique_ptr<CameraSource> CreateInstance(VideoDevice dev);
    static void destroyInstance();

    explicit CameraSource(const VideoDevice &dev);
    ~CameraSource() override;

    QVector<VideoMode> getVideoModes();


    void setupDefault();

    // VideoSource interface
    void subscribe() override;
    void unsubscribe() override;
protected:
    void onCompleted() override;
    void onFrame(std::shared_ptr<VideoFrame>) override;

private:

    QRecursiveMutex mutex;
    QThread* deviceThread;
    QFuture<void> streamFuture;

    VideoDevice dev;
    VideoMode mode;
    CameraDevice* device;

signals:
    void deviceOpened();
    void openFailed();


public slots:
    void openDevice();
    void closeDevice();
    void setupDevice(const QString& deviceName_, const VideoMode& mode);

};
}  // namespace lib::video
#endif  // CAMERA_H
