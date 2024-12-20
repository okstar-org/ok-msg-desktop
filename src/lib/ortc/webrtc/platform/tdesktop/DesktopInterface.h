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

#ifndef TGCALLS_DESKTOP_INTERFACE_H
#define TGCALLS_DESKTOP_INTERFACE_H

#include <memory>
#include "../../VideoCapturerInterface.h"
#include "../PlatformInterface.h"
#include "webrtc/platform/PlatformInterface.h"

namespace lib::ortc {

class DesktopInterface : public PlatformInterface {
public:
    std::unique_ptr<webrtc::VideoEncoderFactory> makeVideoEncoderFactory(
            bool preferHardwareEncoding = false, bool isScreencast = false) override;
    std::unique_ptr<webrtc::VideoDecoderFactory> makeVideoDecoderFactory() override;
    bool supportsEncoding(const std::string& codecName) override;
    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> makeVideoSource(
            rtc::Thread* signalingThread, rtc::Thread* workerThread) override;
    void adaptVideoSource(rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> videoSource,
                          int width, int height, int fps) override;
    std::unique_ptr<VideoCapturerInterface> makeVideoCapturer(
            rtc::Thread* signalingThread, rtc::Thread* workerThread,
            rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source, std::string deviceId,
            std::function<void(VideoState)> stateUpdated,
            std::function<void(PlatformCaptureInfo)> captureInfoUpdated,
            std::shared_ptr<PlatformContext> platformContext,
            std::pair<int, int>& outResolution) override;
};

}  // namespace lib::ortc

#endif  // TGCALLS_DESKTOP_INTERFACE_H
