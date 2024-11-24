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

//
// Created by gaojie on 24-11-23.
//

#include "IMConference.h"
#include "IM.h"
#include "application.h"
#include "src/base/uuid.h"

namespace lib::messenger {

IMConference::IMConference(IM* im, QObject* parent) : QObject(parent), im{im} {}

IMConference::~IMConference() {
    qDebug() << __func__;
}

const Conference& IMConference::create(const QString& name) {
    qDebug() << __func__ << name;

    auto session = ok::Application::Instance()->getSession();
    qDebug() << "Username:" << session->getToken().username;

    conference = std::make_unique<Conference>();
    conference->jid = name + "@conference." + session->getSignInInfo().host;
    conference->uid = ok::base::UUID::make();
    conference->startAudioMuted = 9;
    conference->startVideoMuted = 9;
    conference->rtcstatsEnabled = false;

    auto id = im->createMsgId();

    QString xml = QString("<iq id='%7' to='focus.meet.chuanshaninfo.com' from='%6' type='set' "
                          "xmlns='jabber:client'>"
                          "<conference machine-uid='%1'"
                          "  room='%2'"
                          "  xmlns='http://jitsi.org/protocol/focus'>"
                          "  <property name='startAudioMuted' value='%3'/>"
                          "  <property name='startVideoMuted' value='%4'/>"
                          "   <property name='rtcstatsEnabled' value='%5'/>"
                          "   </conference>"
                          "   </iq>")
                          .arg(conference->uid)
                          .arg(conference->jid.bare())
                          .arg(conference->startAudioMuted)
                          .arg(conference->startVideoMuted)
                          .arg(conference->rtcstatsEnabled ? "true" : "false")
                          .arg(qstring(im->self().full()))
                          .arg(id);

    qDebug() << xml;
    im->send(xml);

    return *conference;
}

}  // namespace lib::messenger
