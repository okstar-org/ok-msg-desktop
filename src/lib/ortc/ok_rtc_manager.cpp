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


#include "ok_rtc_defs.h"
#include "ok_rtc_proxy.h"
#include "ok_rtc_renderer.h"
#include "webrtc/ok_rtc.h"

namespace lib {
namespace ortc {

OkRTCManager::OkRTCManager(std::list<IceServer> iceServers,
                           OkRTCHandler *handler,
                           OkRTCRenderer *renderer) {
  rtcProxy = std::make_unique<ORTC>(iceServers, handler, renderer);
}

OkRTCManager::~OkRTCManager() {

}

void OkRTCManager::start(const std::string &peerId,
                         const std::string &sId,
                         lib::ortc::JingleCallType callType){
  rtcProxy->call(peerId, sId, callType);
}

void OkRTCManager::join(const std::string &peerId,
                        const std::string &sId,
                        const JingleContext& context) {
  rtcProxy->join(peerId, sId, context);
}

void OkRTCManager::quit(const std::string &peerId) {
  rtcProxy->quit(peerId);
}

void OkRTCManager::createPeerConnection() {
  rtcProxy->createPeerConnection();
}

void OkRTCManager::SetRemoteDescription(
    const std::string &peerId, const lib::ortc::JingleContext &jingleContext) {
  rtcProxy->SetRemoteDescription(peerId, jingleContext);
}

void OkRTCManager::CreateOffer(const std::string &peerId) {
  rtcProxy->CreateOffer(peerId);
}

void OkRTCManager::CreateAnswer(const std::string &peerId, const lib::ortc::JingleContext &pContent) {
  rtcProxy->CreateAnswer(peerId, pContent);
}

bool OkRTCManager::SetTransportInfo(const std::string &peerId, const OIceUdp &oIceUdp) {
  return rtcProxy->SetTransportInfo(peerId, oIceUdp);
}

void OkRTCManager::ContentAdd(
    std::map<std::string, gloox::Jingle::Session> &sdMap,
    ortc::OkRTCHandler *handler) {
  rtcProxy->ContentAdd(sdMap, handler);
}

void OkRTCManager::ContentRemove(
    std::map<std::string, gloox::Jingle::Session> &sdMap,
    ortc::OkRTCHandler *handler) {
  rtcProxy->ContentRemove(sdMap, handler);
}

void OkRTCManager::SessionTerminate(const std::string &sid) {
  rtcProxy->SessionTerminate(sid);
}

void OkRTCManager::setMute(bool mute) { rtcProxy->setMute(mute); }

void OkRTCManager::setRemoteMute(bool mute) { rtcProxy->setRemoteMute(mute); }

size_t OkRTCManager::getVideoSize() { return rtcProxy->getVideoSize(); }



} // namespace ortc
} // namespace lib
