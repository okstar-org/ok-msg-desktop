
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

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
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
        : deviceThread{new QThread}
        , dev(dev)
        , device(new CameraDevice(dev))
        , mode(VideoMode())
        // clang-format off
        , cctx(nullptr)
        , videoStreamIndex{-1}
{
    qDebug() << __func__;

    qRegisterMetaType<VideoMode>("VideoMode");
    deviceThread->setObjectName("Device thread");
    deviceThread->start();
    moveToThread(deviceThread);

    avdevice_register_all();
}

// clang-format on

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

    QString deviceName = CameraDevice::getDefaultDeviceName();
    qDebug() << "Setup default device:" << deviceName;
    bool isScreen = CameraDevice::isScreen(deviceName);
    VideoMode mode = VideoMode(s->getScreenRegion());
    if (!isScreen) {
        mode = VideoMode(s->getCamVideoRes());
        mode.FPS = s->getCamVideoFPS();
    }

    setupDevice(deviceName, mode);
}

/**
 * @brief Change the device and mode.
 * @note If a device is already open, the source will seamlessly switch to the new device.
 */
void CameraSource::setupDevice(const QString& deviceName_, const VideoMode& Mode) {
    // qDebug() << "Setup device:" << deviceName_;
    // if (QThread::currentThread() != deviceThread) {
        // QMetaObject::invokeMethod(this, "setupDevice", Q_ARG(const QString&, deviceName_),
                                  // Q_ARG(const VideoMode&, Mode));
        // return;
    // }

    // QWriteLocker locker{&deviceMutex};

    // if (deviceName_ == deviceName && Mode == mode) {
        // return;
    // }

    // if (subscriptions) {
    //     // To force close, ignoring optimization
    //     int subs = subscriptions;
    //     subscriptions = 0;
    //     closeDevice();
    //     subscriptions = subs;
    // }

    // deviceName = deviceName_;
    // _isNone = (deviceName == "none");
    // mode = Mode;

    // if (subscriptions && !_isNone) {
        // openDevice();
    // }
}

CameraSource::~CameraSource() {
    QWriteLocker locker{&streamMutex};
    QWriteLocker locker2{&deviceMutex};

    // Stop the device thread
    deviceThread->exit(0);
    deviceThread->wait();
    delete deviceThread;

    // Free all remaining VideoFrame
    VideoFrame::untrackFrames(id, true);

    if (cctx) {
        avcodec_free_context(&cctx);
    }

    if (device) {
        device->close();
        device = nullptr;
    }

    locker.unlock();

    // Synchronize with our stream thread
    while (streamFuture.isRunning()) QThread::yieldCurrentThread();
}

QVector<VideoMode> CameraSource::getVideoModes()
{
    return device->getVideoModes();
}

void CameraSource::subscribe() {
    QWriteLocker locker{&deviceMutex};

    // ++subscriptions;
    openDevice();
}

void CameraSource::unsubscribe() {
    QWriteLocker locker{&deviceMutex};

    // --subscriptions;
    // if (subscriptions == 0) {
        // closeDevice();
    // }
}

/**
 * @brief Opens the video device and starts streaming.
 * @note Callers must own the biglock.
 */
void CameraSource::openDevice() {
    if (QThread::currentThread() != deviceThread) {
        QMetaObject::invokeMethod(this, "openDevice");
        return;
    }

    QWriteLocker locker{&streamMutex};


    auto deviceName = dev.name;
    qDebug() << "Opening device" << deviceName;

    //    if (device) {
    //        device->open();
    //        emit openFailed();
    //        return;
    //    }

    // We need to create a new CameraDevice(Enable camera light)
    bool opened = device->open(mode);
    if (!opened) {
        qWarning() << "Failed to open device:" << deviceName;
        emit openFailed();
        return;
    }

    // Find the first video stream, if any
    for (unsigned i = 0; i < device->context->nb_streams; ++i) {
        AVMediaType type = device->context->streams[i]->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        qWarning() << "Video stream not found";
        emit openFailed();
        return;
    }


    // Get the stream's codec's parameters and find a matching decoder
    AVCodecParameters* cparams = device->context->streams[videoStreamIndex]->codecpar;
    AVCodecID codecId = cparams->codec_id;

    const AVCodec* codec = avcodec_find_decoder(codecId);
    if (!codec) {
        qWarning() << "Codec not found for:" << codecId;
        device->close();
        emit openFailed();
        return;
    }


    // Create a context for our codec, using the existing parameters
    cctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(cctx, cparams) < 0) {
        qWarning() << "Can't create AV context from parameters";
        emit openFailed();
        return;
    }

    // Open codec
    if (avcodec_open2(cctx, codec, nullptr) < 0) {
        qWarning() << "Can't open codec";
        avcodec_free_context(&cctx);
        emit openFailed();
        return;
    }

    if (streamFuture.isRunning())
        qDebug() << "The stream thread is already running! Keeping the current one open.";
    else
        streamFuture = QtConcurrent::run(std::bind(&CameraSource::stream, this));

    // Synchronize with our stream thread
    while (!streamFuture.isRunning()) QThread::yieldCurrentThread();

    emit deviceOpened();
}

/**
 * @brief Closes the video device and stops streaming.
 * @note Callers must own the biglock.
 */
void CameraSource::closeDevice() {
    if (QThread::currentThread() != deviceThread) {
        QMetaObject::invokeMethod(this, "closeDevice");
        return;
    }

    QWriteLocker locker{&streamMutex};


    qDebug() << "Closing device" << dev.name;

    // Free all remaining VideoFrame
    VideoFrame::untrackFrames(id, true);

    // Free our resources and close the device
    videoStreamIndex = -1;
    avcodec_free_context(&cctx);

    if (!device) {
        qDebug() <<"The device already was closed";
        return ;
    }
    device->close();
    device = nullptr;
}

/**
 * @brief Blocking. Decodes video stream and emits new frames.
 * @note Designed to run in its own thread.
 */
void CameraSource::stream() {
    auto streamLoop = [this]() {
        AVPacket packet;
        if (av_read_frame(device->context, &packet) != 0) {
            return;
        }

        // Forward packets to the decoder and grab the decoded frame
        bool isVideo = packet.stream_index == videoStreamIndex;
        bool readyToRecive = isVideo && !avcodec_send_packet(cctx, &packet);

        if (readyToRecive) {
            AVFrame* frame = av_frame_alloc();
            if (frame && !avcodec_receive_frame(cctx, frame)) {
                VideoFrame* vframe = new VideoFrame(id, frame);
                emit frameAvailable(vframe->trackFrame());
            } else {
                av_frame_free(&frame);
            }
        }

        av_packet_unref(&packet);
    };

    forever {
        QReadLocker locker{&streamMutex};

        // Exit if device is no longer valid
        if (!device) {
            break;
        }

        streamLoop();
    }
}
}  // namespace lib::video
