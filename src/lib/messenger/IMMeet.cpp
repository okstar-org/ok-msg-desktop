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

#include "IMMeet.h"
#include "IM.h"
#include "application.h"
#include "meetmanager.h"
#include "src/base/uuid.h"

namespace lib::messenger {

IMMeet::IMMeet(IM* im, QObject* parent) : QObject(parent), im{im}, manager{nullptr} {
    manager = new gloox::MeetManager(im->getClient());
    manager->registerHandler(this);

    connect(im, &IM::selfVCard, [this](IMVCard vCard_) { vCard = vCard_; });

    // request self vcard.
    im->requestVCards();
}

IMMeet::~IMMeet() {
    qDebug() << __func__;
    delete manager;
    manager = nullptr;
}

const Meet& IMMeet::create(const QString& name) {
    qDebug() << __func__ << name;

    auto session = ok::Application::Instance()->getSession();
    qDebug() << "Username:" << session->getToken().username;

    conference = std::make_unique<Meet>();
    conference->jid = name + "@conference." + session->getSignInInfo().host;
    conference->uid = ok::base::UUID::make();
    conference->startAudioMuted = 9;
    conference->startVideoMuted = 9;
    conference->rtcstatsEnabled = false;

    gloox::JID jid = stdstring((name + "@conference." + session->getSignInInfo().host));

    std::map<std::string, std::string> props;
    props.insert(std::pair("startAudioMuted", "9"));
    props.insert(std::pair("startVideoMuted", "9"));
    props.insert(std::pair("rtcstatsEnabled", "false"));
    // focus.meet.chuanshaninfo.com
    gloox::Meet c(jid, stdstring(conference->uid), props);
    manager->createMeet(c);

    return *conference;
}

void IMMeet::disband() {}

void IMMeet::exit() {
    //    manager->exitMeet();
}

void IMMeet::join() {}

void IMMeet::handleCreation(const gloox::JID& jid, bool ready,
                            const std::map<std::string, std::string>& props) {
    qDebug() << __func__ << qstring(jid.full()) << "ready:" << ready;
    for (const auto& kv : props) {
        qDebug() << "property:" << qstring(kv.first) << "=>" << qstring(kv.second);
    }

    for (auto* h : handlers) {
        h->onMeetCreated(ok::base::Jid(jid.full()), ready, props);
    }

    // 加入到会议
    auto self = im->self();
    gloox::Meet meet(jid, "", {});
    gloox::Meet::Participant participant = {
            .region = "region1",
            .codecType = "vp8",
            .avatarUrl = stdstring(vCard.photo.url),
            .email = vCard.emails.isEmpty() ? "" : stdstring(vCard.emails.last().number),
            .nick = stdstring(vCard.nickname),
            .resource = self.resource(),
    };
    manager->join(meet, participant);
}

void IMMeet::handleParticipant(const gloox::Meet::Participant& participant) {
    qDebug() << __func__ << qstring(participant.nick);
}
void IMMeet::handleStatsId(const std::string& statsId) {}

void IMMeet::addMeetHandler(MessengerMeetHandler* hdr) {
    if (!hdr) return;
    handlers.push_back(hdr);
}

}  // namespace lib::messenger
