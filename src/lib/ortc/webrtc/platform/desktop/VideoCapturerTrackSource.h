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

#ifndef OK_RTC_VIDEO_CAPTURER_TRACK_SOURCE_H
#define OK_RTC_VIDEO_CAPTURER_TRACK_SOURCE_H

#include "api/video/video_sink_interface.h"
#include "media/base/video_broadcaster.h"
#include "pc/video_track_source.h"

#include "VideoCameraCapturer.h"

namespace lib::ortc {

class VideoCameraCapturer;
class DesktopCapturer;

class VideoCapturerTrackSource : public webrtc::VideoTrackSource {
public:
    VideoCapturerTrackSource();

    std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink();

private:
    rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override;

    std::shared_ptr<rtc::VideoBroadcaster> _broadcaster;
};

}  // namespace lib::ortc

#endif
