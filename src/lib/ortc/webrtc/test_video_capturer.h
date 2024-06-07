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
#ifndef TEST_TEST_VIDEO_CAPTURER_H_
#define TEST_TEST_VIDEO_CAPTURER_H_

#include <stddef.h>

#include <memory>

#include <pc/video_track_source.h>

#include "api/video/video_frame.h"
#include "api/video/video_source_interface.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"
#include "rtc_base/synchronization/mutex.h"

namespace lib {
namespace ortc {

using namespace webrtc;

class TestVideoCapturer : public rtc::VideoSourceInterface<VideoFrame> {
public:
    class FramePreprocessor {
    public:
        virtual ~FramePreprocessor() = default;
        virtual VideoFrame Preprocess(const VideoFrame &frame) = 0;
    };

  TestVideoCapturer();

  ~TestVideoCapturer();

  void AddOrUpdateSink(rtc::VideoSinkInterface<VideoFrame> *sink,
                       const rtc::VideoSinkWants &wants) override;

  void RemoveSink(rtc::VideoSinkInterface<VideoFrame> *sink) override;

  void SetFramePreprocessor(std::unique_ptr<FramePreprocessor> preprocessor) {
    MutexLock lock(&lock_);
    preprocessor_ = std::move(preprocessor);
  }
//  void OnOutputFormatRequest(int width, int height,
//                             const absl::optional<int> &max_fps);

protected:
  void OnFrame(const VideoFrame &frame);
  rtc::VideoSinkWants GetSinkWants();

private:
  void UpdateVideoAdapter();
  VideoFrame MaybePreprocess(const VideoFrame &frame);

  Mutex lock_;
  std::unique_ptr<FramePreprocessor> preprocessor_ RTC_GUARDED_BY(lock_);
  rtc::VideoBroadcaster broadcaster_;
  cricket::VideoAdapter video_adapter_;
};
} // namespace ortc
} // namespace lib

#endif // TEST_TEST_VIDEO_CAPTURER_H_
