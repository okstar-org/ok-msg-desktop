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
    static OkRTCManager* getInstance(std::list<IceServer> iceServers,
                                     OkRTCHandler *handler,
                                     OkRTCRenderer *renderer);
  ~OkRTCManager();



  OkRTC* createInstance();
  void destroyInstance(OkRTC*);

  void start(const std::string &peerId,
             const std::string &sId,
             JingleCallType callType);

  void join(const std::string &peerId,
            const std::string &sId,
            const JingleContext &context);

  void quit(const std::string &peerId);

  size_t getVideoSize();

  void createPeerConnection();

  void SetRemoteDescription(const std::string &peerId,
                            const lib::ortc::JingleContext &jingleContext);

  void CreateOffer(const std::string &peerId);

  void CreateOffer(const std::string &peerId,
                   const lib::ortc::JingleContext &pContent);

  void CreateAnswer(const std::string &peerId,
                    const lib::ortc::JingleContext &pContent);

  bool SetTransportInfo(const std::string &peerId, const OIceUdp &oIceUdp);

  void ContentAdd(std::map<std::string, gloox::Jingle::Session> &sdMap,
                  ortc::OkRTCHandler *handler);

  void ContentRemove(std::map<std::string, gloox::Jingle::Session> &sdMap,
                     ortc::OkRTCHandler *handler);

  void SessionTerminate(const std::string &peerId);

  void setMute(bool mute);
  void setRemoteMute(bool mute);

private:
  OkRTCManager(std::list<IceServer> iceServers,
               OkRTCHandler *handler,
               OkRTCRenderer *renderer);

  std::unique_ptr<OkRTC> rtc;
  std::list<IceServer> iceServers;
  OkRTCHandler *handler;
  OkRTCRenderer *renderer;
};

} // namespace ortc
} // namespace lib
