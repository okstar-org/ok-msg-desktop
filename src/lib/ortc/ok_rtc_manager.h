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

#pragma once

#include <memory>
#include <string>

#include "ok_rtc.h"
#include "ok_rtc_renderer.h"

namespace lib {
namespace ortc {

class OkRTCManager {
public:
    static OkRTCManager* getInstance();
    static void destroyInstance();

    ~OkRTCManager();

    OkRTC* getRtc();
    void destroyRtc();

    void addIceServer(const IceServer& ice);

    size_t getVideoSize();

    void CreateOffer(const std::string& peerId);

    void CreateOffer(const std::string& peerId, const lib::ortc::OJingleContent& pContent);


    void SessionTerminate(const std::string& peerId);

    void setMute(bool mute);
    void setRemoteMute(bool mute);

private:
    OkRTCManager();

    std::list<IceServer> _iceOptions;
    std::unique_ptr<OkRTC> rtc;
};

}  // namespace ortc
}  // namespace lib
