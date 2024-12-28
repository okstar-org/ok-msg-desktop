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
#include <range/v3/all.hpp>
#include <utility>

#include "IM.h"
#include "application.h"
#include "src/base/uuid.h"

namespace lib::messenger {

Participant IMMeet::toParticipant(const gloox::Meet::Participant& participant) const {
    Participant p{.email = qstring(participant.email),
                  .nick = qstring(participant.nick),
                  .resource = qstring(participant.resource),
                  .avatarUrl = participant.avatarUrl,
                  .jid = ok::base::Jid(participant.jid.full()),
                  .affiliation = qstring(participant.affiliation),
                  .role = qstring(participant.role)};

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

IMMeet::IMMeet(IM* im, QObject* parent) : IMJingle(im, parent), manager(nullptr), meet(nullptr) {
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

    // OkMSG.root-host.[meet-241219-51-g57a9b7d0].OTE5Y2 --> OTE5Y2
    auto split = qstring(im->self().resource()).split(".");
    resource = split[split.size() - 1];
    qDebug() << "Resource is:" << resource;

    // qRegisterMetaType
    qRegisterMetaType<ortc::OJingleContentAv>("const ortc::OJingleContentAv&");

    ortc::OkRTCManager* rtcManager = ortc::OkRTCManager::getInstance();
    //    rtcManager->setIceServerers(im->getExternalServiceDiscovery());

    auto rtc = rtcManager->createRtc(ortc::Mode::meet, stdstring(resource));
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
    if (rtc) {
        rtc->removeRTCHandler(this);
        pRtcManager->destroyRtc();
    }
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

void IMMeet::disband() {
    leave();

    // TODO 执行销毁会议
}

void IMMeet::leave() {
    manager->exitMeet();
}

void IMMeet::join() {}

void IMMeet::sendMessage(const QString& msg) {
    if (msg.isEmpty()) {
        qWarning() << "Empty message!";
        return;
    }
    meet->send(stdstring(msg));
}

void IMMeet::handleHostPresence(const gloox::JID& from, const gloox::Presence& presence) {
    qDebug() << __func__ << qstring(from.full()) << "presence:" << presence.presence();

    auto pt = presence.subtype();
    auto t = presence.getOriginTag();
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
                h->onParticipantLeft(ok::base::Jid(from.full()), qstring(from.resource()));
            }
            break;
        }
        default: {
            qWarning() << "Unable to handle PresenceType:" << pt;
        }
    }
}

void IMMeet::handleHostMessage(const gloox::JID& from, const gloox::Message& msg) {
    auto message = qstring(msg.body());
    auto participant = qstring(from.resource());

    qDebug() << __func__ << qstring(from.full()) << "msg:" << message;
    if (participant.isEmpty()) {
        return;
    }

    for (const auto& item : handlers) {
        item->onParticipantMessage(participant, message);
    }
}

void IMMeet::handleHostMessageSession(const gloox::JID& from, const std::string& sid) {}

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
    gloox::Meet::Participant participant = {
            .region = "region1",
            .codecType = "vp9",
            .avatarUrl = stdstring(vCard.photo.url),
            .email = vCard.emails.isEmpty() ? "" : stdstring(vCard.emails.last().number),
            .nick = stdstring(vCard.nickname),
            .resource = stdstring(resource),
    };
    manager->join(*meet, participant);

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

    auto sId = qstring(jingle->sid());
    qDebug() << __func__ << "sid:" << sId << "peer:" << peerId.toString();

    ortc::OJingleContentAv cav;
    ParseAV(jingle, cav);
    if (!cav.isValid()) {
        qDebug() << "Is no av session!";
        return false;
    }
    cav.sdpType = ortc::JingleSdpType::Offer;

    QMetaObject::invokeMethod(this, "doStartRTC", Qt::QueuedConnection,
                              Q_ARG(const IMPeerId&, peerId),
                              Q_ARG(const ortc::OJingleContentAv&, cav));

    currentSid = sId;
    currentSession = session;
    return true;
}

void IMMeet::doStartRTC(const IMPeerId& peerId, const ortc::OJingleContentAv& cav) const {
    qDebug() << __func__;
    auto rtcManager = ortc::OkRTCManager::getInstance();
    auto rtc = rtcManager->getRtc();
    rtc->CreateAnswer(stdstring(peerId.toString()), cav);

    // auto& map = cav.getSsrcBundle();
    // rtc->addSource(stdstring(peerId.toString()), map);
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
    qDebug() << __func__;
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

void IMMeet::handleJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm) {
    qDebug() << __func__;
}

void IMMeet::clearSessionInfo(const QString& sId) {
    qDebug() << __func__ << sId;
}

void IMMeet::ToMeetSdp(const ortc::OJingleContentAv* av, gloox::Jingle::PluginList& plugins) {
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
    qDebug() << __func__;
}

void IMMeet::onLocalDescriptionSet(const std::string& sId, const std::string& peerId,
                                   const ortc::OJingleContentAv* osd) {
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
    QString qsId = qstring(sId);
    QString qPeerId = qstring(peerId);

    qDebug() << __func__ << "sId:" << qsId << "peerId:" << qPeerId
             << "state:" << qstring(IceGatheringStateAsStr(state));

    emit iceGatheringStateChanged(IMPeerId(qPeerId), qsId, state);

    if (state == ortc::IceGatheringState::Complete) {
        QThread::sleep(3);
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

    auto rtc = ortc::OkRTCManager::getInstance()->getRtc();
    auto av = rtc->getLocalSdp(stdstring(peerId));

    gloox::Jingle::PluginList plugins;
    ToMeetSdp(av.get(), plugins);
    currentSession->sessionAccept(plugins);
}

void IMMeet::onIceConnectionChange(const std::string& sId, const std::string& peerId,
                                   ortc::IceConnectionState state) {
    QString qsId = qstring(sId);
    QString qPeerId = qstring(peerId);
    qDebug() << __func__ << "sId:" << qsId << "peerId:" << qPeerId
             << "state:" << qstring(ortc::IceConnectionStateAsStr(state));
}

void IMMeet::onPeerConnectionChange(const std::string& sId, const std::string& peerId,
                                    ortc::PeerConnectionState state) {
    QString qsId = qstring(sId);
    QString qPeerId = qstring(peerId);
    qDebug() << __func__ << "sId:" << qsId << "peerId:" << qPeerId
             << "state:" << qstring(ortc::PeerConnectionStateAsStr(state));
}

void IMMeet::onSignalingChange(const std::string& sId, const std::string& peerId,
                               ortc::SignalingState state) {
    QString qsId = qstring(sId);
    QString qPeerId = qstring(peerId);
    qDebug() << __func__ << "sId:" << qsId << "peerId:" << qPeerId
             << "state:" << qstring(ortc::SignalingStateAsStr(state));
}

void IMMeet::onRender(const ortc::RendererImage& image,
                      const std::string& friendId,
                      const std::string& resource_) {
    //    qDebug() << __func__ << "render friendId:" << qstring(friendId) //
    //             << " image {w:" << image.width_ << ", h:" << image.height_ << "}";
    for (auto h : handlers) {
        if (friendId.empty()) {
            h->onParticipantVideoFrame(resource, image);
        } else {
            // resource format is like: f1b4629b-video-0-13
            auto s = qstring(resource_).split("-");
            if (!s.empty()) {
                h->onParticipantVideoFrame(s[0], image);
            }
        }
    }
}

void IMMeet::switchVideoDevice(const QString& deviceId) {
    auto pManager = ortc::OkRTCManager::getInstance();
    auto rtc = pManager->getRtc();
    if (!rtc) {
        return;
    }
    rtc->switchVideoDevice(stdstring(deviceId));
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

void IMMeet::setEnable(bool audio, bool video) {
    auto pManager = ortc::OkRTCManager::getInstance();
    auto rtc = pManager->getRtc();
    if (!rtc) {
        return;
    }
    rtc->setEnable(audio, video);

    auto self = meet->getSelf();
    //{"f4921c8d-a0":{"muted":true}, "f4921c8d-a0":{"muted":true}}
    self.sourceInfo = "{\"" + self.resource + "-a0\": {\"muted\":" + (audio ? "false" : "true") +
                      "}, " + self.resource + "-v0\": {\"muted\":" + (audio ? "false" : "true") +
                      " }}";
    meet->sendPresence(self);
}

}  // namespace lib::messenger
