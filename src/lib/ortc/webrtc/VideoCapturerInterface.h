#ifndef ORTC_VIDEO_CAPTURER_INTERFACE_H
#define ORTC_VIDEO_CAPTURER_INTERFACE_H

#include "Instance.h"

#include <functional>
#include <memory>

namespace rtc {
template <typename VideoFrameT> class VideoSinkInterface;
}  // namespace rtc

namespace webrtc {
class VideoFrame;
}  // namespace webrtc

namespace lib::ortc {

class VideoCapturerInterface {
public:
    virtual ~VideoCapturerInterface() = default;

    virtual void setState(VideoState state) = 0;
    virtual void setPreferredCaptureAspectRatio(float aspectRatio) = 0;
    virtual void setUncroppedOutput(
            std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink) = 0;
    virtual int getRotation() = 0;
    virtual void setOnFatalError(std::function<void()> error) {
        // TODO: make this function pure virtual when everybody implements it.
    }
    virtual void setOnPause(std::function<void(bool)> pause) {
        // TODO: make this function pure virtual when everybody implements it.
    }
    virtual void withNativeImplementation(std::function<void(void*)> completion) {
        completion(nullptr);
    }
};

}  // namespace lib::ortc

#endif
