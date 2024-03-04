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

#include "IMJingleSession.h"
#include "lib/ortc/ok_rtc_proxy.h"
#include "lib/ortc/ok_rtc_renderer.h"
#include "base/basic_types.h"
#include <QDebug>

namespace lib {
namespace messenger {

IMJingleSession::IMJingleSession(const std::string &peerId,
                                 const std::string &sId_,
                                 lib::ortc::JingleCallType callType,
                                 Session *mSession,
                                 std::list<ortc::IceServer> iceServers,
                                 ortc::OkRTCHandler *handler,
                                 ortc::OkRTCRenderer *renderer)
    : sId(sId_),
      session(mSession),                                                 //
      accepted(false),                                                   //
      _callDirection(CallOut)                                            //
{
  qDebug() << "Create jingle session type:" << qstring(JingleCallTypeToString(callType))
           <<" for peer:"<<qstring(peerId);

  if(callType == ortc::JingleCallType::audio || callType == ortc::JingleCallType::video){
    _rtcManager = std::make_unique<lib::ortc::OkRTCManager>(iceServers, handler, renderer); //
    _rtcManager->start(peerId, sId, callType);
  }
  qDebug() << "Jingle session be created:" << qstring(sId);
}

IMJingleSession::~IMJingleSession() {
  qDebug() << "Jingle session has be destroyed:" << qstring(sId);
}

Session *IMJingleSession::getSession() const { return session; }

const Session::Jingle *IMJingleSession::getJingle() const { return jingle; }

void IMJingleSession::setJingle(const Session::Jingle *jingle) {
  IMJingleSession::jingle = jingle;
}

void IMJingleSession::setContext(const ortc::JingleContext &jc) {
  context = jc;
}

} // namespace IM
} // namespace lib