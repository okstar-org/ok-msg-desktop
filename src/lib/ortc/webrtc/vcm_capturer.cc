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

#include "vcm_capturer.h"

#include <iostream>
#include <stdint.h>
#include <memory>

#include "modules/video_capture/video_capture_factory.h"

namespace lib {
namespace ortc {

VcmCapturer::VcmCapturer(VideoCaptureModule::DeviceInfo* deviceInfo) :
    deviceInfo_(deviceInfo),
    vcm_(nullptr) {
}

VcmCapturer::~VcmCapturer() { Destroy(); }


bool VcmCapturer::Init(size_t width,
                       size_t height,
                       size_t target_fps,
                       size_t capture_device_index) {

  char device_name[256];
  char unique_name[256];
  int32_t r = deviceInfo_->GetDeviceName(static_cast<uint32_t>(capture_device_index),
                                 device_name,
                                 sizeof(device_name),
                                 unique_name,
                                 sizeof(unique_name));
  if(r){
//    qDebug() <<"Failed to GetDeviceName, result=>" << r;
    return false ;
  }
//  qDebug() << "Device device name is:"<< device_name << "unique name is:" << unique_name;
  vcm_ = webrtc::VideoCaptureFactory::Create(unique_name);
  if (!vcm_) {
//    qWarning() <<"Cannot create capturer for device:" << unique_name;
    return false;
  }

  vcm_->RegisterCaptureDataCallback(this);

  deviceInfo_->GetCapability(vcm_->CurrentDeviceName(), 0, capability_);

//  capability_.width = static_cast<int32_t>(width);
//  capability_.height = static_cast<int32_t>(height);
  capability_.maxFPS = static_cast<int32_t>(target_fps);
  capability_.videoType = VideoType::kI420;

  int sc = vcm_->StartCapture(capability_);
  if (sc != 0) {
    std::cerr << "Cannot start capture.";
    Destroy();
    return false;
  }

  if (vcm_->CaptureStarted()) {
    std::cerr << "Capture is not be started.";
    return false;
  }

  return true;
}

bool VcmCapturer::Create(size_t width,
                                 size_t height,
                                 size_t target_fps,
                                 size_t capture_device_index) {

    bool yes = Init(width, height, target_fps, capture_device_index);
    if(!yes)
      std::cerr << "Failed to create VcmCapturer(w = " << width
                        << ", h = " << height << ", fps = " << target_fps
                        << ")";
    return yes;
}

void VcmCapturer::Destroy() {
  if (!vcm_.get())
    return;

  vcm_->StopCapture();
  vcm_->DeRegisterCaptureDataCallback();
  // Release reference to VCM.
  vcm_ = nullptr;
}


void VcmCapturer::OnFrame(const VideoFrame &frame) {
  TestVideoCapturer::OnFrame(frame);
}

} // namespace ortc
} // namespace lib
