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

IMJingleSession::IMJingleSession(IM* im,
                                 const IMPeerId& peerId,
                                 const QString& sId_,
                                 lib::ortc::JingleCallType callType,
                                 Session* mSession)
        : im{im}
        , sId(sId_)
        , session(mSession)
        , accepted(false)
        ,
        //      fileHandlers{fileHandlers},
        m_callType{callType} {
    qDebug() << __func__ << "type:" << (int)m_callType << "sid:" << sId
             << "to peer:" << peerId.toString();
    qDebug() << __func__ << "be created.";
}

IMJingleSession::~IMJingleSession() { qDebug() << __func__ << sId; }

Session* IMJingleSession::getSession() const { return session; }

void IMJingleSession::onAccept() {
    // 对方接收
    qDebug() << __func__;

    if (m_callType == lib::ortc::JingleCallType::file) {
        //  file
        //        for (auto& file : m_waitSendFiles) {
        //            doStartFileSendTask(session, file);
        //        }
    } else {
        // av
        auto peerId = session->remote().full();

        lib::ortc::OJingleContentAv cav;
        cav.sdpType = lib::ortc::JingleSdpType::Answer;
        cav.parse(jingle);

        // RTC 接受会话
        lib::ortc::OkRTCManager::getInstance()->getRtc()->setRemoteDescription(peerId, cav);

        //        emit receiveFriendHangup(
        //            peerId.username, answer.hasVideo() ? SENDING_V
        //                                               : SENDING_A);
    }
}

void IMJingleSession::onTerminate() {
    qDebug() << __func__;

    lib::ortc::OkRTCManager::getInstance()->destroyRtc();
}

void IMJingleSession::doTerminate() {
    qDebug() << __func__;

    // 发送结束协议
    session->sessionTerminate(new Session::Reason(Session::Reason::Reasons::Success));
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
    auto self = im->self().bareJID();
    return (sender == self) ? CallDirection::CallOut : CallDirection::CallIn;
}

void IMJingleSession::setCallStage(CallStage state) { m_callStage = state; }

void IMJingleSession::setContext(const ortc::OJingleContent& jc) { context = jc; }

}  // namespace messenger
}  // namespace lib
