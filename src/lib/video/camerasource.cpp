
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
        // : deviceThread{new QThread}
        : dev(dev)
        , mode(VideoMode())
{
    qDebug() << __func__;

    // Crate camera device
    device = new CameraDevice(dev, this);

    qRegisterMetaType<VideoMode>("VideoMode");
    qRegisterMetaType<std::shared_ptr<lib::video::OVideoFrame>>(
            "std::shared_ptr<lib::video::OVideoFrame>");

    // deviceThread->setObjectName("Device thread");
    // deviceThread->start();
    // moveToThread(deviceThread);
}



CameraSource::~CameraSource() {
    qDebug() << __func__;

    closeDevice();

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
    }

    delete device;
    emit sourceStopped();
}

void CameraSource::onFrame(std::shared_ptr<OVideoFrame> vf)
{
    qDebug() << __func__ << "frame:" << vf->sourceID;
    emit frameAvailable(vf);
}

/**
 * @brief Opens the video device and starts streaming.
 * @note Callers must own the biglock.
 */
void CameraSource::openDevice() {
    // if (QThread::currentThread() != deviceThread) {
        // QMetaObject::invokeMethod(this, "openDevice");
        // return;
    // }

    QMutexLocker locker{&mutex};

    auto deviceName = dev.name;
    qDebug() << "Opening device" << deviceName;

    if(!device){
        // Crate camera device
        device = new CameraDevice(dev, this);
    }

    // We need to create a new CameraDevice(Enable camera light)
    bool opened = device->open(mode);
    if (!opened) {
        qWarning() << "Failed to open device:" << deviceName;
        emit openFailed();
        return;
    }


    // // Find the first video stream, if any
    // for (unsigned i = 0; i < device->context->nb_streams; ++i) {
    //     AVMediaType type = device->context->streams[i]->codecpar->codec_type;
    //     if (type == AVMEDIA_TYPE_VIDEO) {
    //         videoStreamIndex = i;
    //         break;
    //     }
    // }

    // if (videoStreamIndex == -1) {
    //     qWarning() << "Video stream not found";
    //     emit openFailed();
    //     return;
    // }


    // // Get the stream's codec's parameters and find a matching decoder
    // AVCodecParameters* cparams = device->context->streams[videoStreamIndex]->codecpar;
    // AVCodecID codecId = cparams->codec_id;

    // const AVCodec* codec = avcodec_find_decoder(codecId);
    // if (!codec) {
    //     qWarning() << "Codec not found for:" << codecId;
    //     device->close();
    //     emit openFailed();
    //     return;
    // }


    // // Create a context for our codec, using the existing parameters
    // cctx = avcodec_alloc_context3(codec);
    // if (avcodec_parameters_to_context(cctx, cparams) < 0) {
    //     qWarning() << "Can't create AV context from parameters";
    //     emit openFailed();
    //     return;
    // }

    // // Open codec
    // if (avcodec_open2(cctx, codec, nullptr) < 0) {
    //     qWarning() << "Can't open codec";
    //     avcodec_free_context(&cctx);
    //     emit openFailed();
    //     return;
    // }

    // if (streamFuture.isRunning())
        // qDebug() << "The stream thread is already running! Keeping the current one open.";
    // else
        // streamFuture = QtConcurrent::run([this] { device->stream(); });

    // Synchronize with our stream thread
    // while (!streamFuture.isRunning()) QThread::yieldCurrentThread();
    deviceThread = std::make_unique<std::jthread>([&](){
        device->stream();
    });

    emit deviceOpened();
}

/**
 * @brief Closes the video device and stops streaming.
 * @note Callers must own the biglock.
 */
void CameraSource::closeDevice() {
    // if (QThread::currentThread() != deviceThread) {
    //     QMetaObject::invokeMethod(this, "closeDevice");
    //     return;
    // }

    QMutexLocker locker{&mutex};


    qDebug() << "Closing device" << dev.name;

    // Free all remaining VideoFrame
    // VideoFrame::untrackFrames(id, true);

    // Stop record
    if (device) {
        device->stop();
    }
    deviceThread.reset();
    // qDebug() << "Stop the device thread";
    // deviceThread->exit(0);
    // deviceThread->wait();
    // delete deviceThread;
    // qDebug() << __func__ << "finished.";

    // Synchronize with our stream thread
    // while (streamFuture.isRunning()){
    //     streamFuture.waitForFinished();
    //     // QThread::yieldCurrentThread();
    // }

    // device = nullptr;
    // Synchronize with our stream thread
    // while (streamFuture.isRunning()) QThread::yieldCurrentThread();
}




}  // namespace lib::video
