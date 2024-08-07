//
//  DesktopCaptureSourceManager.h
//  TgVoipWebrtc
//
//  Created by Mikhail Filimonov on 28.12.2020.
//  Copyright © 2020 Mikhail Filimonov. All rights reserved.
//
#ifndef TGCALLS_DESKTOP_CAPTURE_SOURCE_MANAGER_H__
#define TGCALLS_DESKTOP_CAPTURE_SOURCE_MANAGER_H__

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

#endif  // TGCALLS_DESKTOP_CAPTURE_SOURCE_MANAGER_H__
