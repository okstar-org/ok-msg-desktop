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

#include "VideoCaptureInterfaceImpl.h"

#include "StaticThreads.h"
#include "VideoCapturerInterface.h"
#include "platform/PlatformInterface.h"

namespace lib::ortc {

VideoCaptureInterfaceObject::VideoCaptureInterfaceObject(rtc::Thread* signalingThread,
                                                         rtc::Thread* workerThread,
                                                         std::string deviceId,
                                                         bool isScreenCapture,
                                                         std::shared_ptr<PlatformContext>
                                                                 platformContext)
        : _videoSource(PlatformInterface::SharedInstance()->makeVideoSource(signalingThread,
                                                                            workerThread))
        , signalingThread(signalingThread)
        , workerThread(workerThread) {
    _platformContext = platformContext;
    switchToDevice(deviceId, isScreenCapture);
}

VideoCaptureInterfaceObject::~VideoCaptureInterfaceObject() {
    if (_videoCapturer) {
        _videoCapturer->setUncroppedOutput(nullptr);
    }
}

rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> VideoCaptureInterfaceObject::source() {
    return _videoSource;
}

int VideoCaptureInterfaceObject::getRotation() {
    if (_videoCapturer) {
        return _videoCapturer->getRotation();
    } else {
        return 0;
    }
}

bool VideoCaptureInterfaceObject::isScreenCapture() {
    return _isScreenCapture;
}

void VideoCaptureInterfaceObject::switchToDevice(std::string deviceId, bool isScreenCapture) {
    if (!_videoSource) return;

    _isScreenCapture = isScreenCapture;
    if (_videoCapturer) {
        _videoCapturer->setUncroppedOutput(nullptr);
    }

    // this should outlive the capturer
    _videoCapturer = nullptr;
    _videoCapturer = PlatformInterface::SharedInstance()->makeVideoCapturer(
            signalingThread,
            workerThread,
            _videoSource,
            deviceId,
            [this](VideoState state) {
                if (this->_stateUpdated) {
                    this->_stateUpdated(state);
                }
                if (this->_onIsActiveUpdated) {
                    switch (state) {
                        case VideoState::Active: {
                            this->_onIsActiveUpdated(true);
                            break;
                        }
                        default: {
                            this->_onIsActiveUpdated(false);
                            break;
                        }
                    }
                }
            },
            [this](PlatformCaptureInfo info) {
                if (this->_shouldBeAdaptedToReceiverAspectRate !=
                    info.shouldBeAdaptedToReceiverAspectRate) {
                    this->_shouldBeAdaptedToReceiverAspectRate =
                            info.shouldBeAdaptedToReceiverAspectRate;
                }
                if (this->_rotationUpdated) {
                    this->_rotationUpdated(info.rotation);
                }
                this->updateAspectRateAdaptation();
            },
            _platformContext,
            _videoCapturerResolution);

    if (_videoCapturer) {
        if (_preferredAspectRatio > 0) {
            _videoCapturer->setPreferredCaptureAspectRatio(_preferredAspectRatio);
        }
        if (const auto currentUncroppedSink = _currentUncroppedSink.lock()) {
            _videoCapturer->setUncroppedOutput(currentUncroppedSink);
        }
        if (_onFatalError) {
            _videoCapturer->setOnFatalError(_onFatalError);
        }
        if (_onPause) {
            _videoCapturer->setOnPause(_onPause);
        }
        _videoCapturer->setState(_state);
    }
}

void VideoCaptureInterfaceObject::withNativeImplementation(std::function<void(void*)> completion) {
    if (_videoCapturer) {
        _videoCapturer->withNativeImplementation(completion);
    } else {
        completion(nullptr);
    }
}

void VideoCaptureInterfaceObject::setState(VideoState state) {
    if (_state != state) {
        _state = state;
        if (_videoCapturer) {
            _videoCapturer->setState(state);
        }
    }
}

void VideoCaptureInterfaceObject::setPreferredAspectRatio(float aspectRatio) {
    _preferredAspectRatio = aspectRatio;
    updateAspectRateAdaptation();
}

void VideoCaptureInterfaceObject::updateAspectRateAdaptation() {
    if (_videoCapturer) {
        if (_videoCapturerResolution.first != 0 && _videoCapturerResolution.second != 0) {
            if (_preferredAspectRatio > 0.01 && _shouldBeAdaptedToReceiverAspectRate) {
                float originalWidth = (float)_videoCapturerResolution.first;
                float originalHeight = (float)_videoCapturerResolution.second;

                float aspectRatio = _preferredAspectRatio;

                float width = (originalWidth > aspectRatio * originalHeight)
                                      ? int(std::round(aspectRatio * originalHeight))
                                      : originalWidth;
                float height = (originalWidth > aspectRatio * originalHeight)
                                       ? originalHeight
                                       : int(std::round(originalHeight / aspectRatio));

                PlatformInterface::SharedInstance()->adaptVideoSource(
                        _videoSource, (int)width, (int)height, 25);
            } else {
                PlatformInterface::SharedInstance()->adaptVideoSource(
                        _videoSource,
                        _videoCapturerResolution.first,
                        _videoCapturerResolution.second,
                        25);
            }
        }
    }
}

void VideoCaptureInterfaceObject::setOutput(
        std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink) {
    if (_videoCapturer) {
        _videoCapturer->setUncroppedOutput(sink);
    }
    _currentUncroppedSink = sink;
}

void VideoCaptureInterfaceObject::setOnFatalError(std::function<void()> error) {
    if (_videoCapturer) {
        _videoCapturer->setOnFatalError(error);
    }
    _onFatalError = error;
}
void VideoCaptureInterfaceObject::setOnPause(std::function<void(bool)> pause) {
    if (_videoCapturer) {
        _videoCapturer->setOnPause(pause);
    }
    _onPause = pause;
}

void VideoCaptureInterfaceObject::setOnIsActiveUpdated(
        std::function<void(bool)> onIsActiveUpdated) {
    _onIsActiveUpdated = onIsActiveUpdated;
}

void VideoCaptureInterfaceObject::setStateUpdated(std::function<void(VideoState)> stateUpdated) {
    _stateUpdated = stateUpdated;
}

void VideoCaptureInterfaceObject::setRotationUpdated(std::function<void(int)> rotationUpdated) {
    _rotationUpdated = rotationUpdated;
}

VideoCaptureInterfaceImpl::VideoCaptureInterfaceImpl(rtc::Thread* signalingThread,
                                                     rtc::Thread* workerThread,
                                                     std::string deviceId,
                                                     bool isScreenCapture,
                                                     std::shared_ptr<PlatformContext>
                                                             platformContext)
        : _impl(  // threads->getMediaThread(),
                  // [deviceId, isScreenCapture, platformContext,
                  // threads]() {
                  //	return
                  new VideoCaptureInterfaceObject(
                          signalingThread, workerThread, deviceId, isScreenCapture, platformContext)
                  //}
          ) {}

VideoCaptureInterfaceImpl::~VideoCaptureInterfaceImpl() {
    delete _impl;
};

void VideoCaptureInterfaceImpl::switchToDevice(std::string deviceId, bool isScreenCapture) {
    //    	_impl.perform(RTC_FROM_HERE, [deviceId, isScreenCapture](VideoCaptureInterfaceObject
    //    *impl)
    //{

    _impl->switchToDevice(deviceId, isScreenCapture);
    //	});
}

void VideoCaptureInterfaceImpl::withNativeImplementation(std::function<void(void*)> completion) {
    //    _impl.perform(RTC_FROM_HERE, [completion](VideoCaptureInterfaceObject *impl) {
    _impl->withNativeImplementation(completion);
    //    });
}

void VideoCaptureInterfaceImpl::setState(VideoState state) {
    //	_impl.perform(RTC_FROM_HERE, [state](VideoCaptureInterfaceObject *impl) {
    _impl->setState(state);
    //	});
}

void VideoCaptureInterfaceImpl::setPreferredAspectRatio(float aspectRatio) {
    //    _impl.perform(RTC_FROM_HERE, [aspectRatio](VideoCaptureInterfaceObject *impl) {
    _impl->setPreferredAspectRatio(aspectRatio);
    //    });
}
void VideoCaptureInterfaceImpl::setOnFatalError(std::function<void()> error) {
    //    _impl.perform(RTC_FROM_HERE, [error](VideoCaptureInterfaceObject *impl) {
    _impl->setOnFatalError(error);
    //    });
}
void VideoCaptureInterfaceImpl::setOnPause(std::function<void(bool)> pause) {
    //    _impl.perform(RTC_FROM_HERE, [pause](VideoCaptureInterfaceObject *impl) {
    _impl->setOnPause(pause);
    //    });
}

void VideoCaptureInterfaceImpl::setOnIsActiveUpdated(std::function<void(bool)> onIsActiveUpdated) {
    //    _impl.perform(RTC_FROM_HERE, [onIsActiveUpdated](VideoCaptureInterfaceObject *impl) {
    _impl->setOnIsActiveUpdated(onIsActiveUpdated);
    //    });
}

void VideoCaptureInterfaceImpl::setOutput(
        std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink) {
    //	_impl.perform(RTC_FROM_HERE, [sink](VideoCaptureInterfaceObject *impl) {
    _impl->setOutput(sink);
    //	});
}

VideoCaptureInterfaceObject* VideoCaptureInterfaceImpl::object() {
    return _impl;
}

rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> VideoCaptureInterfaceImpl::source() {
    return _impl->source();
}

}  // namespace lib::ortc
