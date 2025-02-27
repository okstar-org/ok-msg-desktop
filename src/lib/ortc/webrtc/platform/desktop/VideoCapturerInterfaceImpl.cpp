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

#include "VideoCapturerInterfaceImpl.h"

#include "VideoCameraCapturer.h"
#include "VideoCapturerTrackSource.h"

#include "pc/video_track_source_proxy.h"
#include "webrtc/platform/desktop_capturer/DesktopCaptureSourceHelper.h"

namespace lib::ortc {
namespace {

std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> GetSink(
        const rtc::scoped_refptr<webrtc::VideoTrackSourceInterface>& nativeSource) {
    const auto proxy = static_cast<webrtc::VideoTrackSourceProxy*>(nativeSource.get());
    const auto internal = static_cast<VideoCapturerTrackSource*>(proxy->internal());
    return internal->sink();
}

}  // namespace

VideoCapturerInterfaceImpl::VideoCapturerInterfaceImpl(
        rtc::Thread* signalingThread, rtc::Thread* workerThread,
        rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source,
        std::string deviceId,
        std::function<void(VideoState)> stateUpdated,
        std::shared_ptr<PlatformContext> platformContext, std::pair<int, int>& outResolution)
        : _source(source), _sink(GetSink(source)), _stateUpdated(stateUpdated) {

#ifdef OK_RTC_UWP_DESKTOP
    // Windows uwp
    if (deviceId == "GraphicsCaptureItem")
    {
        auto uwpContext = std::static_pointer_cast<UwpContext>(platformContext);
        _screenCapturer = std::make_un                                            ique<UwpScreenCapturer>(_sink, uwpContext->item);
        _screenCapturer->setState(VideoState::Active);
        outResolution = _screenCapturer->resolution();
    }
    else
#else
    // Linux
    if (const auto source = DesktopCaptureSourceForKey(deviceId)) {
        const auto data = DesktopCaptureSourceData{
                                                   /*.aspectSize = */{ 1280, 720 },
                                                   /*.fps = */24.,
                                                   /*.captureMouse = */(deviceId != "desktop_capturer_pipewire"),
                                                   };
        _desktopCapturer = std::make_unique<DesktopCaptureSourceHelper>(source, data);
        _desktopCapturer->setOutput(_sink);
        _desktopCapturer->start();
        outResolution = { 1280, 960 };
    } else if (!ShouldBeDesktopCapture(deviceId))
#endif
    {
        _cameraCapturer = std::make_unique<VideoCameraCapturer>(signalingThread, workerThread, _sink);
        _cameraCapturer->setDeviceId(deviceId);
        _cameraCapturer->setState(VideoState::Active);
        outResolution = _cameraCapturer->resolution();
    }
}

VideoCapturerInterfaceImpl::~VideoCapturerInterfaceImpl() {
    RTC_LOG(LS_INFO) << __func__;
}

void VideoCapturerInterfaceImpl::setState(VideoState state) {
#ifdef OK_RTC_UWP_DESKTOP
    if (_screenCapturer) {
        _screenCapturer->setState(state);
    } else
#else
    if (_desktopCapturer) {
        if (state == VideoState::Active) {
            _desktopCapturer->start();
        } else {
            _desktopCapturer->stop();
        }
    } else
#endif  // OK_RTC_UWP_DESKTOP
        if (_cameraCapturer) {
            _cameraCapturer->setState(state);
        }
    if (_stateUpdated) {
        _stateUpdated(state);
    }
}

void VideoCapturerInterfaceImpl::setPreferredCaptureAspectRatio(float aspectRatio) {
    if (_cameraCapturer) {
        _cameraCapturer->setPreferredCaptureAspectRatio(aspectRatio);
    }
}

void VideoCapturerInterfaceImpl::setUncroppedOutput(
        std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink) {
    if (_uncroppedSink != nullptr) {
        _source->RemoveSink(_uncroppedSink.get());
    }
    _uncroppedSink = sink;
    if (_uncroppedSink != nullptr) {
        _source->AddOrUpdateSink(_uncroppedSink.get(), rtc::VideoSinkWants());
    }
}

void VideoCapturerInterfaceImpl::setOnFatalError(std::function<void()> error) {
#ifdef OK_RTC_UWP_DESKTOP
    if (_screenCapturer) {
        _screenCapturer->setOnFatalError(std::move(error));
    } else if (!_screenCapturer && !_cameraCapturer && error) {
        error();
    }
#else   // OK_RTC_UWP_DESKTOP
    if (_desktopCapturer) {
        _desktopCapturer->setOnFatalError(std::move(error));
    } else if (!_desktopCapturer && !_cameraCapturer && error) {
        error();
    }
#endif  // OK_RTC_UWP_DESKTOP
    if (_cameraCapturer) {
        _cameraCapturer->setOnFatalError(std::move(error));
    }
}

void VideoCapturerInterfaceImpl::setOnPause(std::function<void(bool)> pause) {
#ifdef OK_RTC_UWP_DESKTOP
    if (_screenCapturer) {
        _screenCapturer->setOnPause(std::move(pause));
    }
#else   // OK_RTC_UWP_DESKTOP
    if (_desktopCapturer) {
        _desktopCapturer->setOnPause(std::move(pause));
    }
#endif  // OK_RTC_UWP_DESKTOP
}

}  // namespace lib::ortc
