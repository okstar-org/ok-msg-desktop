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

#include <capabilities.h>
#include <utility>

#include "IM.h"
#include "application.h"
#include "meetmanager.h"
#include "src/base/uuid.h"

namespace lib::messenger {

IMMeet::IMMeet(IM* im, QObject* parent) : QObject(parent), im{im}, manager{nullptr} {
    manager = new gloox::MeetManager(im->getClient());
    manager->registerHandler(this);

    connect(im, &IM::selfVCard, this, &IMMeet::onSelfVCard);

    // request self vcard.
    im->requestVCards();

    auto session = ok::Application::Instance()->getSession();
    qDebug() << "Username:" << session->getToken().username;

    auto host = stdstring("conference." + session->getSignInInfo().host);
    im->addFromHostHandler(host, this);
}

IMMeet::~IMMeet() {
    qDebug() << __func__;

    im->clearFromHostHandler();
    disconnect(im, &IM::selfVCard, this, &IMMeet::onSelfVCard);

    delete manager;
    manager = nullptr;
}

const std::string& IMMeet::create(const QString& name) {
    qDebug() << __func__ << name;

    auto session = ok::Application::Instance()->getSession();
    qDebug() << "Username:" << session->getToken().username;

    std::map<std::string, std::string> props;
    props.insert(std::pair("startAudioMuted", "9"));
    props.insert(std::pair("startVideoMuted", "9"));
    props.insert(std::pair("rtcstatsEnabled", "false"));

    gloox::JID jid(stdstring(name) + "@conference." + stdstring(session->getSignInInfo().host));
    meet = new gloox::Meet(jid, stdstring(ok::base::UUID::make()), props);
    manager->createMeet(*meet);
    return meet->getUid();
}

void IMMeet::disband() {}

void IMMeet::exit() {
    //    manager->exitMeet();
}

void IMMeet::join() {}

void IMMeet::handleHostPresence(const gloox::JID& from, const gloox::Presence& presence) {
    qDebug() << __func__ << qstring(from.full()) << "presence:" << presence.presence();
    auto caps = presence.capabilities();
    if (caps->node() != gloox::XMLNS_JITSI_MEET) {
        qWarning() << "Is not support meet.";
        return;
    }

    /**
     * <presence
to='sjdvr4swzf2f@meet.chuanshaninfo.com/OkMSG.root-host.[v25.03.00-3-15-g23458c4].OTE5Y2'
from='test@conference.meet.chuanshaninfo.com/46a04cab'> <stats-id>Chloe-ZsC</stats-id>
     <c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='https://jitsi.org/jitsi-meet'
        ver='p1SSmeQ82gSAjXEw+FlBmWtBv2k='/>
     <features>
        <feature var='https://jitsi.org/meet/e2ee'/>
     </features>
    <SourceInfo>{}</SourceInfo>
    <jitsi_participant_region>region1</jitsi_participant_region>
    <jitsi_participant_codecType>vp9</jitsi_participant_codecType>
    <avatar-url>data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL...</avatar-url>
    <email>eiilpux17@163.com</email>
    <nick xmlns='http://jabber.org/protocol/nick'>李佩旭</nick>
    <jitsi_participant_e2ee.idKey.curve25519>BLE6EaRXYhRdZH5mV2MRWjQmH8tlASmHUlHwNXc7cT8</jitsi_participant_e2ee.idKey.curve25519>
    <jitsi_participant_e2ee.idKey.ed25519>+ARDqdxd9wo41zBXzIIsVuZWdg5c7JHyaDG2THFeSMk</jitsi_participant_e2ee.idKey.ed25519>
    <x xmlns='http://jabber.org/protocol/muc#user'>
        <item jid='px0hzgu9bwzb@meet.chuanshaninfo.com/9f31d7f1-0644-4cfb-82e9-da69305ce32a'
        affiliation='none' role='participant'/>
    </x>
</presence>
*/
    auto pt = presence.subtype();
    auto t = presence.getOriginTag();
    switch (pt) {
        case gloox::Presence::PresenceType::Available: {
            // 成员上线
            auto email = t->findChild("email");
            if (email) {
                gloox::Meet::Participant participant = {
                        .region = t->findChild("jitsi_participant_region")->cdata(),
                        .codecType = t->findChild("jitsi_participant_codecType")->cdata(),
                        .avatarUrl = t->findChild("avatar-url")->cdata(),
                        .email = email->cdata(),
                        .nick = t->findChild("nick")->cdata(),
                        .resource = from.resource(),
                        .e2ee = false};

                auto fts = t->findChild("features");
                if (fts) {
                    auto e2ee = fts->findChild("feature", "var", "https://jitsi.org/meet/e2ee");
                    if (e2ee) {
                        participant.e2ee = true;
                        auto ed25519 = t->findChild("jitsi_participant_e2ee.idKey.ed25519");
                        if (ed25519) {
                            participant.idKeys.insert(std::make_pair("ed25519", ed25519->cdata()));
                        }
                        auto curve25519 = t->findChild("jitsi_participant_e2ee.idKey.curve25519");
                        if (curve25519) {
                            participant.idKeys.insert(
                                    std::make_pair("curve25519", curve25519->cdata()));
                        }
                    }
                }

                // 获取群组用户jid
                auto mucUser = t->findChild("x", "xmlns", "http://jabber.org/protocol/muc#user");
                if (mucUser) {
                    // participant.mucUser = *(mucUser);
                }
                meet->addParticipant(participant);
            }
        }
        case gloox::Presence::PresenceType::Unavailable: {
            // 成员下线
        }
        default: {
            qWarning() << "Unable to handle PresenceType:" << pt;
        }
    }

}  // namespace lib::messenger

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

    for (auto* h : handlers) {
        ok::base::Participant part = {.email = qstring(participant.email),
                                      .nick = qstring(participant.nick),
                                      .resource = qstring(participant.resource),
                                      .avatarUrl = participant.avatarUrl};
        h->onParticipantJoined(ok::base::Jid(jid.full()), part);
    }
}

void IMMeet::handleParticipant(const gloox::JID& jid, const gloox::Meet::Participant& participant) {
    qDebug() << __func__ << qstring(participant.email);
    for (auto* h : handlers) {
        ok::base::Participant part = {.email = qstring(participant.email),
                                      .nick = qstring(participant.nick),
                                      .resource = qstring(participant.resource),
                                      .avatarUrl = participant.avatarUrl};
        h->onParticipantJoined(ok::base::Jid(jid.full()), part);
    }
}
void IMMeet::handleStatsId(const gloox::JID& jid, const std::string& statsId) {}

void IMMeet::handleJsonMessage(const gloox::JID& jid, const gloox::JsonMessage* json) {
    qDebug() << __func__ << qstring(json->getJson());
}

void IMMeet::addMeetHandler(MessengerMeetHandler* hdr) {
    if (!hdr) return;
    handlers.push_back(hdr);
}

void IMMeet::onSelfVCard(const IMVCard& vCard_) {
    vCard = vCard_;
}

}  // namespace lib::messenger
