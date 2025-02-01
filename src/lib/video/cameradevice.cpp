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

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <format>
extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "cameradevice.h"
#include "lib/storage/settings/OkSettings.h"
#include "videoframe.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#define USING_V4L 1
#else
#define USING_V4L 0
#endif

#ifdef Q_OS_WIN
#include "camera/directshow.h"
#include <windows.h>
#endif
#if USING_V4L
#include "base/system/linux/x11_display.h"
#include "camera/v4l2.h"
#endif
#ifdef Q_OS_OSX
#include "camera/avfoundation.h"
#endif

namespace lib::video {

/**
 * @class CameraDevice
 *
 */

CameraDevice::CameraDevice(const VideoDevice &dev, FrameHandler* h)
        : videoDevice(dev)
        , videoStreamIndex(-1)
        , id(0)
        , options(nullptr)
        , context(nullptr)
        , cctx(nullptr)
        , handler(h)
        , run(false)
{
    qDebug() << __func__;
    avdevice_register_all();
}

CameraDevice::~CameraDevice()
{
    qDebug() << __func__;
}


void CameraDevice::stop()
{
    // QMutexLocker locker{&openDeviceLock};
    run = false;
}

void CameraDevice::stream() {
    qDebug() << __func__;

    run = true;

    forever {
        // Exit if context is no longer valid
        if(!run){
            // qWarning() << __func__ << "was Stoped!";
            break;
        }

        readFrame();
    }
    qDebug() << __func__ << "was Finished!";
    if(handler){
        handler->onCompleted();
    }
}


void CameraDevice::readFrame()
{
    QMutexLocker locker{&openDeviceLock};

    if (!context) {
        qWarning() << __func__ << "Exited.";
        return;
    }

    AVPacket packet;
    if (av_read_frame(context, &packet) != 0) {
        return;
    }

    // Forward packets to the decoder and grab the decoded frame
    bool isVideo = packet.stream_index == videoStreamIndex;
    bool readyToRecive = isVideo && !avcodec_send_packet(cctx, &packet);

    if (readyToRecive) {
        AVFrame* frame = av_frame_alloc();
        if(!frame)
            return;

        if (!avcodec_receive_frame(cctx, frame)) {
            // VideoFrame* vframe = new VideoFrame(++id, frame);
            // emit frameAvailable(vframe->trackFrame());
            if(handler)
                handler->onFrame(std::make_unique<VideoFrame>(++id, frame));
        }else{
            av_frame_free(&frame);
        }
    }

    av_packet_unref(&packet);
}

bool CameraDevice::open(const VideoDevice& dev, AVDictionary** options, std::string &error) {
    QMutexLocker locker(&openDeviceLock);

    qDebug() << __func__ << std::format("device:{} url:{}", dev.name.toStdString(), dev.url.toStdString()).c_str();

    auto format = getDefaultInputFormat(dev.type);
    int ret = avformat_open_input(&context, dev.url.toStdString().c_str(), format, options);
    if (ret < 0) {
        char msg[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, msg, AV_ERROR_MAX_STRING_SIZE);
        qWarning() <<  "Error opening device:" << dev.url << msg;
        error = msg;
        return false;
    }


    int aduration = context->max_analyze_duration = 0;
    if (avformat_find_stream_info(context, nullptr) < 0) {
        avformat_close_input(&context);
        avformat_free_context(context);
        error = "Unable to find stream info!";
        qWarning() << error.c_str();
        return false;
    }

    context->max_analyze_duration = aduration;


    // Find the first video stream, if any
    for (unsigned i = 0; i < context->nb_streams; ++i) {
        AVMediaType type = context->streams[i]->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        error = "Video stream not found!";
        qWarning() << error.c_str();
        // emit openFailed();
        return false;
    }


    // Get the stream's codec's parameters and find a matching decoder
    AVCodecParameters* cparams = context->streams[videoStreamIndex]->codecpar;
    AVCodecID codecId = cparams->codec_id;
    qDebug() << "Codec id is:" << codecId;
    const AVCodec* codec = avcodec_find_decoder(codecId);
    if (!codec) {
        auto msg = QString("Codec not found for:%1").arg(avcodec_get_name(codecId));
        qWarning() << msg;
        error = msg.toStdString();
        // device->close();
        // emit openFailed();
        return false;
    }


    // Create a context for our codec, using the existing parameters
    cctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(cctx, cparams) < 0) {
        error = "Can't create AV context from parameters";
        qWarning() << error.c_str();
        // emit openFailed();
        return false;
    }

    // Open codec
    if (avcodec_open2(cctx, codec, nullptr) < 0) {
        error= "Can't open codec";
        qWarning() << error.c_str();
        avcodec_free_context(&cctx);
        // emit openFailed();
        return false;
    }

    return true;
}


bool CameraDevice::open(VideoMode mode) {
    qDebug() << "Open device:" << videoDevice.name;
    qDebug() << "Vide mode is:[" << mode.width << "x" << mode.height << "FPS" << mode.FPS << "]";


    float FPS = 30;
    if (mode.FPS > 0.0f && mode.FPS <= 30) {
        FPS = mode.FPS;
    } else {
        qWarning() << "Using default FPS:" << FPS;
    }

    if (mode.width == 0) {
        mode.width = 340;
    }

    if (mode.height == 0) {
        mode.height = 480;
    }
    options = nullptr;
    //设置视频大小
    const std::string videoSize = QStringLiteral("%1x%2").arg(mode.width).arg(mode.height).toStdString();
    qDebug() << "videoSize: " << QString::fromStdString(videoSize);
    av_dict_set(&options, "video_size", videoSize.c_str(), 0);

    //采样率
    const std::string framerate = std::format("{}", FPS);
    qDebug() << "framerate: " << QString::fromStdString(framerate);
    av_dict_set(&options, "framerate", framerate.c_str(), 0);


    // auto devName = dev.name;
    // if (!iformat)
        // ;
// #if USING_V4L
//     if (devName.startsWith("x11grab#")) {
//         QSize screen;
//         if (mode.width && mode.height) {
//             screen.setWidth(mode.width);
//             screen.setHeight(mode.height);
//         } else {
//             QScreen* defaultScreen = QApplication::primaryScreen();
//             qreal pixRatio = defaultScreen->devicePixelRatio();

//             screen = defaultScreen->size();
//             // Workaround https://trac.ffmpeg.org/ticket/4574 by choping 1 px bottom and right
//             // Actually, let's chop two pixels, toxav hates odd resolutions (off by one stride)
//             screen.setWidth((screen.width() * pixRatio) - 2);
//             screen.setHeight((screen.height() * pixRatio) - 2);
//         }
//         const std::string screenVideoSize =  QStringLiteral("%1x%2").arg(screen.width()).arg(screen.height()).toStdString();

//         devName += QString("+%1,%2").arg(QString().setNum(mode.x), QString().setNum(mode.y));


//     }
//     if (format->name == QString("video4linux2,v4l2") && mode) {
//         av_dict_set(&options, "video_size", videoSize.c_str(), 0);
//         av_dict_set(&options, "framerate", framerate.c_str(), 0);
//         const std::string pixelFormatStr =
//                 v4l2::getPixelFormatString(mode.pixel_format).toStdString();
//         // don't try to set a format string that doesn't exist
//         if (pixelFormatStr != "unknown" && pixelFormatStr != "invalid") {
//             const char* pixel_format = pixelFormatStr.c_str();
//             av_dict_set(&options, "pixel_format", pixel_format, 0);
//         }
//     }
// #endif
// #ifdef Q_OS_WIN
//     else if (devName.startsWith("gdigrab#")) {
//         const std::string offsetX = QString().setNum(mode.x).toStdString();
//         const std::string offsetY = QString().setNum(mode.y).toStdString();
//         av_dict_set(&options, "framerate", framerate.c_str(), 0);
//         av_dict_set(&options, "video_size", videoSize.c_str(), 0);
//         av_dict_set(&options, "offset_x", offsetX.c_str(), 0);
//         av_dict_set(&options, "offset_y", offsetY.c_str(), 0);
//     } else if (iformat->name == QString("dshow") && mode) {
//         // 可能存在设置异常问题
//         //    av_dict_set(&options, "video_size", videoSize.c_str(), 0);
//         //    av_dict_set(&options, "framerate", framerate.c_str(), 0);
//     }
// #endif
// #ifdef Q_OS_OSX
//     else if (iformat->name == QString("avfoundation")) {
//         if (mode) {
//             av_dict_set(&options, "video_size", videoSize.c_str(), 0);
//             av_dict_set(&options, "framerate", framerate.c_str(), 0);
//         } else if (devName.startsWith(avfoundation::CAPTURE_SCREEN)) {
//             av_dict_set(&options, "framerate", framerate.c_str(), 0);
//             av_dict_set_int(&options, "capture_cursor", 1, 0);
//             av_dict_set_int(&options, "capture_mouse_clicks", 1, 0);
//         }
//     }
// #endif
//     else if (mode) {
//         qWarning() << "Video mode-setting not implemented for input " << iformat->name;
//         Q_UNUSED(mode);
//     }

    // CameraDevice* dev0 = new CameraDevice();
    std::string msg;
    bool opened = open(videoDevice, &options, msg);
    if(!opened){
        qWarning() << "Device opened failed: " << msg.c_str();
    }
    //
    return opened;
}

/**
 * @brief Closes the device. Never fails.
 * @note If returns true, "this" becomes invalid.
 * @return True, if device finally deleted (closed last reference),
 * false otherwise (if other references exist).
 */
bool CameraDevice::close() {
    qDebug() << __func__ << videoDevice.name;

    QMutexLocker locker(&openDeviceLock);

    handler = nullptr;

    avformat_close_input(&context);
    context = nullptr;

    avcodec_free_context(&cctx);
    cctx = nullptr;

    av_dict_free(&options);
    options = nullptr;


    qDebug() << "Device: " << videoDevice.name << " closed.";

    // delete this;
    return true;
}

/**
 * @brief Get raw device list
 * @note Uses avdevice_list_devices
 * @return Raw device list
 */
// QVector<QPair<QString, QString>> CameraDevice::getRawDeviceListGeneric() {
//     QVector<QPair<QString, QString>> devices;

//             // if (!getDefaultInputFormat()) return devices;

//             // Alloc an input device context
//     AVFormatContext* s;
//     if (!(s = avformat_alloc_context())) return devices;

//     if (!format->priv_class || !AV_IS_INPUT_DEVICE(format->priv_class->category)) {
//         avformat_free_context(s);
//         return devices;
//     }

//     s->iformat = format;
//     if (s->iformat->priv_data_size > 0) {
//         s->priv_data = av_mallocz(s->iformat->priv_data_size);
//         if (!s->priv_data) {
//             avformat_free_context(s);
//             return devices;
//         }
//         if (s->iformat->priv_class) {
//             *(const AVClass**)s->priv_data = s->iformat->priv_class;
//             av_opt_set_defaults(s->priv_data);
//         }
//     } else {
//         s->priv_data = nullptr;
//     }

//             // List the devices for this context
//     AVDeviceInfoList* devlist = nullptr;
//     AVDictionary* tmp = nullptr;
//     av_dict_copy(&tmp, nullptr, 0);
//     if (av_opt_set_dict2(s, &tmp, AV_OPT_SEARCH_CHILDREN) < 0) {
//         av_dict_free(&tmp);
//         avformat_free_context(s);
//         return devices;
//     }
//     avdevice_list_devices(s, &devlist);
//     av_dict_free(&tmp);
//     avformat_free_context(s);
//     if (!devlist) {
//         qWarning() << "avdevice_list_devices failed";
//         return devices;
//     }

//             // Convert the list to a QVector
//     devices.resize(devlist->nb_devices);
//     for (int i = 0; i < devlist->nb_devices; ++i) {
//         AVDeviceInfo* dev = devlist->devices[i];
//         devices[i].first = dev->device_name;
//         devices[i].second = dev->device_description;
//     }
//     avdevice_free_list_devices(&devlist);
//     return devices;
// }

/**
 * @brief Get device list with desciption
 * @return A list of device names and descriptions.
 * The names are the first part of the pair and can be passed to open(QString).
 */
QVector<VideoDevice> CameraDevice::getDeviceList() {
    QVector<VideoDevice> devices;
    for(auto i = 0; i < sizeof(VideoType); ++i){
        auto type = (VideoType)i;
        auto iformat = getDefaultInputFormat(type);
        if(!iformat){
            continue;
        }
#ifdef Q_OS_LINUX
        //获取摄像头
        if(iformat->name == QString("video4linux2,v4l2")){
            for(auto p: v4l2::getDeviceList()){
                devices += VideoDevice{
                        .type = type,
                        .name = p.second,
                        .url = p.first
                };
            }
        }
        // 获取屏幕数量
        if(iformat->name == QString("x11grab")){
            auto count = ok::base::X11Display::Count();
            for (size_t c = 0; c < count; ++c) {
                QString dev = ":" + QString::number(c);
                QString name = QString("Desktop %1").arg(c);
                devices.push_back(VideoDevice{.type = type, .name = name, .url=dev});
            }
        }
#endif  // Q_OS_LINUX

#ifdef Q_OS_WIN
        //摄像头
        if (iformat->name == QString("dshow")){
            for(auto p: DirectShow::getDeviceList()){
                devices += VideoDevice{
                        .type = type,
                        .name = p.second,
                        .url = p.first
                };
            }
        }

        //屏幕
        if (iformat->name == QString("gdigrab")){
            QString url = "gdigrab#desktop";
            QString name = "Desktop";
            devices.push_back(VideoDevice{.type = type, .name = name, .url=dev});
        }
#endif  // Q_OS_WIN


// #ifdef Q_OS_OSX
    // if (iformat->name == QString("avfoundation"))
            // devices += avfoundation::getDeviceList();
// #endif
    }
            // devices += getRawDeviceListGeneric();

    return devices;
}

/**
 * @brief Get the default device name.
 * @return The short name of the default device
 * This is either the device in the settings or the system default.
 */
QString CameraDevice::getDefaultDeviceName() {
    auto& s = lib::settings::OkSettings::getInstance();

    QString defaultdev = s.getVideoDev();
    qDebug() << "Get the set video device:" << defaultdev;
    // if (!getDefaultInputFormat()) return defaultdev;

    auto devlist = getDeviceList();
    for (auto & device : devlist)
        if (defaultdev == device.name)
            return defaultdev;

    if (devlist.isEmpty()) return defaultdev;

    if (devlist.size() > 1) {
        return devlist[1].name;
    }
    return devlist[0].name;
}

/**
 * @brief Checks if a device name specifies a display.
 * @param devName Device name to check.
 * @return True, if device is screen, false otherwise.
 */
bool CameraDevice::isScreen(const QString& devName) {
    return devName.startsWith("x11grab") || devName.startsWith("gdigrab");
}

/**
 * @brief Get list of resolutions and position of screens
 * @return Vector of avaliable screen modes with offset
 */
QVector<VideoMode> CameraDevice::getScreenModes() {
    QList<QScreen*> screens = QApplication::screens();
    QVector<VideoMode> result;

    std::for_each(screens.begin(), screens.end(), [&result](QScreen* s) {
        QRect rect = s->geometry();
        QPoint p = rect.topLeft();
        qreal pixRatio = s->devicePixelRatio();

        VideoMode mode(rect.width() * pixRatio, rect.height() * pixRatio, p.x() * pixRatio,
                       p.y() * pixRatio);
        result.push_back(mode);
    });

    return result;
}

/**
 * @brief Get the list of video modes for a device.
 * @param devName Device name to get nodes from.
 * @return Vector of available modes for the device.
 */
QVector<VideoMode> CameraDevice::getVideoModes() {

    auto devName = videoDevice.name;
    auto iformat = getDefaultInputFormat(videoDevice.type);
    if(!iformat){
        return {};
    }

    //如果是屏幕
    if (isScreen(videoDevice.url))
        return getScreenModes();

#ifdef Q_OS_WIN
    if (iformat->name == QString("dshow"))
        return DirectShow::getDeviceModes(devName);
#endif
#if USING_V4L
    if (iformat->name == QString("video4linux2,v4l2"))
        return v4l2::getDeviceModes(devName);
#endif
#ifdef Q_OS_OSX
    if (iformat->name == QString("avfoundation"))
        return avfoundation::getDeviceModes(devName);
#endif
    else
        qWarning() << "Video mode listing not implemented for input " << iformat->name;

    return {};
}

/**
 * @brief Get the name of the pixel format of a video mode.
 * @param pixel_format Pixel format to get the name from.
 * @return Name of the pixel format.
 */
QString CameraDevice::getPixelFormatString(uint32_t pixel_format) {
#if USING_V4L
    return v4l2::getPixelFormatString(pixel_format);
#else
    return QString("unknown");
#endif
}

/**
 * @brief Compare two pixel formats.
 * @param a First pixel format to compare.
 * @param b Second pixel format to compare.
 * @return True if we prefer format a to b,
 * false otherwise (such as if there's no preference).
 */
bool CameraDevice::betterPixelFormat(uint32_t a, uint32_t b) {
#if USING_V4L
    return v4l2::betterPixelFormat(a, b);
#else
    return false;
#endif
}

/**
 * @brief Sets CameraDevice::iformat to default.
 * @return True if success, false if failure.
 */
const AVInputFormat* CameraDevice::getDefaultInputFormat(VideoType type) {
    // QMutexLocker locker(&iformatLock);
    avdevice_register_all();
    switch (type) {
        // Webcam input formats
        case VideoType::Camera:{
#if USING_V4L
        auto iformat = av_find_input_format("v4l2");
        if(iformat) return iformat;
#endif

#ifdef Q_OS_WIN
        auto iformat = av_find_input_format("dshow");
        if(iformat) return iformat;
        iformat = av_find_input_format("vfwcap");
        if(iformat) return iformat;
#endif

#ifdef Q_OS_OSX
        auto iformat = av_find_input_format("avfoundation");
        if(iformat) return iformat;
        iformat = av_find_input_format("qtkit");
        if(iformat) return iformat;
#endif
        break;
        }

        // Desktop capture input formats
        case VideoType::Desktop:{
#if USING_V4L
        auto iformat = av_find_input_format("x11grab");
        if(iformat) return iformat;
#endif

#ifdef Q_OS_WIN
        auto iformat = av_find_input_format("gdigrab");
        if(iformat) return iformat;
#endif
        break;
        }

        case VideoType::Stream:

            break;
        case VideoType::File:
            break;
    }

    return nullptr;
}
}  // namespace lib::video
