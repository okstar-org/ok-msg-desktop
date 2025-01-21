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
// Created by gaojie on 24-5-29.
//
#include <QDebug>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>

#include <extdisco.h>
#include <jinglegroup.h>
#include <jingleiceudp.h>
#include <jinglertp.h>

#include "IM.h"
#include "IMCall.h"
#include "base/basic_types.h"
#include "lib/ortc/ok_rtc.h"
#include "lib/ortc/ok_rtc_manager.h"

namespace lib::messenger {

inline void packCandidates(const ortc::CandidateList& src,
                           gloox::Jingle::ICEUDP::CandidateList& to) {
    for (auto& c : src) {
        to.push_front(gloox::Jingle::ICEUDP::Candidate{
                .component = c.component,
                .foundation = c.foundation,
                .generation = c.generation,
                .id = c.id,
                .ip = c.ip,
                .network = c.network,
                .port = c.port,
                .priority = c.priority,
                .protocol = c.protocol,
                .tcptype = c.tcptype,
                .rel_addr = c.rel_addr,
                .rel_port = c.rel_port,
                .type = static_cast<gloox::Jingle::ICEUDP::Type>(c.type)});
    }
}

inline void packDtls(const ortc::Dtls& src, gloox::Jingle::ICEUDP::Dtls& to) {
    to.hash = src.hash;
    to.setup = src.setup;
    to.fingerprint = src.fingerprint;
}

IMCallSession::IMCallSession(const std::string& sId_,
                             gloox::Jingle::Session* mSession,
                             const IMContactId& selfId,
                             const IMPeerId& peerId,
                             lib::ortc::JingleCallType callType)
        : sId(sId_), session(mSession), selfId(selfId), accepted(false), m_callType{callType} {
}

IMCallSession::~IMCallSession() {}

gloox::Jingle::Session* IMCallSession::getSession() const {
    return session;
}

void IMCallSession::onAccept() {
    // 对方接收
}

void IMCallSession::onTerminate() {
    lib::ortc::OkRTCManager::getInstance()->destroyRtc();
}

void IMCallSession::doTerminate() {
    // 发送结束协议
    session->sessionTerminate(
            new gloox::Jingle::Session::Reason(gloox::Jingle::Session::Reason::Reasons::Success));
}

const gloox::Jingle::Session::Jingle* IMCallSession::getJingle() const {
    return jingle;
}

void IMCallSession::setJingle(const gloox::Jingle::Session::Jingle* jingle_) {
    jingle = jingle_;
}

CallDirection IMCallSession::direction() const {
    auto sender = session->initiator().bareJID();
    auto self = gloox::JID((selfId.toString())).bareJID();
    return (sender == self) ? CallDirection::CallOut : CallDirection::CallIn;
}

void IMCallSession::setCallStage(CallStage state) {
    m_callStage = state;
}

void IMCallSession::setContext(const ortc::OJingleContent& jc) {
    context = jc;
}

void IMCallSession::start() {}

void IMCallSession::stop() {}

IMCall::IMCall(IM* im) : IMJingle(im) {
    im->addIMHandler(this);
}

IMCall::~IMCall() {
}

void IMCall::onImStartedCall() {
    auto client = im->getClient();
    assert(client);
    client->registerMessageHandler(this);

    auto disco = client->disco();
    im->addSessionHandler(this);
}

void IMCall::addCallHandler(CallHandler* hdr) {
    /**
     * callHandlers
     */
    callHandlers.push_back(hdr);
}

bool IMCall::callToFriend(const std::string& friendId, const std::string& sId, bool video) {
    proposeJingleMessage(friendId, sId, video);

    auto resources = im->getOnlineResources((friendId));
    if (resources.empty()) {
        return false;
    }

    proposeJingleMessage(friendId, sId, video);

    return true;
}

bool IMCall::callToPeerId(const IMPeerId& to, const std::string& sId, bool video) {
    //    qDebug() << std::string("peerId:%1 video:%2").arg((to.toString())).arg(video);
    //    auto r = createCall(to, sId, video);
    //    qDebug() << "createdCall=>" << r;
    return true;
}

bool IMCall::callAnswerToFriend(const IMPeerId& f, const std::string& callId, bool video) {
    return answer(f, callId, video);
}

void IMCall::callCancel(const IMContactId& f, const std::string& sId) {
    cancelCall(f, sId);
}

void IMCall::callReject(const IMPeerId& f, const std::string& sId) {
    rejectCall(f, sId);
}

// startCall
bool IMCall::startCall(const std::string& friendId, const std::string& sId, bool video) {
    auto resources = im->getOnlineResources((friendId));
    if (resources.empty()) {
        return false;
    }
    sendCallToResource(friendId, sId, video);
    return true;
}

bool IMCall::sendCallToResource(const std::string& friendId, const std::string& sId, bool video) {
    proposeJingleMessage(friendId, sId, video);
    return true;
}

bool IMCall::createCall(const IMPeerId& to, const std::string& sId, bool video) {
    auto rtcManager = lib::ortc::OkRTCManager::getInstance();
    rtcManager->setIceServers(im->getExternalServiceDiscovery());

    auto rtc = rtcManager->createRtc(ortc::Mode::p2p, im->self().resource());
    rtc->addRTCHandler(this);

    auto created = rtc->CreateOffer((to.toString()), (sId), video);
    if (created) {
        createSession(im->getSelfId(), to, sId, ortc::JingleCallType::av);
        currentSid = sId;

        for (auto h : callHandlers) {
            h->onCallCreated(to, sId);
        }
    }
    return created;
}
//
// void IMCall::cancel(const std::string& friendId) {
//    qDebug() << __func__ << friendId;
//
//    auto sId = m_friendSessionMap.value(IMPeerId(friendId));
//    auto session = m_sessionMap.value(sId);
//    if (session) {
//        cancelCall(IMContactId{friendId}, qstring(session->getSession()->sid()));
//        clearSessionInfo(sId);
//    }
//
//    currentSid.clear();
//}

void IMCall::cancelCall(const IMContactId& friendId, const std::string& sId) {
    IMCallSession* s = findSession(sId);
    if (s) {
        terminated = true;

        auto pRtcManager = ortc::OkRTCManager::getInstance();
        if (pRtcManager) {
            pRtcManager->destroyRtc();
        }

        s->doTerminate();
        s->setCallStage(CallStage::StageNone);
        clearSessionInfo(sId);
    } else {
        retractJingleMessage(friendId.toString(), sId);
    }
    currentSid.clear();
}

void IMCall::rejectCall(const IMPeerId& peerId, const std::string& sId) {
    IMCallSession* s = findSession(sId);
    if (s) {
        s->doTerminate();
        clearSessionInfo(sId);
    }
    rejectJingleMessage(peerId.toString(), sId);
}

bool IMCall::answer(const IMPeerId& peerId, const std::string& callId, bool video) {
    ortc::OkRTCManager* rtcManager = lib::ortc::OkRTCManager::getInstance();
    rtcManager->setIceServers(im->getExternalServiceDiscovery());
    auto rtc = rtcManager->createRtc(ortc::Mode::p2p, im->self().resource());
    rtc->addRTCHandler(this);
    acceptJingleMessage(peerId, callId, video);
    return true;
}

void IMCall::setCtrlState(ortc::CtrlState state) {
    auto rtc = lib::ortc::OkRTCManager::getInstance()->getRtc();
    if (!rtc) return;
    rtc->setEnable(state);
}

void IMCall::setSpeakerVolume(uint32_t vol) {
    auto rtc = lib::ortc::OkRTCManager::getInstance()->getRtc();
    if (!rtc) return;
    rtc->setSpeakerVolume(vol);
}

void IMCall::onCreatePeerConnection(const std::string& sId, const std::string& peerId, bool ok) {
    qDebug() << __func__;
}

void IMCall::onFailure(const std::string& sId,
                       const std::string& peerId,
                       const std::string& error) {
    // TODO 通知前台处理错误情况
}

void IMCall::onIceGatheringChange(const std::string& sId, const std::string& peerId,
                                  ortc::IceGatheringState state) {

    if (state == ortc::IceGatheringState::Complete) {
        doForIceCompleted(sId, peerId);
    }

    for (auto h : callHandlers) {
        h->onIceGatheringChange(IMPeerId(peerId), sId, state);
    }
}

/**
 * Ice交互完成，处理事项
 * @param sId
 * @param peerId
 * @param qsId
 */
void IMCall::doForIceCompleted(const std::string& sId, const std::string& peerId) {
    auto pSession = findSession(sId);
    if (!pSession) {
        return;
    }

    ortc::OkRTC* rtc = ortc::OkRTCManager::getInstance()->getRtc();
    auto av = rtc->getLocalSdp((peerId));

    gloox::Jingle::PluginList plugins;
    ToPlugins(av.get(), plugins);

    if (pSession->direction() == CallDirection::CallIn) {
        pSession->getSession()->sessionAccept(plugins);
    } else if (pSession->direction() == CallDirection::CallOut) {
        pSession->getSession()->sessionInitiate(plugins);
    }
}

void IMCall::destroyRtc() {
    if (terminated && !destroyedRtc) {
        ortc::OkRTC* rtc = ortc::OkRTCManager::getInstance()->getRtc();
        if (rtc) {
            lib::ortc::OkRTCManager::getInstance()->destroyRtc();
        }
        destroyedRtc = true;
    }
}

void IMCall::onIceConnectionChange(const std::string& sId,
                                   const std::string& peerId,
                                   ortc::IceConnectionState state) {
    /**
     *
    OnIceConnectionChange=>checking
    OnIceConnectionChange=>connected
    OnIceConnectionChange=>completed
    OnIceConnectionChange=>disconnected
    OnIceConnectionChange=>closed
     */
    qDebug() << __func__ << qstring(ortc::IceConnectionStateAsStr(state));

    for (auto h : callHandlers) {
        h->onIceConnectionChange(IMPeerId(peerId), (sId), state);
    }
}

void IMCall::onPeerConnectionChange(const std::string& sId, const std::string& peerId,
                                    ortc::PeerConnectionState state) {
    /**
     * OnConnectionChange : connecting
     * OnConnectionChange : connected
     * OnConnectionChange : closed
     */
    qDebug() << __func__ << qstring(ortc::PeerConnectionStateAsStr(state));

    for (const auto& item : callHandlers) {
        assert(item);
        item->onPeerConnectionChange(IMPeerId{peerId}, (sId), state);
    }
}

void IMCall::onSignalingChange(const std::string& sId, const std::string& peerId,
                               ortc::SignalingState state) {
    qDebug() << __func__ << "sId:" << qstring(ortc::SignalingStateAsStr(state));
    if (state == ortc::SignalingState::Closed) {
        for (auto h : callHandlers) {
            h->onEnd(IMPeerId((peerId)));
        }
    }
}

void IMCall::onLocalDescriptionSet(const std::string& sid,     //
                                   const std::string& peerId,  //
                                   const ortc::OJingleContentMap* oContext) {
    auto sId = (sid);
    qDebug() << __func__ << "sId:" << qstring(sId) << "peerId:" << qstring(peerId);
}

void IMCall::onIce(const std::string& sId,     //
                   const std::string& peerId,  //
                   const lib::ortc::OIceUdp& oIceUdp) {
    auto sid = (sId);
    if (sid.empty()) {
        qWarning() << "sid is empty.";
        return;
    }

    qDebug() << __func__ << "sId:" << qstring(sid) << "peerId:" << qstring(peerId)
             << "mid:" << qstring(oIceUdp.mid);

    auto* session = findSession(sid);
    if (!session) {
        qWarning() << "Unable to find session:" << &sId;
        return;
    }

    gloox::Jingle::ICEUDP::CandidateList cl;
    packCandidates(oIceUdp.candidates, cl);

    auto* iceUdp = new gloox::Jingle::ICEUDP(oIceUdp.pwd, oIceUdp.ufrag, cl);

    gloox::Jingle::ICEUDP::Dtls dtls;
    packDtls(oIceUdp.dtls, dtls);

    iceUdp->setDtls(dtls);

    gloox::Jingle::PluginList pluginList;
    pluginList.push_back(iceUdp);
    auto c = new gloox::Jingle::Content(oIceUdp.mid, pluginList);
    session->getSession()->transportInfo(c);
}

/**
 * 视频渲染
 * @param peerId
 * @param image
 */
void IMCall::onRender(const lib::ortc::RendererImage& image,
                      const std::string& peerId,
                      const std::string& resource) {
    if (peerId.empty()) {
        for (const auto& item : callHandlers) {
            item->onSelfVideoFrame(image.width_, image.height_, image.y, image.u, image.v,
                                   image.ystride, image.ustride, image.vstride);
        }
    } else {
        for (const auto& item : callHandlers) {
            item->onFriendVideoFrame(IMPeerId(peerId).toFriendId(), image.width_, image.height_,
                                     image.y, image.u, image.v, image.ystride, image.ustride,
                                     image.vstride);
        }
    }
}

bool IMCall::doSessionAccept(gloox::Jingle::Session* session,
                             const gloox::Jingle::Session::Jingle* jingle,
                             const lib::messenger::IMPeerId& peerId) {
    if (currentSid.empty()) {
        return false;
    }

    auto sId = (session->sid());
    ortc::OJingleContentMap av;
    av.sdpType = ortc::JingleSdpType::Answer;
    ParseAV(jingle, av);

    if (!av.isValid()) {
        qWarning() << "Is no call session";
        return false;
    }
    IMCallSession* pSession = nullptr;
    auto it = m_sessionMap.find(sId);
    if (it == m_sessionMap.end()) {
        // 创建session
        auto selfId = im->getSelfId();
        pSession = new IMCallSession(sId, session, selfId, peerId, ortc::JingleCallType::av);
        m_sessionMap.insert(std::make_pair(sId, pSession));
    } else {
        pSession = it->second;
    }

    // RTC 接受会话
    auto rtc = ortc::OkRTCManager::getInstance()->getRtc();
    const std::string& id = (peerId.toString());
    rtc->setRemoteDescription(id, av);

    //        std::this_thread::sleep_for(std::chrono::seconds(3));

    auto map = rtc->getCandidates(id);
    for (const auto& kv : map) {
        auto& oIceUdp = kv.second;

        gloox::Jingle::ICEUDP::CandidateList cl;
        packCandidates(oIceUdp.candidates, cl);

        auto* iceUdp = new gloox::Jingle::ICEUDP(oIceUdp.pwd, oIceUdp.ufrag, cl);

        gloox::Jingle::ICEUDP::Dtls dtls;
        packDtls(oIceUdp.dtls, dtls);

        iceUdp->setDtls(dtls);

        gloox::Jingle::PluginList pluginList;
        pluginList.push_back(iceUdp);
        auto c = new gloox::Jingle::Content(oIceUdp.mid, pluginList);
        pSession->getSession()->transportInfo(c);
    }

    return true;
}

void IMCall::doJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm) {
    qDebug() << __func__ << "peerId:" << qstring(peerId.toString()) << "sId:" << qstring(jm->id())
             << "action:" << gloox::Jingle::ActionValues[jm->action()];

    auto friendId = peerId.toFriendId();
    qDebug() << "friendId:" << qstring(friendId);

    auto sId = (jm->id());

    switch (jm->action()) {
        case gloox::Jingle::JingleMessage::reject: {
            /**
             * 对方拒绝
             */
            const auto& ms = jm->medias();
            for (auto handler : callHandlers) {
                handler->receiveCallStateRejected(peerId, sId, ms.size() > 1);
            }
            break;
        }
        case gloox::Jingle::JingleMessage::propose: {
            // 被对方发起呼叫
            qDebug() << "On call from:" << qstring(peerId.toString());

            // 获取呼叫类型
            bool audio = false;
            bool video = false;
            for (auto& m : jm->medias()) {
                if (m == gloox::Jingle::audio) {
                    audio = true;
                } else if (m == gloox::Jingle::video) {
                    video = true;
                }
            }
            for (auto handler : callHandlers) {
                handler->onCall(peerId, sId, audio, video);
            }

            break;
        }
        case gloox::Jingle::JingleMessage::retract: {
            /**
             * 撤回(需要判断是对方还是自己其它终端)
             */

            for (auto handler : callHandlers) {
                handler->onCallRetract(IMPeerId(friendId), CallState::NONE);
            }
            break;
        }
        case gloox::Jingle::JingleMessage::accept: {
            // 自己其它终端接受，挂断自己
            if (peerId != im->getSelfPeerId()) {
                for (auto handler : callHandlers) {
                    handler->onHangup(IMPeerId(friendId), CallState::NONE);
                }

            } else {
                // 自己终端接受，不处理
                //            OkRTCManager::getInstance()->getRtc()->CreateAnswer(peerId.toString());
            }
            break;
        }
        case gloox::Jingle::JingleMessage::proceed: {
            // 对方接受
            auto removed = m_sidVideo.erase(sId);
            createCall(peerId, sId, removed == 1);

            for (auto handler : callHandlers) {
                handler->receiveCallStateAccepted(peerId, sId, removed == 1);
            }
            break;
        }
        case gloox::Jingle::JingleMessage::finish:
            break;
    }
}

void IMCall::proposeJingleMessage(const std::string& friendId, const std::string& callId,
                                  bool video) {
    qDebug() << __func__ << "friend:" << qstring(friendId) << qstring(callId);

    gloox::StanzaExtensionList exts;
    auto jm = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::propose, (callId));
    jm->addMedia(gloox::Jingle::Media::audio);
    if (video) {
        jm->addMedia(gloox::Jingle::Media::video);
        m_sidVideo.insert(std::make_pair(callId, true));
    }
    exts.push_back(jm);

    auto jid = gloox::JID{(friendId)};
    gloox::Message m(gloox::Message::Chat, jid, {}, {});
    for (auto ext : exts) m.addExtension(ext);

    im->getClient()->send(m);
}

void IMCall::rejectJingleMessage(const std::string& peerId, const std::string& callId) {
    qDebug() << __func__ << "friend:" << qstring(peerId) << "callId:" << callId.c_str();

    gloox::StanzaExtensionList exts;
    auto reject = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::reject, (callId));
    exts.push_back(reject);

    auto jid = gloox::JID{(peerId)};
    gloox::Message m(gloox::Message::Chat, jid, {}, {});
    for (auto ext : exts) m.addExtension(ext);

    im->getClient()->send(m);
}

void IMCall::retractJingleMessage(const std::string& friendId, const std::string& callId) {
    qDebug() << __func__ << "friend:" << friendId.c_str() << "callId:" << callId.c_str();

    auto* jm = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::retract, (callId));

    auto jid = gloox::JID{(friendId)};
    gloox::Message m(gloox::Message::Chat, jid, {}, {});
    m.addExtension(jm);

    im->getClient()->send(m);
}

void IMCall::acceptJingleMessage(const IMPeerId& peerId, const std::string& callId, bool video) {
    qDebug() << __func__ << "friend:" << peerId.toString().c_str() << "callId:" << callId.c_str();

    auto proceed = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::proceed, (callId));
    gloox::Message proceedMsg(gloox::Message::Chat, gloox::JID((peerId.toString())));
    proceedMsg.addExtension(proceed);
    im->getClient()->send(proceedMsg);
    qDebug() << "Sent proceed=>" << peerId.toString().c_str();

    // 发送给自己其它终端
    auto accept = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::accept, (callId));

    auto self = im->self().bareJID();

    gloox::Message msg(gloox::Message::Chat, self);
    msg.addExtension(accept);
    im->getClient()->send(msg);
    qDebug() << "Sent accept=>" << qstring(self.full());

    // 设置状态为接受
    auto ws = findSession(callId);
    if (!ws) {
        ws = createSession(im->getSelfId(), peerId, callId, ortc::JingleCallType::av);
    }
    ws->setAccepted(true);
}

IMCallSession* IMCall::cacheSessionInfo(const IMContactId& from,
                                        const IMPeerId& to,
                                        const std::string& sId,
                                        lib::ortc::JingleCallType callType) {
    qDebug() << __func__ << "to:" << qstring(to.toString());
    auto session = getIM()->createSession(gloox::JID((to.toString())), (sId), this);

    m_friendSessionMap.insert(std::make_pair(to, sId));

    auto ws = new IMCallSession(sId, session, from, to, ortc::JingleCallType::av);
    m_sessionMap.insert(std::make_pair(sId, ws));

    return ws;
}

void IMCall::clearSessionInfo(const std::string& sId) {
    qDebug() << __func__ << qstring(sId);
    auto it = m_sessionMap.find(sId);
    if (it == m_sessionMap.end()) {
        return;
    }

    auto session = it->second;
    auto s = session->getSession();
    auto& responder = s->remote();
    m_friendSessionMap.erase(IMPeerId(responder));
    m_sessionMap.erase(sId);
}

IMCallSession* IMCall::createSession(const IMContactId& self,
                                     const IMPeerId& peer,
                                     const std::string& sId,
                                     ortc::JingleCallType ct) {
    return cacheSessionInfo(self, peer, sId, ct);
}

void IMCall::handleJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm) {
    qDebug() << __func__;
    doJingleMessage(peerId, jm);
}

bool IMCall::doSessionInfo(const gloox::Jingle::Session::Jingle* jingle, const IMPeerId& friendId) {
    qDebug() << "jingle:%1 peerId:%2"   //
             << qstring(jingle->sid())  //
             << friendId.toString().c_str();
    return true;
}

bool IMCall::doContentAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doContentRemove(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doContentModify(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doContentAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doContentReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doTransportInfo(const gloox::Jingle::Session::Jingle* jingle, const IMPeerId& peerId) {
    auto sid = (jingle->sid());
    qDebug() << __func__ << "sId:" << qstring(sid) << "peerId:" << qstring(peerId.toString());

    if (currentSid.empty()) {
        return false;
    }

    auto s = findSession(sid);
    if (!s) {
        qWarning() << ("Session is no existing.");
        return false;
    }

    ortc::OJingleContentMap av;
    ParseAV(jingle, av);

    for (auto& kv : av.getContents()) {
        auto& oSdp = kv.second;
        ortc::OkRTCManager::getInstance()
                ->getRtc()  //
                ->setTransportInfo((peerId.toString()), jingle->sid(), oSdp.iceUdp);
    }

    return true;
}

bool IMCall::doTransportAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doTransportReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doTransportReplace(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doSecurityInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doDescriptionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

bool IMCall::doInvalidAction(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    return true;
}

void IMCall::onConnecting()
{

}

void IMCall::onConnected()
{

}

void IMCall::onDisconnected(int)
{

}

void IMCall::onStarted()
{
    qDebug() << __func__;
    onImStartedCall();
}

void IMCall::onStopped()
{
    qDebug() << __func__;
}

bool IMCall::doSessionInitiate(gloox::Jingle::Session* session,
                               const gloox::Jingle::Session::Jingle* jingle,
                               const IMPeerId& peerId) {
    auto& from = session->remote();
    if (from.server().starts_with("conference.")) {
        return false;
    }

    auto sId = (jingle->sid());
    qDebug() << __func__ << "sid:" << sId.c_str();

    ortc::OJingleContentMap cav;
    ParseAV(jingle, cav);
    if (!cav.isValid()) {
        qWarning() << "Is invalid jingle content!";
        return false;
    }

    for (const auto& item : cav.getContents()) {
        if (!item.second.isAV()) {
            qWarning() << "Is no av content!";
            return false;
        }
    }

    cav.sdpType = lib::ortc::JingleSdpType::Offer;
    auto rtc = ortc::OkRTCManager::getInstance()->createRtc(ortc::Mode::p2p, im->self().resource());
    rtc->CreateAnswer((peerId.toString()), cav);
    currentSid = sId;
    return true;
}

bool IMCall::doSessionTerminate(gloox::Jingle::Session* session,
                                const gloox::Jingle::Session::Jingle*,
                                const lib::messenger::IMPeerId& peerId) {
    if (currentSid.empty()) {
        return false;
    }

    auto sId = (session->sid());
    qDebug() << __func__ << "sId:" << sId.c_str();
    auto s = findSession(sId);
    if (s) {
        s->doTerminate();
    }

    for (auto h : callHandlers) {
        h->onHangup(peerId, CallState::FINISHED);
    }

    auto rtcManager = ortc::OkRTCManager::getInstance();
    auto rtc = rtcManager->getRtc();
    if (rtc) rtc->removeRTCHandler(this);

    clearSessionInfo(sId);
    currentSid.clear();

    return true;
}

bool lib::messenger::IMCall::doSourceAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

}  // namespace lib::messenger
