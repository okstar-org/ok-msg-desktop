#include "VideoCaptureInterface.h"

#include "VideoCaptureInterfaceImpl.h"

namespace lib::ortc {

std::unique_ptr<VideoCaptureInterface> VideoCaptureInterface::Create(
        rtc::Thread* signalingThread, rtc::Thread* workerThread, std::string deviceId,
        bool isScreenCapture, std::shared_ptr<PlatformContext> platformContext) {
    return std::make_unique<VideoCaptureInterfaceImpl>(signalingThread, workerThread, deviceId,
                                                       isScreenCapture, platformContext);
}

VideoCaptureInterface::~VideoCaptureInterface() = default;

}  // namespace lib::ortc
