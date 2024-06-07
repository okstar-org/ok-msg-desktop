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
#ifndef TEST_VCM_CAPTURER_H_
#define TEST_VCM_CAPTURER_H_

#include <memory>
#include <vector>

#include "api/scoped_refptr.h"
#include "modules/video_capture/video_capture.h"
#include "test_video_capturer.h"


namespace lib {
namespace ortc {

using namespace webrtc;

class VcmCapturer :
        public rtc::VideoSinkInterface<VideoFrame>,
        public TestVideoCapturer                    {
public:
    VcmCapturer(VideoCaptureModule::DeviceInfo*);
    ~VcmCapturer();

    bool Create(size_t width,
                        size_t height,
                        size_t target_fps,
                        size_t capture_device_index);


    void OnFrame(const VideoFrame &frame) override;

    void Destroy();

private:
    bool Init(size_t width, size_t height, size_t target_fps,
              size_t capture_device_index);

    VideoCaptureModule::DeviceInfo* deviceInfo_;
    rtc::scoped_refptr<VideoCaptureModule> vcm_;
    VideoCaptureCapability capability_;
};

} // namespace ortc
} // namespace lib

#endif // TEST_VCM_CAPTURER_H_
