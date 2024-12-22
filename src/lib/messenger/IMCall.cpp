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

#include <range/v3/range.hpp>
#include <range/v3/view.hpp>
#include <thread>

#include <extdisco.h>
#include <jinglegroup.h>
#include <jingleiceudp.h>
#include <jinglertp.h>

#include "IM.h"
#include "IMCall.h"
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

IMCallSession::IMCallSession(const QString& sId_,
                             gloox::Jingle::Session* mSession,
                             const IMContactId& selfId,
                             const IMPeerId& peerId,
                             lib::ortc::JingleCallType callType)
        : sId(sId_), session(mSession), selfId(selfId), accepted(false), m_callType{callType} {
    qDebug() << __func__ << "type:" << (int)m_callType << "sid:" << sId
             << "to peer:" << peerId.toString();
    qDebug() << __func__ << "be created.";
}

IMCallSession::~IMCallSession() {
    qDebug() << __func__ << sId;
}

gloox::Jingle::Session* IMCallSession::getSession() const {
    return session;
}

void IMCallSession::onAccept() {
    // 对方接收
    qDebug() << __func__;
}

void IMCallSession::onTerminate() {
    qDebug() << __func__;
    lib::ortc::OkRTCManager::getInstance()->destroyRtc();
}

void IMCallSession::doTerminate() {
    qDebug() << __func__;
    // 发送结束协议
    session->sessionTerminate(
            new gloox::Jingle::Session::Reason(gloox::Jingle::Session::Reason::Reasons::Success));
    // 销毁rtc
    lib::ortc::OkRTCManager::getInstance()->destroyRtc();
}

const gloox::Jingle::Session::Jingle* IMCallSession::getJingle() const {
    return jingle;
}

void IMCallSession::setJingle(const gloox::Jingle::Session::Jingle* jingle_) {
    jingle = jingle_;
}

CallDirection IMCallSession::direction() const {
    auto sender = session->initiator().bareJID();
    auto self = gloox::JID(stdstring(selfId.toString())).bareJID();
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

IMCall::IMCall(IM* im, QObject* parent) : IMJingle(im, parent) {
    qDebug() << __func__ << "...";
    qRegisterMetaType<CallState>("CallState");
    connect(im, &IM::started, this, &IMCall::onImStartedCall);
    connectCall(this);
}

IMCall::~IMCall() {
    qDebug() << __func__ << "...";
}

void IMCall::onImStartedCall() {
    auto client = im->getClient();
    assert(client);

    auto disco = client->disco();
    // jingle av
    disco->addFeature(gloox::XMLNS_JINGLE_ICE_UDP);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_DTLS);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_DTLS_SCTP);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP);
    disco->addFeature(gloox::XMLNS_JINGLE_FEATURE_AUDIO);
    disco->addFeature(gloox::XMLNS_JINGLE_FEATURE_VIDEO);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_FB);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_HDREXT);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_GROUP);

    // session manager
    im->sessionManager()->registerPlugin(new gloox::Jingle::Content());
    im->sessionManager()->registerPlugin(new gloox::Jingle::ICEUDP());
    im->sessionManager()->registerPlugin(new gloox::Jingle::Group());
    im->sessionManager()->registerPlugin(new gloox::Jingle::RTP());
    im->addSessionHandler(this);
}

void IMCall::addCallHandler(CallHandler* hdr) {
    /**
     * callHandlers
     */
    callHandlers.push_back(hdr);
}

void IMCall::connectCall(IMCall* imCall) {
    qDebug() << __func__ << imCall;

    connect(imCall, &IMCall ::receiveCallRequest, this,
            [&](IMPeerId peerId, QString callId, bool audio, bool video) {
                for (auto handler : callHandlers) {
                    handler->onCall(peerId, callId, audio, video);
                }
            });

    connect(imCall, &IMCall::receiveCallRetract, this, [&](QString friendId, CallState state) {
        for (auto handler : callHandlers) {
            handler->onCallRetract(friendId, state);
        }
    });

    connect(imCall, &IMCall::receiveCallAcceptByOther, this,
            [&](const QString& callId, const IMPeerId& peerId) {
                for (auto handler : callHandlers) {
                    handler->onCallAcceptByOther(callId, peerId);
                }
            });

    connect(imCall, &IMCall::receiveCallStateAccepted, this, &IMCall::onCallAccepted);

    connect(imCall, &IMCall::receiveCallStateRejected, this,
            [&](IMPeerId friendId, QString callId, bool video) {
                for (auto handler : callHandlers) {
                    handler->receiveCallStateRejected(friendId, callId, video);
                }
            });

    connect(imCall, &IMCall::receiveFriendHangup, this, [&](QString friendId, CallState state) {
        for (auto handler : callHandlers) {
            handler->onHangup(friendId, state);
        }
    });
}

// 对方接受呼叫
void IMCall::onCallAccepted(IMPeerId peerId, QString callId, bool video) {
    qDebug() << __func__ << "peerId:" << peerId.toString() << "sId" << callId << "video?" << video;
    for (auto handler : callHandlers) {
        handler->receiveCallStateAccepted(peerId, callId, video);
    }
}

bool IMCall::callToFriend(const QString& friendId, const QString& sId, bool video) {
    qDebug() << __func__ << "friend:" << friendId << "sId" << sId << "video?" << video;

    proposeJingleMessage(friendId, sId, video);

    auto resources = im->getOnlineResources(stdstring(friendId));
    if (resources.empty()) {
        qWarning() << "Can not find online friends" << friendId;
        return false;
    }

    proposeJingleMessage(friendId, sId, video);
    qDebug() << "Sent propose jingle message.";
    return true;
}

bool IMCall::callToPeerId(const IMPeerId& to, const QString& sId, bool video) {
    //    qDebug() << QString("peerId:%1 video:%2").arg((to.toString())).arg(video);
    //    auto r = createCall(to, sId, video);
    //    qDebug() << "createdCall=>" << r;
    return true;
}

bool IMCall::callAnswerToFriend(const IMPeerId& f, const QString& callId, bool video) {
    return answer(f, callId, video);
}

void IMCall::callRetract(const IMContactId& f, const QString& sId) {
    cancelCall(f, sId);
}

void IMCall::callReject(const IMPeerId& f, const QString& sId) {
    rejectCall(f, sId);
}

// startCall
bool IMCall::startCall(const QString& friendId, const QString& sId, bool video) {
    qDebug() << __func__ << "friendId:" << friendId << "video:" << video;

    auto resources = im->getOnlineResources(stdstring(friendId));
    if (resources.empty()) {
        qWarning() << "目标用户不在线！";
        return false;
    }

    sendCallToResource(friendId, sId, video);
    return true;
}

bool IMCall::sendCallToResource(const QString& friendId, const QString& sId, bool video) {
    proposeJingleMessage(friendId, sId, video);
    return true;
}

bool IMCall::createCall(const IMPeerId& to, const QString& sId, bool video) {
    qDebug() << __func__ << "to:" << to.toString() << "sId:" << sId << "video:" << video;

    auto rtcManager = lib::ortc::OkRTCManager::getInstance();
    auto rtc = rtcManager->createRtc();
    rtc->addRTCHandler(this);

    const auto& discos = im->getExternalServiceDiscovery();
    for (const auto& item : discos) {
        ortc::IceServer ice;
        ice.uri = item.type + ":" + item.host + ":" + std::to_string(item.port) +
                  "?transport=" + item.transport;
        ice.username = item.username;
        ice.password = item.password;
        qDebug() << "Add ice:" << ice.uri.c_str() << "user:" << qstring(ice.username)
                 << "password:" << qstring(ice.password);
        rtcManager->addIceServer(ice);
    }

    auto created = rtc->CreateOffer(stdstring(to.toString()), stdstring(sId), video);
    qDebug() << __func__ << "CreateOffer=>" << created;
    if (created) {
        createSession(im->getSelfId(), to, sId, ortc::JingleCallType::av);
        currentSid = sId;
        emit callCreated(to, sId, created);
    }
    return created;
}

void IMCall::cancel(const QString& friendId) {
    qDebug() << __func__ << friendId;

    auto sId = m_friendSessionMap.value(IMPeerId(friendId));
    auto session = m_sessionMap.value(sId);
    if (session) {
        cancelCall(IMContactId{friendId}, qstring(session->getSession()->sid()));
        clearSessionInfo(sId);
    }

    currentSid.clear();
}

void IMCall::cancelCall(const IMContactId& friendId, const QString& sId) {
    qDebug() << __func__ << friendId.toString() << sId;

    IMCallSession* s = findSession(sId);
    if (s) {
        s->doTerminate();
        s->setCallStage(CallStage::StageNone);
        clearSessionInfo(sId);
    }
    retractJingleMessage(friendId.toString(), sId);

    currentSid.clear();
    auto rtcManager = ortc::OkRTCManager::getInstance();
    auto rtc = rtcManager->getRtc();
    if (rtc) {
        rtc->removeRTCHandler(this);
    }
}

void IMCall::rejectCall(const IMPeerId& peerId, const QString& sId) {
    qDebug() << __func__ << peerId.toString() << sId;

    IMCallSession* s = findSession(sId);
    if (s) {
        s->doTerminate();
        clearSessionInfo(sId);
    }
    rejectJingleMessage(peerId.toString(), sId);
}

bool IMCall::answer(const IMPeerId& peerId, const QString& callId, bool video) {
    qDebug() << __func__ << "peer:" << peerId.toString() << "callId:" << callId
             << "video:" << video;

    ortc::OkRTCManager* rtcManager = lib::ortc::OkRTCManager::getInstance();
    auto rtc = rtcManager->createRtc();
    rtc->addRTCHandler(this);

    const auto& discos = im->getExternalServiceDiscovery();
    for (const auto& item : discos) {
        ortc::IceServer ice;
        ice.uri = item.type + ":" + item.host + ":" + std::to_string(item.port) +
                  "?transport=" + item.transport;
        ice.username = item.username;
        ice.password = item.password;
        qDebug() << "Add iceServer:" << ice.uri.c_str();
        rtcManager->addIceServer(ice);
    }

    acceptJingleMessage(peerId, callId, video);

    return true;
}

void IMCall::setMute(bool mute) {
    lib::ortc::OkRTCManager::getInstance()->getRtc()->setMute(mute);
}

void IMCall::setRemoteMute(bool mute) {
    lib::ortc::OkRTCManager::getInstance()->getRtc()->setRemoteMute(mute);
}

void IMCall::onCreatePeerConnection(const std::string& sId, const std::string& peerId, bool ok) {
    auto p = qstring(peerId);
    auto s = qstring(sId);
    qDebug() << __func__ << "sId:" << s << "peerId:" << p << "isOk=>" << ok;
}

void IMCall::onFailure(const std::string& sId,
                       const std::string& peerId,
                       const std::string& error) {
    const QString& qsId = qstring(sId);
    const QString& qPeerId = qstring(peerId);
    qDebug() << __func__ << "sId:" << qsId << "peerId:" << qPeerId;
    qDebug() << "error:" << qstring(error);

    // TODO 通知前台处理错误情况
}

void IMCall::onIceGatheringChange(const std::string& sId, const std::string& peerId,
                                  ortc::IceGatheringState state) {
    const QString& qsId = qstring(sId);
    const QString& qPeerId = qstring(peerId);
    qDebug() << __func__ << "sId:" << qsId << "peerId:" << qPeerId
             << "state:" << static_cast<int>(state);

    emit iceGatheringStateChanged(IMPeerId(qPeerId), qsId, state);

    if (state == ortc::IceGatheringState::Complete) {
        doForIceCompleted(qsId, qPeerId);
    }
}

/**
 * Ice交互完成，处理事项
 * @param sId
 * @param peerId
 * @param qsId
 */
void IMCall::doForIceCompleted(const QString& sId, const QString& peerId) {
    auto pSession = findSession(sId);
    if (!pSession) {
        qWarning() << "Unable to find jingle session:" << sId;
        return;
    }

    ortc::OkRTC* rtc = ortc::OkRTCManager::getInstance()->getRtc();
    auto av = rtc->getLocalSdp(stdstring(peerId));

    gloox::Jingle::PluginList plugins;
    ToPlugins(av.get(), plugins);

    if (pSession->direction() == CallDirection::CallIn) {
        pSession->getSession()->sessionAccept(plugins);
    } else if (pSession->direction() == CallDirection::CallOut) {
        pSession->getSession()->sessionInitiate(plugins);
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
}

void IMCall::onPeerConnectionChange(const std::string& sId, const std::string& peerId,
                                    ortc::PeerConnectionState state) {
    /**
     * OnConnectionChange : connecting
     * OnConnectionChange : connected
     * OnConnectionChange : closed
     */

    for (const auto& item : callHandlers) {
        assert(item);
        item->onPeerConnectionChange(IMPeerId{peerId}, qstring(sId), state);
    }
}

void IMCall::onSignalingChange(const std::string& sId, const std::string& peerId,
                               lib::ortc::SignalingState state) {
    /**
     * OnSignalingChange=>have-local-offer
     * OnSignalingChange=>stable
     * OnSignalingChange=>closed
     */
    qDebug() << __func__ << "sId:" << state;
}

void IMCall::onRTP(const std::string& sid,     //
                   const std::string& peerId,  //
                   const ortc::OJingleContentAv& oContext) {
    auto sId = qstring(sid);
    qDebug() << __func__ << "sId:" << sId << "peerId:" << qstring(peerId);

    gloox::Jingle::PluginList plugins;
    ToPlugins(&oContext, plugins);

    auto pSession = findSession(sId);
    if (!pSession) {
        qWarning() << "Unable to find session" << &sId;
        return;
    }

    if (pSession->direction() == CallDirection::CallIn) {
        pSession->getSession()->sessionAccept(plugins);
    } else if (pSession->direction() == CallDirection::CallOut) {
        pSession->getSession()->sessionInitiate(plugins);
    }
}

void IMCall::onIce(const std::string& sId,     //
                   const std::string& peerId,  //
                   const lib::ortc::OIceUdp& oIceUdp) {
    auto sid = qstring(sId);
    if (sid.isEmpty()) {
        qWarning() << "sid is empty.";
        return;
    }

    qDebug() << __func__ << "sId:" << sid << "peerId:" << qstring(peerId)
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
    if (currentSid.isEmpty()) {
        return false;
    }

    auto sId = qstring(session->sid());
    ortc::OJingleContentAv av;
    av.sdpType = ortc::JingleSdpType::Answer;
    ParseAV(jingle, av);

    if (!av.isValid()) {
        qWarning() << "Is no call session";
        return false;
    }

    auto pSession = m_sessionMap.value(sId);
    if (!pSession) {
        // 创建session
        auto selfId = im->getSelfId();
        pSession = new IMCallSession(sId, session, selfId, peerId, ortc::JingleCallType::av);
        m_sessionMap.insert(sId, pSession);
    }

    // RTC 接受会话
    auto rtc = ortc::OkRTCManager::getInstance()->getRtc();
    const std::string& id = stdstring(peerId.toString());
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
    qDebug() << __func__ << "peerId:" << peerId.toString() << "sId:" << qstring(jm->id())
             << "action:" << gloox::Jingle::ActionValues[jm->action()];

    auto friendId = peerId.toFriendId();
    qDebug() << "friendId:" << friendId;

    auto sId = qstring(jm->id());

    switch (jm->action()) {
        case gloox::Jingle::JingleMessage::reject: {
            /**
             * 对方拒绝
             */
            //      mPeerRequestMedias.clear();
            const auto& ms = jm->medias();
            emit receiveCallStateRejected(peerId, sId, ms.size() > 1);
            //      emit receiveFriendHangup(friendId, 0);
            break;
        }
        case gloox::Jingle::JingleMessage::propose: {
            // 被对方发起呼叫
            qDebug() << "On call from:" << peerId.toString();

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

            emit receiveCallRequest(peerId, sId, audio, video);
            //      emit receiveCallAcceptByOther(sId, peerId);
            break;
        }
        case gloox::Jingle::JingleMessage::retract: {
            /**
             * 撤回(需要判断是对方还是自己其它终端)
             */
            emit receiveCallRetract(friendId, CallState::NONE);
            break;
        }
        case gloox::Jingle::JingleMessage::accept: {
            // 自己其它终端接受，挂断自己
            if (peerId != im->getSelfPeerId()) {
                emit receiveFriendHangup(friendId, CallState::NONE);
            } else {
                // 自己终端接受，不处理
                //            OkRTCManager::getInstance()->getRtc()->CreateAnswer(peerId.toString());
            }
            break;
        }
        case gloox::Jingle::JingleMessage::proceed: {
            // 对方接受
            auto removed = m_sidVideo.remove(sId);
            createCall(peerId, sId, removed == 1);
            emit receiveCallStateAccepted(peerId, sId, removed == 1);
            break;
        }
        case gloox::Jingle::JingleMessage::finish:
            break;
    }
}

void IMCall::proposeJingleMessage(const QString& friendId, const QString& callId, bool video) {
    qDebug() << __func__ << "friend:" << friendId << callId;

    gloox::StanzaExtensionList exts;
    auto jm = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::propose,
                                               stdstring(callId));
    jm->addMedia(gloox::Jingle::Media::audio);
    if (video) {
        jm->addMedia(gloox::Jingle::Media::video);
        m_sidVideo.insert(callId, true);
    }
    exts.push_back(jm);

    auto jid = gloox::JID{stdstring(friendId)};
    gloox::Message m(gloox::Message::Chat, jid, {}, {});
    for (auto ext : exts) m.addExtension(ext);

    im->getClient()->send(m);
}

void IMCall::rejectJingleMessage(const QString& peerId, const QString& callId) {
    qDebug() << __func__ << "friend:" << peerId << callId;

    gloox::StanzaExtensionList exts;
    auto reject = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::reject,
                                                   stdstring(callId));
    exts.push_back(reject);

    auto jid = gloox::JID{stdstring(peerId)};
    gloox::Message m(gloox::Message::Chat, jid, {}, {});
    for (auto ext : exts) m.addExtension(ext);

    im->getClient()->send(m);
}

void IMCall::retractJingleMessage(const QString& friendId, const QString& callId) {
    qDebug() << __func__ << "friend:" << friendId << callId;

    auto* jm = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::retract,
                                                stdstring(callId));

    auto jid = gloox::JID{stdstring(friendId)};
    gloox::Message m(gloox::Message::Chat, jid, {}, {});
    m.addExtension(jm);

    im->getClient()->send(m);
}

void IMCall::acceptJingleMessage(const IMPeerId& peerId, const QString& callId, bool video) {
    qDebug() << __func__ << "friend:" << peerId.toFriendId() << callId;

    auto proceed = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::proceed,
                                                    stdstring(callId));
    gloox::Message proceedMsg(gloox::Message::Chat, gloox::JID(stdstring(peerId.toString())));
    proceedMsg.addExtension(proceed);
    im->getClient()->send(proceedMsg);
    qDebug() << "Sent proceed=>" << peerId.toString();

    // 发送给自己其它终端
    auto accept = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::accept,
                                                   stdstring(callId));

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
                                        const QString& sId,
                                        lib::ortc::JingleCallType callType) {
    qDebug() << __func__ << "to:" << to.toString();
    auto session =
            getIM()->createSession(gloox::JID(stdstring(to.toString())), stdstring(sId), this);

    m_friendSessionMap.insert(to, sId);

    auto ws = new IMCallSession(sId, session, from, to, ortc::JingleCallType::av);
    m_sessionMap.insert(sId, ws);

    return ws;
}

void IMCall::clearSessionInfo(const QString& sId) {
    qDebug() << __func__ << sId;
    auto session = m_sessionMap.value(sId);
    if (!session) {
        return;
    }

    auto s = session->getSession();
    auto& responder = s->remote();
    m_friendSessionMap.remove(IMPeerId(responder));
    m_sessionMap.remove(sId);
}

IMCallSession* IMCall::createSession(const IMContactId& self,
                                     const IMPeerId& peer,
                                     const QString& sId,
                                     ortc::JingleCallType ct) {
    return cacheSessionInfo(self, peer, sId, ct);
}

void IMCall::handleJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm) {
    doJingleMessage(peerId, jm);
}

bool IMCall::doSessionInfo(const gloox::Jingle::Session::Jingle* jingle, const IMPeerId& friendId) {
    qDebug() << "jingle:%1 peerId:%2"   //
             << qstring(jingle->sid())  //
             << friendId.toString();
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
    auto sid = qstring(jingle->sid());
    qDebug() << __func__ << "sId:" << sid << "peerId:" << peerId.toString();

    if (currentSid.isEmpty()) {
        return false;
    }

    auto s = findSession(sid);
    if (!s) {
        qWarning() << ("Session is no existing.");
        return false;
    }

    ortc::OJingleContentAv av;
    ParseAV(jingle, av);

    for (auto& kv : av.getContents()) {
        auto& oSdp = kv.second;
        ortc::OkRTCManager::getInstance()
                ->getRtc()  //
                ->setTransportInfo(stdstring(peerId.toString()), jingle->sid(), oSdp.iceUdp);
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

bool IMCall::doSessionInitiate(gloox::Jingle::Session* session,
                               const gloox::Jingle::Session::Jingle* jingle,
                               const IMPeerId& peerId) {
    auto& from = session->remote();
    if (from.server().starts_with("conference.")) {
        return false;
    }

    auto sId = qstring(jingle->sid());
    qDebug() << __func__ << "sid:" << sId;

    ortc::OJingleContentAv cav;
    ParseAV(jingle, cav);
    if (!cav.isValid()) {
        qDebug() << "Is no av session!";
        return false;
    }

    cav.sdpType = lib::ortc::JingleSdpType::Offer;
    ortc::OkRTCManager::getInstance()->getRtc()->CreateAnswer(stdstring(peerId.toString()), cav);
    currentSid = sId;
    return true;
}

bool IMCall::doSessionTerminate(gloox::Jingle::Session* session,
                                const gloox::Jingle::Session::Jingle*,
                                const lib::messenger::IMPeerId& peerId) {
    if (currentSid.isEmpty()) {
        return false;
    }

    auto sId = qstring(session->sid());
    qDebug() << __func__ << "sId:" << sId;
    auto s = findSession(sId);
    if (s) {
        s->doTerminate();
    }

    emit receiveFriendHangup(peerId.toFriendId(), CallState::FINISHED);

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
