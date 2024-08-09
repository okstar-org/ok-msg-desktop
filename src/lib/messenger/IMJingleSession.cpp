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
#include <lib/ortc/ok_rtc_defs.h>
#include <lib/ortc/ok_rtc_manager.h>
#include <QDebug>
#include "IMFile.h"
#include "IMFileTask.h"
#include "base/basic_types.h"
#include "lib/ortc/ok_rtc.h"
#include "lib/ortc/ok_rtc_renderer.h"

namespace lib {
namespace messenger {

IMJingleSession::IMJingleSession(const QString& sId_,
                                 Session* mSession,
                                 const IMContactId& selfId,
                                 const IMPeerId& peerId,
                                 lib::ortc::JingleCallType callType)
        : sId(sId_), session(mSession), selfId(selfId), accepted(false), m_callType{callType} {
    qDebug() << __func__ << "type:" << (int)m_callType << "sid:" << sId
             << "to peer:" << peerId.toString();
    qDebug() << __func__ << "be created.";
}

IMJingleSession::~IMJingleSession() { qDebug() << __func__ << sId; }

Session* IMJingleSession::getSession() const { return session; }

void IMJingleSession::onAccept() {
    // 对方接收
    qDebug() << __func__;
}

void IMJingleSession::onTerminate() {
    qDebug() << __func__;
    lib::ortc::OkRTCManager::getInstance()->destroyRtc();
}

void IMJingleSession::doTerminate() {
    qDebug() << __func__;

    // 发送结束协议
    session->sessionTerminate(new Session::Reason(Session::Reason::Reasons::Success));

    // 销毁rtc
    lib::ortc::OkRTCManager::getInstance()->destroyRtc();
}

void IMJingleSession::createOffer(const std::string& peerId) {
    qDebug() << __func__ << "to" << peerId.c_str();
    auto rm = lib::ortc::OkRTCManager::getInstance();
    auto r = rm->getRtc();
    r->CreateOffer(peerId);
}

const Session::Jingle* IMJingleSession::getJingle() const { return jingle; }

void IMJingleSession::setJingle(const Session::Jingle* jingle_) { jingle = jingle_; }

CallDirection IMJingleSession::direction() const {
    auto sender = session->initiator().bareJID();
    auto self = JID(stdstring(selfId.toString())).bareJID();
    return (sender == self) ? CallDirection::CallOut : CallDirection::CallIn;
}

void IMJingleSession::setCallStage(CallStage state) { m_callStage = state; }

void IMJingleSession::setContext(const ortc::OJingleContent& jc) { context = jc; }

void IMJingleSession::start() {}

void IMJingleSession::stop() {}

}  // namespace messenger
}  // namespace lib
