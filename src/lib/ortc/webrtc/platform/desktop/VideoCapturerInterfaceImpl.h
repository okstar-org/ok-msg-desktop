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

#ifndef OK_RTC_VIDEO_CAPTURER_INTERFACE_IMPL_H
#define OK_RTC_VIDEO_CAPTURER_INTERFACE_IMPL_H

#include <rtc_base/thread.h>
#include "../../VideoCapturerInterface.h"

#ifdef OK_RTC_UWP_DESKTOP
#include "platform/uwp/UwpContext.h"
#include "platform/uwp/UwpScreenCapturer.h"
#endif  // OK_RTC_UWP_DESKTOP

#include "api/media_stream_interface.h"

namespace lib::ortc {

class DesktopCaptureSourceHelper;
class VideoCameraCapturer;
class PlatformContext;

class VideoCapturerInterfaceImpl final : public VideoCapturerInterface {
public:
    VideoCapturerInterfaceImpl(rtc::Thread* signalingThread,
                               rtc::Thread* workerThread,
                               rtc::scoped_refptr<webrtc::VideoTrackSourceInterface>
                                       source,
                               std::string deviceId,
                               std::function<void(VideoState)>
                                       stateUpdated,
                               std::shared_ptr<PlatformContext>
                                       platformContext,
                               std::pair<int, int>& outResolution);
    ~VideoCapturerInterfaceImpl() override;

    void setState(VideoState state) override;
    void setPreferredCaptureAspectRatio(float aspectRatio) override;
    void setUncroppedOutput(
            std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink) override;
    int getRotation() override {
        return 0;
    }
    void setOnFatalError(std::function<void()> error) override;
    void setOnPause(std::function<void(bool)> pause) override;

private:
    rtc::Thread* signalingThread;
    rtc::Thread* workerThread;
    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> _source;
    std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> _sink;
#ifdef OK_RTC_UWP_DESKTOP
    std::unique_ptr<UwpScreenCapturer> _screenCapturer;
#else   // OK_RTC_UWP_DESKTOP
    std::unique_ptr<DesktopCaptureSourceHelper> _desktopCapturer;
#endif  // OK_RTC_UWP_DESKTOP
    std::unique_ptr<VideoCameraCapturer> _cameraCapturer;
    std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> _uncroppedSink;
    std::function<void(VideoState)> _stateUpdated;
    std::function<void()> _onFatalError;
};

}  // namespace lib::ortc

#endif
