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

#include "ok_video_sink.h"

#include <api/video/i420_buffer.h>
#include <rtc_base/logging.h>

#include <utility>

namespace lib::ortc {

VideoSink::VideoSink(const std::vector<OkRTCHandler*>& handlers,  //
                     std::string peerId,                          //
                     std::string resource_)
        : handlers(handlers), _peer_id(std::move(peerId)), resource(std::move(resource_)) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " peerId:" << _peer_id << " mid:" << resource;
}
VideoSink::~VideoSink() {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " peerId:" << _peer_id << " mid:" << resource;
}
void VideoSink::OnFrame(const webrtc::VideoFrame& frame) {
    //    RTC_LOG(LS_INFO) << __FUNCTION__ << " peer:" << _peer_id << " mid:" << _mid
    //                     << " frame:" << frame.size();

    bool conv = false;
    RendererImage image{};
    auto v_buffer = frame.video_frame_buffer();
    switch (v_buffer->type()) {
        case webrtc::VideoFrameBuffer::Type::kI420: {
            auto i420 = v_buffer->GetI420();
            //            if (frame.rotation() != webrtc::kVideoRotation_0) {
            //                // 翻转
            //                webrtc::VideoFrame rotated_frame =
            //                        webrtc::VideoFrame::Builder()
            //                                .set_video_frame_buffer(
            //                                        webrtc::I420Buffer::Rotate(*v_buffer,
            //                                        frame.rotation()))
            //                                .set_rotation(webrtc::kVideoRotation_0)
            //                                .set_timestamp_us(frame.timestamp_us())
            //                                .set_id(frame.id())
            //                                .build();
            //                i420 = rotated_frame.video_frame_buffer()->GetI420();
            //            }
            image.width_ = static_cast<size_t>(i420->width());
            image.height_ = static_cast<size_t>(i420->height());
            image.y = const_cast<uint8_t*>(i420->DataY());
            image.u = const_cast<uint8_t*>(i420->DataU());
            image.v = const_cast<uint8_t*>(i420->DataV());
            image.ystride = i420->StrideY();
            image.ustride = i420->StrideU();
            image.vstride = i420->StrideV();
            conv = true;
            break;
        }
        default:
            RTC_LOG(LS_WARNING) << "Not supported frame type: "
                                << webrtc::VideoFrameBufferTypeToString(v_buffer->type());
            break;
    }

    if (!conv) {
        return;
    }

    for (auto handler : handlers) {
        handler->onRender(image, _peer_id, resource);
    }
}

}  // namespace lib::ortc
