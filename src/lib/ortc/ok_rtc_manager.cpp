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

#include "ok_rtc_manager.h"

#include <memory>

#include "ok_rtc.h"
#include "ok_rtc_defs.h"
#include "ok_rtc_renderer.h"
#include "webrtc/webrtc.h"

namespace lib {
namespace ortc {

static OkRTCManager* instance = nullptr;
static std::mutex mtx;

OkRTCManager::OkRTCManager() {}

OkRTCManager::~OkRTCManager() { rtc.reset(); }

OkRTCManager* OkRTCManager::getInstance() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!instance) {
        RTC_DLOG_F(LS_INFO) << "Creating instance.";
        instance = new OkRTCManager();
    }
    return instance;
}

void OkRTCManager::destroyInstance() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!instance) {
        RTC_DLOG_F(LS_WARNING) << "The instance has been destroyed!";
        return;
    }

    RTC_DLOG_F(LS_INFO) << "instance:" << instance;
    delete instance;
    instance = nullptr;
    RTC_DLOG_F(LS_WARNING) << "Destroy the instance successfully.";
}

OkRTC* OkRTCManager::getRtc() {
    if (!rtc) {
        rtc = std::make_unique<WebRTC>();
        rtc->setIceOptions(_iceOptions);
    }
    return rtc.get();
}

void OkRTCManager::destroyRtc() { rtc.reset(); }

void OkRTCManager::addIceServer(const IceServer& ice) { _iceOptions.push_back(ice); }

void OkRTCManager::CreateOffer(const std::string& peerId) { rtc->CreateOffer(peerId); }

void OkRTCManager::SessionTerminate(const std::string& sid) { rtc->SessionTerminate(sid); }

void OkRTCManager::setMute(bool mute) { rtc->setMute(mute); }

void OkRTCManager::setRemoteMute(bool mute) {}

size_t OkRTCManager::getVideoSize() { return rtc->getVideoSize(); }

}  // namespace ortc
}  // namespace lib
