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
#include "webrtc/webrtc.h"

namespace lib::ortc {

static OkRTCManager* instance = nullptr;
static std::recursive_mutex mtx;

OkRTCManager::OkRTCManager() {}

OkRTCManager::~OkRTCManager() {
    rtc.reset();
}

OkRTCManager* OkRTCManager::getInstance() {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (!instance) {
        RTC_LOG(LS_INFO) << "Creating instance.";
        instance = new OkRTCManager();
    }
    return instance;
}

void OkRTCManager::destroyInstance() {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (!instance) {
        RTC_LOG(LS_WARNING) << "The instance has been destroyed!";
        return;
    }

    RTC_LOG(LS_INFO) << "instance:" << instance;
    delete instance;
    instance = nullptr;
    RTC_LOG(LS_WARNING) << "Destroy the instance successfully.";
}

OkRTC* OkRTCManager::createRtc(Mode mode, const std::string& res) {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (!rtc) {
        rtc = std::make_unique<WebRTC>(mode, res);
        if (!_iceOptions.empty()) {
            rtc->setIceServers(_iceOptions);
        }
    }
    return rtc.get();
}

void OkRTCManager::destroyRtc() {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    rtc.reset();
}

OkRTC* OkRTCManager::getRtc() {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    return rtc.get();
}

void OkRTCManager::setIceServers(const std::vector<IceServer>& ices) {
    _iceOptions = ices;
    if (rtc) {
        rtc->setIceServers(ices);
    }
}

std::map<std::string, OIceUdp> OkRTCManager::getCandidates(const std::string& peerId) {
    return rtc->getCandidates(peerId);
}

void OkRTCManager::SessionTerminate(const std::string& sid) {
    rtc->SessionTerminate(sid);
}

void OkRTCManager::setEnable(CtrlState state) {
    rtc->setEnable(state);
}

size_t OkRTCManager::getVideoSize() {
    return rtc->getVideoSize();
}

}  // namespace lib::ortc
