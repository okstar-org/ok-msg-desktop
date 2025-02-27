
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
#include <QThread>
#include <QtConcurrent/QtConcurrentRun>
#include "cameradevice.h"
#include "camerasource.h"
#include "lib/storage/settings/OkSettings.h"
#include "videoframe.h"


namespace lib::video {

CameraSource::CameraSource(const VideoDevice &dev)
        : dev(dev)
        , mode(VideoMode())
        , device(new CameraDevice(dev, this))
        , deviceThread(nullptr)
{
    qDebug() << __func__ << "Created.";

    qRegisterMetaType<VideoMode>("VideoMode");
    qRegisterMetaType<std::shared_ptr<lib::video::OVideoFrame>>(
            "std::shared_ptr<lib::video::OVideoFrame>");
}

CameraSource::~CameraSource() {
    qDebug() << __func__;
    closeDevice();
    delete device;
    qDebug() << __func__ << "Destroyed.";
}

/**
 * @brief Creates the instance.
 */
std::unique_ptr<CameraSource> CameraSource::CreateInstance(const VideoDevice &dev) {
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
    QMutexLocker locker{&mutex};
    mode = vm;

    bool isOpened = device->isOpened();

    auto y = device->setVideoMode(mode);
    if(!y){
        return;
    }

    if(isOpened){
        closeDevice();
        openDevice();
    }
}

void CameraSource::setupDevice(const QString& deviceName_, const VideoMode& Mode) {

}

void CameraSource::stream()
{
    device->stream();
}

QVector<VideoMode> CameraSource::getVideoModes()
{
    QMutexLocker locker{&mutex};
    return device->getVideoModes();
}

void CameraSource::onCompleted()
{
    if (device) {
        device->close();

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

    if(device->isOpened()){
        qWarning() << "Was opened!";
        return;
    }

    QMutexLocker locker{&mutex};

    auto deviceName = dev.name;
    qDebug() << "Opening device" << deviceName;

            // We need to create a new CameraDevice(Enable camera light)
    bool opened = device->open();
    if (!opened) {
        qWarning() << "Failed to open device:" << deviceName;
        emit openFailed();
        return;
    }

            //Make device thread
    if(!deviceThread){
        deviceThread = new QThread(this);
        deviceThread->setObjectName("Camera thread");
        connect(deviceThread, &QThread::started, this, &CameraSource::stream);
        moveToThread(deviceThread);
        deviceThread->start();
    }

            // Device is opened
    emit deviceOpened();
}

/**
 * @brief Closes the video device and stops streaming.
 * @note
 */
void CameraSource::closeDevice() {
    qDebug() << "Closing device" << dev.name;
    QMutexLocker locker{&mutex};

    if(!device->isOpened()){
        qWarning() << "Was closed!";
        return;
    }

            //stop device
    if (device) {
        device->stop();
    }

            //destroy device thread(auto wait thread on destroy).
    qDebug() << "Stop the device thread";
    deviceThread->exit();
    deviceThread->wait();
    deviceThread = nullptr;
    emit deviceClosed();
}

}  // namespace lib::video
