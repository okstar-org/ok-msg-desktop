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
#include "src/base/uuid.h"

#include <jinglejsonmessage.h>
#include <meetmanager.h>

namespace lib::messenger {

Participant IMMeet::toParticipant(const gloox::Meet::Participant& participant) const {
    return Participant{.email = qstring(participant.email),
                       .nick = qstring(participant.nick),
                       .resource = qstring(participant.resource),
                       .avatarUrl = participant.avatarUrl,
                       .jid = ok::base::Jid(participant.jid.full()),
                       .affiliation = qstring(participant.affiliation),
                       .role = qstring(participant.role)};
}

IMMeet::IMMeet(IM* im, QObject* parent) : IMJingle(im, parent), manager{nullptr} {
    manager = new gloox::MeetManager(im->getClient());
    manager->registerHandler(this);

    connect(im, &IM::selfVCard, this, &IMMeet::onSelfVCard);

    // request self vcard.
    im->requestVCards();

    auto session = ok::Application::Instance()->getSession();
    qDebug() << "Username:" << session->getToken().username;

    auto host = stdstring("conference." + session->getSignInInfo().host);
    im->addFromHostHandler(host, this);
    im->addSessionHandler(this);

    // jingle json-message
    im->sessionManager()->registerPlugin(new gloox::Jingle::JsonMessage());

    auto rtc = ortc::OkRTCManager::getInstance()->createRtc();
    rtc->addRTCHandler(this);
}

IMMeet::~IMMeet() {
    qDebug() << __func__;
    im->removeSessionHandler(this);
    im->clearFromHostHandler();
    disconnect(im, &IM::selfVCard, this, &IMMeet::onSelfVCard);

    delete manager;
    manager = nullptr;

    ortc::OkRTCManager* pRtcManager = ortc::OkRTCManager::getInstance();
    auto rtc = pRtcManager->getRtc();
    if (rtc) rtc->removeRTCHandler(this);
    pRtcManager->destroyRtc();
}

const std::string& IMMeet::create(const QString& name) {
    qDebug() << __func__ << name;

    auto session = ok::Application::Instance()->getSession();
    qDebug() << "Username:" << session->getToken().username;

    std::map<std::string, std::string> props;
    props.insert(std::pair("startAudioMuted", "9"));
    props.insert(std::pair("startVideoMuted", "9"));
    props.insert(std::pair("rtcstatsEnabled", "false"));

    gloox::JID room(stdstring(name) + "@conference." + stdstring(session->getSignInInfo().host));
    meet = manager->createMeet(room, props);

    return meet->getUid();
}

void IMMeet::disband() {}

void IMMeet::leave() {
    manager->exitMeet();
}

void IMMeet::join() {}

void IMMeet::handleHostPresence(const gloox::JID& from, const gloox::Presence& presence) {
    qDebug() << __func__ << qstring(from.full()) << "presence:" << presence.presence();
    //    auto caps = presence.capabilities();
    //    if (caps->node() != gloox::XMLNS_JITSI_MEET) {
    //        qWarning() << "Is not support meet.";
    //        return;
    //    }

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
                        .avatarUrl = t->findChild("avatar-url")
                                             ? t->findChild("avatar-url")->cdata()
                                             : "",
                        .email = email->cdata(),
                        .nick = t->findChild("nick") ? t->findChild("nick")->cdata() : "",
                        .resource = from.resource(),
                        .jid = from.full(),
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
                auto userTag = t->findChild("x", "xmlns", "http://jabber.org/protocol/muc#user");
                if (userTag) {
                    // <item
                    // jid='px0hzgu9bwzb@meet.chuanshaninfo.com/9f31d7f1-0644-4cfb-82e9-da69305ce32a'
                    // affiliation='none' role='participant'/>
                    gloox::MUCRoom::MUCUser mucUser(userTag);
                }
                meet->addParticipant(participant);

                for (auto* h : handlers) {
                    h->onParticipantJoined(ok::base::Jid(from.full()), toParticipant(participant));
                }
            }
            break;
        }
        case gloox::Presence::PresenceType::Unavailable: {
            // 成员离开 "<presence type='unavailable'
            // from='ykmfkvsa3t0f@meet.chuanshaninfo.com/19a59e74-0a10-4615-9f32-529969fcd59b'
            // to='sjdvr4swzf2f@meet.chuanshaninfo.com'/>"

            for (auto* h : handlers) {
                h->onParticipantLeft(ok::base::Jid(from.full()), qstring(from.resource()));
            }
            break;
        }
        default: {
            qWarning() << "Unable to handle PresenceType:" << pt;
        }
    }
}

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
            .codecType = "vp9",
            .avatarUrl = stdstring(vCard.photo.url),
            .email = vCard.emails.isEmpty() ? "" : stdstring(vCard.emails.last().number),
            .nick = stdstring(vCard.nickname),
            .resource = self.resource(),
    };
    manager->join(meet, participant);

    for (auto* h : handlers) {
        h->onParticipantJoined(ok::base::Jid(jid.full()), toParticipant(participant));
    }
}

void IMMeet::handleParticipant(const gloox::JID& jid, const gloox::Meet::Participant& participant) {
    qDebug() << __func__ << qstring(participant.email);
    for (auto* h : handlers) {
        h->onParticipantJoined(ok::base::Jid(jid.full()), toParticipant(participant));
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

bool lib::messenger::IMMeet::doSessionInitiate(gloox::Jingle::Session* session,
                                               const gloox::Jingle::Session::Jingle* jingle,
                                               const IMPeerId& peerId) {
    auto& from = session->remote();
    if (!from.server().starts_with("conference.")) {
        // 非会议
        return false;
    }

    auto sId = qstring(jingle->sid());
    qDebug() << __func__ << "sid:" << sId << "peer:" << peerId.toString();

    ortc::OJingleContentAv cav;
    ParseAV(jingle, cav);
    if (!cav.isValid()) {
        qDebug() << "Is no av session!";
        return false;
    }

    cav.sdpType = lib::ortc::JingleSdpType::Offer;
    auto rtc = ortc::OkRTCManager::getInstance()->getRtc();
    rtc->CreateAnswer(stdstring(peerId.toString()), cav);

    currentSid = sId;
    currentSession = session;
    return true;
}

bool IMMeet::doSessionAccept(gloox::Jingle::Session* session,
                             const gloox::Jingle::Session::Jingle* jingle,
                             const IMPeerId& peerId) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doSessionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doContentAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doContentRemove(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doContentModify(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doContentAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doContentReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doTransportInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doTransportAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doTransportReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doTransportReplace(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doSecurityInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doDescriptionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMMeet::doSourceAdd(const gloox::Jingle::Session::Jingle* jingle, const IMPeerId& peerId) {
    SESSION_CHECK(currentSid);
    for (const auto p : jingle->plugins()) {
        if (p->pluginType() == gloox::Jingle::PluginJsonMessage) {
            auto jm = static_cast<const gloox::Jingle::JsonMessage*>(p);
            if (jm) {
                qDebug() << "json-message:" << jm->json().c_str();
                std::map<std::string, ortc::OMeetSSRCBundle> map;
                ParseOMeetSSRCBundle(jm->json(), map);

                auto rtc = ortc::OkRTCManager::getInstance()->getRtc();
                if (rtc) {
                    rtc->addSource(stdstring(peerId.toString()), map);
                }
            }
        }
    }

    return true;
}

bool IMMeet::doInvalidAction(const gloox::Jingle::Session::Jingle* jingle, const IMPeerId&) {
    if (currentSid.toStdString() != jingle->sid()) {
        qWarning() << __func__ << "Unable to handle session:" << currentSid;
        return false;
    }

    qDebug() << __func__;

    return true;
}

bool IMMeet::doSessionTerminate(gloox::Jingle::Session* session,
                                const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    currentSid.clear();
    currentSession = nullptr;
    return true;
}

void IMMeet::handleJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm) {}

void IMMeet::clearSessionInfo(const QString& sId) {}

void IMMeet::onCreatePeerConnection(const std::string& sId, const std::string& peerId, bool ok) {}

void IMMeet::onRTP(const std::string& sId, const std::string& peerId,
                   const ortc::OJingleContentAv& osd) {}

void IMMeet::onFailure(const std::string& sId, const std::string& peerId,
                       const std::string& error) {}

void IMMeet::onIceGatheringChange(const std::string& sId, const std::string& peerId,
                                  ortc::IceGatheringState state) {
    QString qsId = qstring(sId);
    QString qPeerId = qstring(peerId);

    qDebug() << __func__ << "sId:" << qsId << "peerId:" << qPeerId;
    qDebug() << "state:" << static_cast<int>(state);

    emit iceGatheringStateChanged(IMPeerId(qPeerId), qsId, state);

    if (state == ortc::IceGatheringState::Complete) {
        doForIceCompleted(qsId, qPeerId);
    }
}

void IMMeet::onIce(const std::string& sId, const std::string& peerId, const ortc::OIceUdp& iceUdp) {

}

void IMMeet::doForIceCompleted(const QString& sId, const QString& peerId) {
    if (currentSession == nullptr) {
        qWarning() << "Unable to find jingle session:" << sId;
        return;
    }

    ortc::OJingleContentAv av;
    ortc::OkRTC* rtc = ortc::OkRTCManager::getInstance()->getRtc();
    rtc->getLocalSdp(stdstring(peerId), av);

    gloox::Jingle::PluginList plugins;
    ToPlugins(av, plugins);

    //    if (pSession->direction() == CallDirection::CallIn) {
    currentSession->sessionAccept(plugins);
    //    } else if (pSession->direction() == CallDirection::CallOut) {
    //        pSession->getSession()->sessionInitiate(plugins);
    //    }
}

void IMMeet::onIceConnectionChange(const std::string& sId, const std::string& peerId,
                                   ortc::IceConnectionState state) {
    QString qsId = qstring(sId);
    QString qPeerId = qstring(peerId);
    qDebug() << __func__ << "sId:" << qsId << "peerId:" << qPeerId;
    qDebug() << "state:" << static_cast<int>(state);
}

void IMMeet::onPeerConnectionChange(const std::string& sId, const std::string& peerId,
                                    ortc::PeerConnectionState state) {
    QString qsId = qstring(sId);
    QString qPeerId = qstring(peerId);
    qDebug() << __func__ << "sId:" << qsId << "peerId:" << qPeerId;
    qDebug() << "state:" << qstring(ortc::PeerConnectionStateAsStr(state));
}

void IMMeet::onSignalingChange(const std::string& sId, const std::string& peerId,
                               ortc::SignalingState state) {
    return;
}

void IMMeet::onRender(const std::string& friendId, ortc::RendererImage image) {
    qDebug() << __func__ << "render image {w:" << image.width_ << ", h:" << image.height_ << "}";
}

}  // namespace lib::messenger
