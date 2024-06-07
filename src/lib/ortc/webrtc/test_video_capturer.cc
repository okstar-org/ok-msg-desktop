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

#include "test_video_capturer.h"

#include <algorithm>

#include "api/scoped_refptr.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_rotation.h"

namespace lib {
namespace ortc {

using namespace webrtc;


TestVideoCapturer::~TestVideoCapturer() = default;

//void TestVideoCapturer::OnOutputFormatRequest(
//    int width, int height, const absl::optional<int> &max_fps) {
//  absl::optional<std::pair<int, int>> target_aspect_ratio =
//      std::make_pair(width, height);
//  absl::optional<int> max_pixel_count = width * height;
//  video_adapter_.OnOutputFormatRequest(target_aspect_ratio, max_pixel_count,
//                                       max_fps);
//}

void TestVideoCapturer::OnFrame(const VideoFrame &original_frame) {
  int cropped_width = 0;
  int cropped_height = 0;
  int out_width = 0;
  int out_height = 0;

  VideoFrame frame = MaybePreprocess(original_frame);

  if (!video_adapter_.AdaptFrameResolution(
          frame.width(), frame.height(), frame.timestamp_us() * 1000,
          &cropped_width, &cropped_height, &out_width, &out_height)) {
    // Drop frame in order to respect frame rate constraint.
    return;
  }

  if (out_height != frame.height() || out_width != frame.width()) {
    // Video adapter has requested a down-scale. Allocate a new buffer and
    // return scaled version.
    // For simplicity, only scale here without cropping.
    rtc::scoped_refptr<I420Buffer> scaled_buffer =
        I420Buffer::Create(out_width, out_height);
    scaled_buffer->ScaleFrom(*frame.video_frame_buffer()->ToI420());
    VideoFrame::Builder new_frame_builder =
        VideoFrame::Builder()
            .set_video_frame_buffer(scaled_buffer)
            .set_rotation(kVideoRotation_0)
            .set_timestamp_us(frame.timestamp_us())
            .set_id(frame.id());
//    TODO disable 20240305
    //    if (frame.has_update_rect()) {
//      VideoFrame::UpdateRect new_rect = frame.update_rect().ScaleWithFrame(
//          frame.width(), frame.height(), 0, 0, frame.width(), frame.height(),
//          out_width, out_height);
//      new_frame_builder.set_update_rect(new_rect);
//    }
    broadcaster_.OnFrame(new_frame_builder.build());

  } else {
    // No adaptations needed, just return the frame as is.
    broadcaster_.OnFrame(frame);
  }
}

rtc::VideoSinkWants TestVideoCapturer::GetSinkWants() {
  return broadcaster_.wants();
}

TestVideoCapturer::TestVideoCapturer()
{

}

void TestVideoCapturer::AddOrUpdateSink(
    rtc::VideoSinkInterface<VideoFrame> *sink,
    const rtc::VideoSinkWants &wants) {
  broadcaster_.AddOrUpdateSink(sink, wants);
  UpdateVideoAdapter();
}

void TestVideoCapturer::RemoveSink(rtc::VideoSinkInterface<VideoFrame> *sink) {
  broadcaster_.RemoveSink(sink);
  UpdateVideoAdapter();
}

void TestVideoCapturer::UpdateVideoAdapter() {
  video_adapter_.OnSinkWants(broadcaster_.wants());
}

VideoFrame TestVideoCapturer::MaybePreprocess(const VideoFrame &frame) {
  MutexLock lock(&lock_);
  if (preprocessor_ != nullptr) {
    return preprocessor_->Preprocess(frame);
  } else {
    return frame;
  }
}

} // namespace ortc
} // namespace lib
