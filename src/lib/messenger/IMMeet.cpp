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
#include <jinglegroup.h>
#include <jinglejsonmessage.h>
#include <meetmanager.h>
#include <QByteArray>
#include <iostream>
#include <range/v3/all.hpp>
#include <utility>

#include "IM.h"
#include "base/jsons.h"
#include "src/base/uuid.h"

namespace lib::messenger {

Participant IMMeet::toParticipant(const gloox::Meet::Participant& participant) const {
    Participant p{.email = (participant.email),
                  .nick = (participant.nick),
                  .resource = (participant.resource),
                  .avatarUrl = participant.avatarUrl,
                  .jid = (participant.jid.full()),
                  .affiliation = (participant.affiliation),
                  .role = (participant.role)};

    auto json = ok::base::Jsons::toJSON(QByteArray::fromStdString(participant.sourceInfo)).object();
    for (const auto& k : json.keys()) {
        const QJsonObject& object = json.value(k).toObject();
        if (k.endsWith("-a0")) {
            p.sourceInfo.audioMute = object.value("muted") == "true";
        } else if (k.endsWith("-v0")) {
            p.sourceInfo.videoMute = object.value("muted") == "true";
        }
    }
    return p;
}

std::string_view extractLastPartView(std::string_view input) {
    size_t pos = input.rfind('.');
    if (pos != std::string_view::npos) {
        return input.substr(pos + 1);
    }
    return {};
}

IMMeet::IMMeet(IM* im) : IMJingle(im), meetManager(nullptr), meet(nullptr) {
    meetManager = new gloox::MeetManager(im->getClient());
    meetManager->registerHandler(this);

    im->addSelfHandler(this);

    auto host = ("conference." + im->host());
    im->addFromHostHandler(host, this);
    im->addSessionHandler(this);
    // jingle json-message
    im->sessionManager()->registerPlugin(new gloox::Jingle::JsonMessage());

    // request self vcard.
    im->requestVCards();

    // OkMSG.root-host.[meet-241219-51-g57a9b7d0].OTE5Y2 --> OTE5Y2
    auto split = extractLastPartView(im->self().resource());
    resource = split[split.size() - 1];

    // qRegisterMetaType
    qRegisterMetaType<ortc::OJingleContentMap>("const ortc::OJingleContentMap&");

    ortc::OkRTCManager* rtcManager = ortc::OkRTCManager::getInstance();
    //    rtcManager->setIceServerers(im->getExternalServiceDiscovery());

    auto rtc = rtcManager->createRtc(ortc::Mode::meet, resource);
    rtc->addRTCHandler(this);
    rtc->start();
}

IMMeet::~IMMeet() {
    qDebug() << __func__;

    im->removeSelfHandler(this);
    im->removeSessionHandler(this);
    im->clearFromHostHandler();

    delete meetManager;
    meetManager = nullptr;

    auto pRtcManager = ortc::OkRTCManager::getInstance();
    auto rtc = pRtcManager->getRtc();
    if (rtc) {
        rtc->removeRTCHandler(this);
    }
}

const std::string& IMMeet::create(const std::string& name) {
    std::cout << __func__ << name;

    std::map<std::string, std::string> props;
    props.insert(std::pair("startAudioMuted", "9"));
    props.insert(std::pair("startVideoMuted", "9"));
    props.insert(std::pair("rtcstatsEnabled", "false"));

    gloox::JID room(name + "@conference." + im->host());
    meet = meetManager->createMeet(room, resource, props);

    return meet->getUid();
}

void IMMeet::disband() {
    std::cout << __func__;
    leave();
}

void IMMeet::leave() {
    meetManager->exitMeet();

    auto pManager = ortc::OkRTCManager::getInstance();
    auto rtc = pManager->getRtc();
    if (rtc) {
        pManager->destroyRtc();
    }
}

void IMMeet::join() {}

void IMMeet::sendMessage(const std::string& msg) {
    if (msg.empty()) {
        std::cerr << "Empty message!";
        return;
    }
    meet->send((msg));
}

void IMMeet::handleHostPresence(const gloox::JID& from, const gloox::Presence& presence) {
    std::cout << __func__ << (from.full()) << "presence:" << presence.presence();

    auto pt = presence.subtype();

    switch (pt) {
        case gloox::Presence::PresenceType::Available: {
            // 成员上线
            auto participant = meet->parseParticipant(from, presence);
            if (participant.email.empty()) {
                return;
            }

            meet->addParticipant(participant);

            for (auto* h : handlers) {
                h->onParticipantJoined(ok::base::Jid(from.full()), toParticipant(participant));
            }

            break;
        }
        case gloox::Presence::PresenceType::Unavailable: {
            // 成员离开 "<presence type='unavailable'
            // from='ykmfkvsa3t0f@meet.chuanshaninfo.com/19a59e74-0a10-4615-9f32-529969fcd59b'
            // to='sjdvr4swzf2f@meet.chuanshaninfo.com'/>"

            for (auto* h : handlers) {
                h->onParticipantLeft(ok::base::Jid(from.full()), (from.resource()));
            }
            break;
        }
        default: {
            std::cerr << "Unable to handle PresenceType:" << pt;
        }
    }
}

void IMMeet::handleHostMessage(const gloox::JID& from, const gloox::Message& msg) {
    auto message = (msg.body());
    auto participant = (from.resource());

    std::cout << __func__ << (from.full()) << "msg:" << message;
    if (participant.empty()) {
        return;
    }

    for (const auto& item : handlers) {
        item->onParticipantMessage(participant, message);
    }
}

void IMMeet::handleHostMessageSession(const gloox::JID& from, const std::string& sid) {}

void IMMeet::handleCreation(const gloox::JID& jid, bool ready,
                            const std::map<std::string, std::string>& props) {
    std::cout << __func__ << (jid.full()) << "ready:" << ready;
    for (const auto& kv : props) {
        std::cout << "property:" << (kv.first) << "=>" << (kv.second);
    }

    for (auto* h : handlers) {
        h->onMeetCreated(ok::base::Jid(jid.full()), ready, props);
    }

    // 加入到会议
    gloox::Meet::Participant participant = {
            .region = "region1",
            .codecType = "vp9",
            .avatarUrl = (vCard.photo.url),
            .email = vCard.emails.empty() ? "" : (vCard.emails.front().number),
            .nick = (vCard.nickname),
            .resource = (resource),
    };
    meetManager->join(*meet, participant);

    for (auto* h : handlers) {
        h->onParticipantJoined(ok::base::Jid(jid.full()), toParticipant(participant));
    }
}

void IMMeet::handleParticipant(const gloox::JID& jid, const gloox::Meet::Participant& participant) {
    std::cout << __func__ << (participant.email);
    for (auto* h : handlers) {
        h->onParticipantJoined(ok::base::Jid(jid.full()), toParticipant(participant));
    }
}
void IMMeet::handleStatsId(const gloox::JID& jid, const std::string& statsId) {}

void IMMeet::handleJsonMessage(const gloox::JID& jid, const gloox::JsonMessage* json) {
    std::cout << __func__ << (json->getJson());
}

void IMMeet::onConnecting() {}

void IMMeet::onConnected() {}

void IMMeet::onDisconnected(int) {}

void IMMeet::onStarted() {}

void IMMeet::onStopped() {}

void IMMeet::onSelfIdChanged(const std::string& id) {}

void IMMeet::onSelfNameChanged(const std::string& name) {}

void IMMeet::onSelfAvatarChanged(const std::string& avatar) {}

void IMMeet::onSelfStatusChanged(IMStatus status, const std::string& msg) {}

void IMMeet::onSelfVCardChanged(IMVCard& imvCard) {
    emit onSelfVCard(imvCard);
}

void IMMeet::addMeetHandler(MessengerMeetHandler* hdr) {
    if (!hdr) return;
    handlers.push_back(hdr);
}

void IMMeet::removeMeetHandler(MessengerMeetHandler* hdr) {
    if (!hdr) return;
    auto it = std::remove(handlers.begin(), handlers.end(), hdr);
    if (it != handlers.end()) {
        handlers.erase(it, handlers.end());
    }
}

void IMMeet::onSelfVCard(const IMVCard& vCard_) {
    vCard = vCard_;
}

bool IMMeet::doSessionInitiate(gloox::Jingle::Session* session,
                               const gloox::Jingle::Session::Jingle* jingle,
                               const IMPeerId& peerId) {
    auto& from = session->remote();
    if (!from.server().starts_with("conference.")) {
        // 非会议
        return false;
    }

    auto sId = (jingle->sid());
    std::cout << __func__ << "sid:" << sId << "peer:" << peerId.toString();

    ortc::OJingleContentMap cav;
    ParseAV(jingle, cav);
    if (!cav.isValid()) {
        std::cout << "Is no av session!";
        return false;
    }
    cav.sdpType = ortc::JingleSdpType::Offer;

    currentSid = sId;
    currentSession = session;

    for (auto h : handlers) {
        h->onMeetInitiate(peerId, cav);
    }
    return true;
}

void IMMeet::doStartRTC(const IMPeerId& peerId, const ortc::OJingleContentMap& map) const {
    std::cout << __func__;
    auto rtcManager = ortc::OkRTCManager::getInstance();
    auto rtc = rtcManager->getRtc();
    rtc->CreateAnswer((peerId.toString()), map);

    // auto& map = cav.getSsrcBundle();
    // rtc->addSource((peerId.toString()), map);
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
    std::cout << __func__;
    for (const auto p : jingle->plugins()) {
        if (p->pluginType() == gloox::Jingle::PluginJsonMessage) {
            auto jm = static_cast<const gloox::Jingle::JsonMessage*>(p);
            if (jm) {
                std::cout << "json-message:" << jm->json().c_str();
                std::map<std::string, ortc::OMeetSSRCBundle> map;
                ParseOMeetSSRCBundle(jm->json(), map);

                auto rtc = ortc::OkRTCManager::getInstance()->getRtc();
                if (rtc) {
                    rtc->addSource((peerId.toString()), map);
                }
            }
        }
    }

    return true;
}

bool IMMeet::doInvalidAction(const gloox::Jingle::Session::Jingle* jingle, const IMPeerId&) {
    if (currentSid != jingle->sid()) {
        std::cerr << __func__ << "Unable to handle session:" << currentSid;
        return false;
    }

    std::cout << __func__;

    return true;
}

bool IMMeet::doSessionTerminate(gloox::Jingle::Session* session,
                                const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    currentSid.clear();
    currentSession = nullptr;
    return true;
}

void IMMeet::handleJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm) {
    std::cout << __func__;
}

void IMMeet::clearSessionInfo(const std::string& sId) {
    std::cout << __func__ << sId;
}

void IMMeet::ToMeetSdp(const ortc::OJingleContentMap* av, gloox::Jingle::PluginList& plugins) {
    gloox::Jingle::Group::ContentList contentList;

    // audio
    for (auto& it : av->getContents()) {
        auto& sdp = it.second;
        if (sdp.rtp.media == ortc::Media::audio) {
            std::string mid = "audio";
            contentList.push_back(gloox::Jingle::Group::Content({.name = mid}));

            auto pContent = ToContent(mid, sdp, gloox::Jingle::Content::CResponder, false);
            plugins.emplace_back(pContent.release());
            break;
        }
    }

    // video
    for (auto& it : av->getContents()) {
        auto& sdp = it.second;
        if (sdp.rtp.media == ortc::Media::video) {
            std::string mid = "video";
            contentList.push_back(gloox::Jingle::Group::Content({.name = mid}));
            auto pContent = ToContent(mid, sdp, gloox::Jingle::Content::CResponder, false);
            plugins.emplace_back(pContent.release());
            break;
        }
    }

    // data
    for (auto& it : av->getContents()) {
        auto& sdp = it.second;
        if (sdp.rtp.media == ortc::Media::application) {
            std::string mid = "data";
            contentList.push_back(gloox::Jingle::Group::Content{.name = mid});
            auto pContent = ToContent(mid, sdp, gloox::Jingle::Content::CResponder, false);
            plugins.emplace_back(pContent.release());
            break;
        }
    }

    auto group = new gloox::Jingle::Group("BUNDLE", contentList);
    plugins.push_back(group);
}

void IMMeet::onCreatePeerConnection(const std::string& sId, const std::string& peerId, bool ok) {
    std::cout << __func__;
}

void IMMeet::onLocalDescriptionSet(const std::string& sId, const std::string& peerId,
                                   const ortc::OJingleContentMap* osd) {
    auto& map = osd->getSsrcBundle();
    if (map.empty()) {
        return;
    }

    std::string str;
    FormatOMeetSSRCBundle(map, str);

    currentSession->sourceAdd(new gloox::Jingle::JsonMessage(str));
}

void IMMeet::onFailure(const std::string& sId, const std::string& peerId,
                       const std::string& error) {}

void IMMeet::onIceGatheringChange(const std::string& sId, const std::string& peerId,
                                  ortc::IceGatheringState state) {
    std::cout << __func__ << "sId:" << sId << "peerId:" << peerId
              << "state:" << (IceGatheringStateAsStr(state));

    if (state == ortc::IceGatheringState::Complete) {
        doForIceCompleted(sId, peerId);
    }
}

void IMMeet::onIce(const std::string& sId, const std::string& peerId, const ortc::OIceUdp& iceUdp) {

}

void IMMeet::doForIceCompleted(const std::string& sId, const std::string& peerId) {
    if (currentSession == nullptr) {
        std::cerr << "Unable to find jingle session:" << sId;
        return;
    }

    auto rtc = ortc::OkRTCManager::getInstance()->getRtc();
    auto av = rtc->getLocalSdp((peerId));

    gloox::Jingle::PluginList plugins;
    ToMeetSdp(av.get(), plugins);
    currentSession->sessionAccept(plugins);
}

void IMMeet::onIceConnectionChange(const std::string& sId, const std::string& peerId,
                                   ortc::IceConnectionState state) {
    std::cout << __func__ << "sId:" << sId << "peerId:" << peerId
              << "state:" << (ortc::IceConnectionStateAsStr(state));
}

void IMMeet::onPeerConnectionChange(const std::string& sId, const std::string& peerId,
                                    ortc::PeerConnectionState state) {
    std::cout << __func__ << "sId:" << sId << "peerId:" << peerId
              << "state:" << (ortc::PeerConnectionStateAsStr(state));
}

void IMMeet::onSignalingChange(const std::string& sId, const std::string& peerId,
                               ortc::SignalingState state) {
    std::cout << __func__ << "sId:" << sId << "peerId:" << peerId
              << "state:" << (ortc::SignalingStateAsStr(state));

    if (state == ortc::SignalingState::Closed) {
        for (auto h : handlers) {
            h->onEnd();
        }
    }
}
std::string extractFirstPart(const std::string& input) {
    size_t pos = input.find('-');
    if (pos != std::string::npos) {
        return input.substr(0, pos);
    }
    return "";
}
void IMMeet::onRender(const ortc::RendererImage& image,
                      const std::string& friendId,
                      const std::string& resource_) {
    //    std::cout << __func__ << "render friendId:" << (friendId) //
    //             << " image {w:" << image.width_ << ", h:" << image.height_ << "}";
    for (auto h : handlers) {
        if (friendId.empty()) {
            h->onParticipantVideoFrame(resource, image);
        } else {
            // resource format is like: f1b4629b-video-0-13
            auto s = extractFirstPart(resource_);
            if (!s.empty()) {
                h->onParticipantVideoFrame(s, image);
            }
        }
    }
}

void IMMeet::switchVideoDevice(const std::string& deviceId) {
    auto pManager = ortc::OkRTCManager::getInstance();
    auto rtc = pManager->getRtc();
    if (!rtc) {
        return;
    }
    rtc->switchVideoDevice((deviceId));
}

void IMMeet::switchVideoDevice(int selected) {
    auto pManager = ortc::OkRTCManager::getInstance();
    auto rtc = pManager->getRtc();
    if (!rtc) {
        return;
    }
    rtc->switchVideoDevice(selected);
}

std::vector<std::string> IMMeet::getVideoDeviceList() {
    auto pManager = ortc::OkRTCManager::getInstance();
    auto rtc = pManager->getRtc();
    if (rtc) return rtc->getVideoDeviceList();
    return {};
}

void IMMeet::setEnable(ortc::CtrlState state) {
    std::cout << __func__;
    auto pManager = ortc::OkRTCManager::getInstance();
    auto rtc = pManager->getRtc();
    if (!rtc) {
        return;
    }
    rtc->setEnable(state);

    auto self = meet->getSelf();
    auto res = self.resource;
    if (res.empty()) {
        return;
    }

    //{"OTE5Y2-a0": {"muted":true}, "OTE5Y2-v0": {"muted":true }}
    self.sourceInfo = "{\"" + res + "-a0\": {\"muted\":" + (state.enableMic ? "false" : "true") +
                      "},\"" + res + "-v0\": {\"muted\":" + (state.enableMic ? "false" : "true") +
                      " }}";
    meet->sendPresence(self);
}

}  // namespace lib::messenger
