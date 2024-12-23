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

#ifndef OK_RTC_VIDEO_CAMERA_CAPTURER_H
#define OK_RTC_VIDEO_CAMERA_CAPTURER_H

#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_source_interface.h"
#include "media/base/video_adapter.h"
#include "modules/video_capture/video_capture.h"

#include "../../VideoCaptureInterface.h"

#include <stddef.h>
#include <memory>
#include <vector>

namespace lib::ortc {

class VideoCameraCapturer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    explicit VideoCameraCapturer(rtc::Thread* signalingThread, rtc::Thread* workerThread,
                                 std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink);
    ~VideoCameraCapturer();

    void setState(VideoState state);
    void setDeviceId(std::string deviceId);
    void setPreferredCaptureAspectRatio(float aspectRatio);
    void setOnFatalError(std::function<void()> error);

    std::pair<int, int> resolution() const;

    void OnFrame(const webrtc::VideoFrame& frame) override;

private:
    void create();
    bool create(webrtc::VideoCaptureModule::DeviceInfo* info, const std::string& deviceId);
    void destroy();
    void failed();

    rtc::Thread* signalingThread;
    rtc::Thread* workerThread;
    std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> _sink;
    rtc::scoped_refptr<webrtc::VideoCaptureModule> _module;
    webrtc::VideoCaptureCapability _capability;

    VideoState _state = VideoState::Inactive;
    std::string _requestedDeviceId;
    std::pair<int, int> _dimensions;
    std::function<void()> _error;
    float _aspectRatio = 0.;
    bool _failed = false;
};

}  // namespace lib::ortc

#endif
