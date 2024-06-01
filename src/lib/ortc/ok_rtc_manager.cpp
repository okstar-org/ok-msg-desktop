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

OkRTCManager::OkRTCManager(std::list<IceServer> iceServers,
                           OkRTCHandler *handler,
                           OkRTCRenderer *renderer)
    : iceServers{iceServers}, handler{handler}, renderer{renderer}
{

}

OkRTCManager *OkRTCManager::getInstance(std::list<IceServer> iceServers, OkRTCHandler *handler, OkRTCRenderer *renderer)
{
    static OkRTCManager *instance = nullptr;
    if(!instance){
        instance = new OkRTCManager(iceServers, handler, renderer);
    }
    return instance;
}

OkRTCManager::~OkRTCManager() {

}

OkRTC *OkRTCManager::createInstance()
{
    rtc = std::make_unique<WebRTC>(iceServers, handler, renderer);
    return rtc.get();
}

void OkRTCManager::destroyInstance(OkRTC *rtc_)
{
   rtc.reset();
}

void OkRTCManager::start(const std::string &peerId,
                         const std::string &sId,
                         lib::ortc::JingleCallType callType){
  rtc->call(peerId, sId, callType);
}

void OkRTCManager::join(const std::string &peerId,
                        const std::string &sId,
                        const JingleContext& context) {
  rtc->join(peerId, sId, context);
}

void OkRTCManager::quit(const std::string &peerId) {
  rtc->quit(peerId);
}

void OkRTCManager::createPeerConnection() {
  rtc->createPeerConnection();
}

void OkRTCManager::SetRemoteDescription(
    const std::string &peerId, const lib::ortc::JingleContext &jingleContext) {
  rtc->SetRemoteDescription(peerId, jingleContext);
}

void OkRTCManager::CreateOffer(const std::string &peerId) {
  rtc->CreateOffer(peerId);
}

void OkRTCManager::CreateAnswer(const std::string &peerId, const lib::ortc::JingleContext &pContent) {
  rtc->CreateAnswer(peerId, pContent);
}

bool OkRTCManager::SetTransportInfo(const std::string &peerId, const OIceUdp &oIceUdp) {
  return rtc->SetTransportInfo(peerId, oIceUdp);
}

void OkRTCManager::ContentAdd(
    std::map<std::string, gloox::Jingle::Session> &sdMap,
    ortc::OkRTCHandler *handler) {
  rtc->ContentAdd(sdMap, handler);
}

void OkRTCManager::ContentRemove(
    std::map<std::string, gloox::Jingle::Session> &sdMap,
    ortc::OkRTCHandler *handler) {
  rtc->ContentRemove(sdMap, handler);
}

void OkRTCManager::SessionTerminate(const std::string &sid) {
  rtc->SessionTerminate(sid);
}

void OkRTCManager::setMute(bool mute) { rtc->setMute(mute); }

void OkRTCManager::setRemoteMute(bool mute) {  }

size_t OkRTCManager::getVideoSize() { return rtc->getVideoSize(); }


} // namespace ortc
} // namespace lib
