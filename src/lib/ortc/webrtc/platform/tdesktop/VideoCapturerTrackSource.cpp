#include "VideoCapturerTrackSource.h"

namespace lib::ortc {

VideoCapturerTrackSource::VideoCapturerTrackSource()
        : VideoTrackSource(/*remote=*/false)
        , _broadcaster(std::make_shared<rtc::VideoBroadcaster>()) {}

auto VideoCapturerTrackSource::sink()
        -> std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> {
    return _broadcaster;
}

rtc::VideoSourceInterface<webrtc::VideoFrame>* VideoCapturerTrackSource::source() {
    return _broadcaster.get();
}

}  // namespace lib::ortc
