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

#ifndef ORTC_VIDEO_CAPTURE_INTERFACE_IMPL_H
#define ORTC_VIDEO_CAPTURE_INTERFACE_IMPL_H

#include <api/media_stream_interface.h>
#include <api/scoped_refptr.h>
#include <memory>
#include "VideoCaptureInterface.h"

namespace lib::ortc {

class Threads;
class VideoCapturerInterface;

class VideoCaptureInterfaceObject {
public:
    VideoCaptureInterfaceObject(rtc::Thread* signalingThread, rtc::Thread* workerThread,
                                std::string deviceId, bool isScreenCapture,
                                std::shared_ptr<PlatformContext> platformContext);
    ~VideoCaptureInterfaceObject();

    void switchToDevice(std::string deviceId, bool isScreenCapture);
    void withNativeImplementation(std::function<void(void*)> completion);
    void setState(VideoState state);
    void setPreferredAspectRatio(float aspectRatio);
    void setOutput(std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink);
    void setStateUpdated(std::function<void(VideoState)> stateUpdated);
    void setRotationUpdated(std::function<void(int)> rotationUpdated);
    void setOnFatalError(std::function<void()> error);
    void setOnPause(std::function<void(bool)> pause);
    void setOnIsActiveUpdated(std::function<void(bool)> onIsActiveUpdated);
    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source();
    int getRotation();
    bool isScreenCapture();

private:
    void updateAspectRateAdaptation();

    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> _videoSource;
    std::weak_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> _currentUncroppedSink;
    std::shared_ptr<PlatformContext> _platformContext;
    std::pair<int, int> _videoCapturerResolution;
    std::unique_ptr<VideoCapturerInterface> _videoCapturer;
    std::function<void(VideoState)> _stateUpdated;
    std::function<void()> _onFatalError;
    std::function<void(bool)> _onPause;
    std::function<void(bool)> _onIsActiveUpdated;
    std::function<void(int)> _rotationUpdated;
    VideoState _state = VideoState::Active;
    float _preferredAspectRatio = 0.0f;
    bool _shouldBeAdaptedToReceiverAspectRate = true;
    bool _isScreenCapture = false;
    rtc::Thread* signalingThread;
    rtc::Thread* workerThread;
};

class VideoCaptureInterfaceImpl : public VideoCaptureInterface {
public:
    VideoCaptureInterfaceImpl(rtc::Thread* signalingThread, rtc::Thread* workerThread,
                              std::string deviceId, bool isScreenCapture,
                              std::shared_ptr<PlatformContext> platformContext);
    virtual ~VideoCaptureInterfaceImpl();

    void switchToDevice(std::string deviceId, bool isScreenCapture) override;
    void withNativeImplementation(std::function<void(void*)> completion) override;
    void setState(VideoState state) override;
    void setPreferredAspectRatio(float aspectRatio) override;
    void setOutput(std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink) override;
    void setOnFatalError(std::function<void()> error) override;
    void setOnPause(std::function<void(bool)> pause) override;
    void setOnIsActiveUpdated(std::function<void(bool)> onIsActiveUpdated) override;
    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source() override;
    VideoCaptureInterfaceObject* object();

private:
    VideoCaptureInterfaceObject* _impl;
};

}  // namespace lib::ortc

#endif
