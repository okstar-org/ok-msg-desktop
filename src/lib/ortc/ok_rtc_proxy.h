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

#include <string>

#include <gloox/client.h>
#include <gloox/jinglesession.h>

#include "ok_rtc_defs.h"
#include "ok_rtc_renderer.h"

namespace lib {
namespace ortc {

struct IceServer {
  std::string uri;
  std::string username;
  std::string password;
};

typedef struct join_options {
  std::string conference;
  std::string conference_id;
  std::string peer_name;
  long peer_id;
} JoinOptions;

class OkRTCHandler {
public:
  virtual void onCreatePeerConnection(const std::string &peerId,
                                      const std::string &sId, bool ok) = 0;


  virtual void onRTP(const std::string &sId,
                     const std::string &peerId,
                     const JingleContext &osd) = 0;

  virtual void onIce(const std::string &sId,
                     const std::string &peerId,
                     const OIceUdp &iceUdp) = 0;
};

class OkRTCProxy {

public:
  virtual void
  SetRemoteDescription(const std::string &peerId,
                       const lib::ortc::JingleContext &jingleContext) = 0;

  virtual void ContentAdd(std::map<std::string, gloox::Jingle::Session> sdMap,
                          OkRTCHandler *handler) = 0;

  virtual void
  ContentRemove(std::map<std::string, gloox::Jingle::Session> sdMap,
                OkRTCHandler *handler) = 0;

  virtual void SessionTerminate(const std::string &peerId) = 0;

  virtual void setMute(bool mute) = 0;

  virtual bool join(const std::string &peerId,
                    const std::string &sId,
                    const JingleContext &context) = 0;

  virtual bool call(const std::string &peerId,
                    const std::string &sId,
                    JingleCallType callType) = 0;

  virtual bool quit(const std::string &peerId) = 0;

  virtual void createPeerConnection() = 0;
  virtual size_t getVideoSize() = 0;

  virtual void CreateAnswer(const std::string &peerId,
                            const lib::ortc::JingleContext &pContent) = 0;
};

} // namespace ortc
} // namespace lib
