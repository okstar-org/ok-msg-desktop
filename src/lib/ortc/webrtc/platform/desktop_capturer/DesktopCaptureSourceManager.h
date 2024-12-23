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
#ifndef OK_RTC_DESKTOP_CAPTURE_SOURCE_MANAGER_H__
#define OK_RTC_DESKTOP_CAPTURE_SOURCE_MANAGER_H__

#include "DesktopCaptureSource.h"
#include "DesktopCaptureSourceHelper.h"

#include <map>
#include <vector>

namespace webrtc {
class DesktopCapturer;
class DesktopCaptureOptions;
}  // namespace webrtc

namespace lib::ortc {

enum class DesktopCaptureType {
    Screen,
    Window,
};

class DesktopCaptureSourceManager {
public:
    explicit DesktopCaptureSourceManager(DesktopCaptureType type);
    ~DesktopCaptureSourceManager();

    std::vector<DesktopCaptureSource> sources();

private:
    static webrtc::DesktopCaptureOptions OptionsForType(DesktopCaptureType type);
    static std::unique_ptr<webrtc::DesktopCapturer> CreateForType(DesktopCaptureType type);

    std::unique_ptr<webrtc::DesktopCapturer> _capturer;
    DesktopCaptureType _type = DesktopCaptureType::Screen;
};

}  // namespace lib::ortc

#endif  // OK_RTC_DESKTOP_CAPTURE_SOURCE_MANAGER_H__
