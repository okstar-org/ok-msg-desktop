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

#pragma once

#include <api/video/video_frame.h>
#include <api/video/video_sink_interface.h>
#include <ok_rtc.h>
#include <mutex>
#include "../ok_rtc_renderer.h"

namespace lib::ortc {

class VideoSink : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    VideoSink(const std::vector<OkRTCHandler*>& handlers, std::string peerId = {});

    ~VideoSink() override;

    virtual void OnFrame(const webrtc::VideoFrame& frame) override;

private:
    std::string _peer_id;
    const std::vector<OkRTCHandler*>& handlers;
};

}  // namespace lib::ortc
