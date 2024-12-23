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
#ifndef OK_RTC_DESKTOP_CAPTURE_SOURCE_HELPER_H__
#define OK_RTC_DESKTOP_CAPTURE_SOURCE_HELPER_H__

#include "DesktopCaptureSource.h"

#include <functional>
#include <memory>

namespace webrtc {
class VideoFrame;
}  // namespace webrtc

namespace rtc {
template <typename T> class VideoSinkInterface;
}  // namespace rtc

namespace lib::ortc {

DesktopCaptureSource DesktopCaptureSourceForKey(const std::string& uniqueKey);
bool ShouldBeDesktopCapture(const std::string& uniqueKey);

class DesktopCaptureSourceHelper {
public:
    DesktopCaptureSourceHelper(DesktopCaptureSource source, DesktopCaptureSourceData data);
    ~DesktopCaptureSourceHelper();

    void setOutput(std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink) const;
    void setSecondaryOutput(
            std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink) const;
    void start() const;
    void stop() const;
    void setOnFatalError(std::function<void()>) const;
    void setOnPause(std::function<void(bool)>) const;

private:
    struct Renderer;
    std::shared_ptr<Renderer> _renderer;
};

}  // namespace lib::ortc

#endif  // OK_RTC_DESKTOP_CAPTURE_SOURCE_HELPER_H__
