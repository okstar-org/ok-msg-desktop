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
#include <QMap>
#include "base/basic_types.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "lib/ortc/ok_rtc_manager.h"
#include <jinglesession.h>

namespace ortc{
struct IceServer;
}

namespace lib {
namespace messenger {

enum CallStage {
  StageNone,
  StageMessage, // XEP-0353: Jingle Message Initiation
  StageSession  // XEP-0166: Jingle https://xmpp.org/extensions/xep-0166.html
};

enum CallDirection { CallNone, CallIn, CallOut };

using namespace gloox;
using namespace gloox::Jingle;

class IMJingleSession {

public:
  explicit IMJingleSession(const std::string &peerId,
                           const std::string &sId,
                           lib::ortc::JingleCallType callType,
                           Session *mSession,
                           std::list<ortc::IceServer> iceServers,
                           ortc::OkRTCHandler *handler,
                           ortc::OkRTCRenderer *renderer);
  virtual ~IMJingleSession();


  [[nodiscard]] Session *getSession() const;
  [[nodiscard]] inline const ortc::JingleContext &getContext() const {
    return context;
  }

  void setContext(const ortc::JingleContext&);

  const Session::Jingle *getJingle() const;
  void setJingle(const Session::Jingle *jingle);

  [[nodiscard]] CallDirection direction() const {
    return _callDirection;
  }

  void setDirection(CallDirection direction){
    _callDirection = direction;
  }

  void setAccepted(bool y) { accepted = y; }

  [[nodiscard]] bool isAccepted() const { return accepted; }

  [[nodiscard]] ortc::OkRTCManager *getRtcManager(){return _rtcManager.get();}

  const std::string & getId() const {
    return sId;
  }

  void appendIce(const ortc::OIceUdp& ice){
    pendingIceCandidates.emplace_back(ice);
  }

  void pollIce(Fn<void(const ortc::OIceUdp&)> fn){
    while (!pendingIceCandidates.empty()){
      fn(pendingIceCandidates.back());
      pendingIceCandidates.pop_back();
    }
  }
private:
  std::string sId;
  Session *session;
  const Session::Jingle *jingle;
  ortc::JingleContext context;

  CallStage m_callStage;
  bool accepted;

  std::unique_ptr<ortc::OkRTCManager> _rtcManager;
  CallDirection _callDirection;

  std::list<ortc::OIceUdp> pendingIceCandidates;
};

} // namespace IM
} // namespace lib
