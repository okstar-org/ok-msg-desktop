
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

// extern "C" {
// #include <libavcodec/avcodec.h>
// #include <libavdevice/avdevice.h>
// #include <libavformat/avformat.h>
// #include <libswscale/swscale.h>
// }
#include <QDebug>

#include <QReadLocker>
#include <QWriteLocker>
#include <QtConcurrent/QtConcurrentRun>
#include <functional>
#include "cameradevice.h"
#include "camerasource.h"
#include "lib/storage/settings/OkSettings.h"
#include "videoframe.h"


namespace lib::video {


CameraSource::CameraSource(const VideoDevice &dev)
        : dev(dev)
        , mode(VideoMode())
        , device(new CameraDevice(dev, this))
{
    qDebug() << __func__ << "Created.";

    qRegisterMetaType<VideoMode>("VideoMode");
    qRegisterMetaType<std::shared_ptr<lib::video::OVideoFrame>>(
            "std::shared_ptr<lib::video::OVideoFrame>");

    // deviceThread->setObjectName("Device thread");
    // deviceThread->start();
    // moveToThread(deviceThread);
}



CameraSource::~CameraSource() {

    closeDevice();
    qDebug() << __func__ << "Destroyed.";

    // deviceThread->exit(0);
    // deviceThread->wait();

    // QMutexLocker locker{&mutex};

    // qDebug() << "untrackFrames" << id;
    // Free all remaining VideoFrame
    // VideoFrame::untrackFrames(id, true);

            // if (cctx) {
            //     avcodec_free_context(&cctx);
            // }
            // locker.unlock();
}


/**
 * @brief Creates the instance.
 */
std::unique_ptr<CameraSource> CameraSource::CreateInstance(VideoDevice dev) {
    qDebug() << __func__ << dev.name;
    return std::make_unique<CameraSource>(dev);
}


/**
 * @brief Setup default device
 * @note If a device is already open, the source will seamlessly switch to the new device.
 */
void CameraSource::setupDefault() {
    auto s = &lib::settings::OkSettings::getInstance();

    auto deviceName = CameraDevice::getDefaultDeviceName();
    qDebug() << "Setup default device:" << deviceName;
    bool isScreen = CameraDevice::isScreen(deviceName);
    VideoMode mode = VideoMode(s->getScreenRegion());
    if (!isScreen) {
        mode = VideoMode(s->getCamVideoRes());
        mode.FPS = s->getCamVideoFPS();
    }

    setupDevice(deviceName, mode);
}

void CameraSource::setup(const VideoMode &vm)
{
    mode = vm;
}

void CameraSource::setupDevice(const QString& deviceName_, const VideoMode& Mode) {

}

QVector<VideoMode> CameraSource::getVideoModes()
{
    return device->getVideoModes();
}

void CameraSource::subscribe() {

}

void CameraSource::unsubscribe() {

}

void CameraSource::onCompleted()
{
    if (device) {
        device->close();
        delete device;
        emit sourceStopped();
    }
}

void CameraSource::onFrame(std::shared_ptr<OVideoFrame> vf)
{
    // qDebug() << __func__ << "frame:" << vf->sourceID;
    emit frameAvailable(vf);
}

/**
 * @brief Opens the video device and starts streaming.
 * @note Callers must own the biglock.
 */
void CameraSource::openDevice() {

    if(deviceThread){
        qWarning() << "Opened device!";
        return;
    }

    QMutexLocker locker{&mutex};

    auto deviceName = dev.name;
    qDebug() << "Opening device" << deviceName;

    // We need to create a new CameraDevice(Enable camera light)
    bool opened = device->open(mode);
    if (!opened) {
        qWarning() << "Failed to open device:" << deviceName;
        emit openFailed();
        return;
    }

    //Make device thread
    deviceThread = std::make_unique<std::jthread>([&](){
        device->stream();
    });

    // Device is opened
    emit deviceOpened();
}

/**
 * @brief Closes the video device and stops streaming.
 * @note
 */
void CameraSource::closeDevice() {

    if(!deviceThread){
        qWarning() << "Is not opened!";
        return;
    }

    QMutexLocker locker{&mutex};
    qDebug() << "Closing device" << dev.name;


    //stop device
    if (device) {
        device->stop();
    }

    //destroy device thread(auto wait thread on destroy).
    qDebug() << "Stop the device thread";
    deviceThread.reset();

    emit deviceClosed();

}

}  // namespace lib::video
