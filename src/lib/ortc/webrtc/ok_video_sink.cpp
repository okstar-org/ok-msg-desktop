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

namespace lib {
namespace ortc {

VideoSink::VideoSink(OkRTCHandler* handler, std::string peerId)
        : handler(handler), _peer_id(std::move(peerId)) {
    RTC_DCHECK(handler);
}

VideoSink::~VideoSink() {}

void VideoSink::OnFrame(const webrtc::VideoFrame& frame) {
    bool conv = false;
    ortc::RendererImage _image{};

    auto v_buffer = frame.video_frame_buffer();
    switch (v_buffer->type()) {
        case webrtc::VideoFrameBuffer::Type::kI420: {
            auto i420 = v_buffer->GetI420();
            if (frame.rotation() != webrtc::kVideoRotation_0) {
                webrtc::VideoFrame rotated_frame =
                        webrtc::VideoFrame::Builder()
                                .set_video_frame_buffer(
                                        webrtc::I420Buffer::Rotate(*v_buffer, frame.rotation()))
                                .set_rotation(webrtc::kVideoRotation_0)
                                .set_timestamp_us(frame.timestamp_us())
                                .set_id(frame.id())
                                .build();
                auto i420_ = rotated_frame.video_frame_buffer()->GetI420();
                _image.width_ = static_cast<size_t>(i420_->width());
                _image.height_ = static_cast<size_t>(i420_->height());
                _image.y = const_cast<uint8_t*>(i420_->DataY());
                _image.u = const_cast<uint8_t*>(i420_->DataU());
                _image.v = const_cast<uint8_t*>(i420_->DataV());
                _image.ystride = i420_->StrideY();
                _image.ustride = i420_->StrideU();
                _image.vstride = i420_->StrideV();
            } else {
                _image.width_ = static_cast<size_t>(i420->width());
                _image.height_ = static_cast<size_t>(i420->height());
                _image.y = const_cast<uint8_t*>(i420->DataY());
                _image.u = const_cast<uint8_t*>(i420->DataU());
                _image.v = const_cast<uint8_t*>(i420->DataV());
                _image.ystride = i420->StrideY();
                _image.ustride = i420->StrideU();
                _image.vstride = i420->StrideV();
            }
        }
            conv = true;
            break;
        default:
            RTC_LOG(LS_WARNING) << "Not supported frame type: "
                                << webrtc::VideoFrameBufferTypeToString(v_buffer->type());
            break;
    }

    if (!conv) {
        return;
    }

    handler->onRender(_peer_id, _image);

    // 渲染次数计数
    _renderCount++;
}

}  // namespace ortc
}  // namespace lib
