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
#include <extdisco.h>
#include <jinglegroup.h>
#include <jingleiceudp.h>
#include <jinglertp.h>

#include "IM.h"
#include "IMCall.h"
#include "lib/ortc/ok_rtc.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "lib/ortc/ok_rtc_manager.h"
#include "lib/session/AuthSession.h"

namespace lib::messenger {

inline void parseCandidates(gloox::Jingle::ICEUDP::CandidateList& src, ortc::CandidateList& to) {
    for (auto& c : src) {
        to.push_front({c.component, c.foundation, c.generation, c.id, c.ip, c.network, c.port,
                       c.priority, c.protocol, c.tcptype, c.rel_addr, c.rel_port,
                       static_cast<ortc::Type>(c.type)});
    }
}

inline void packCandidates(const ortc::CandidateList& src,
                           gloox::Jingle::ICEUDP::CandidateList& to) {
    for (auto& c : src) {
        to.push_front({c.component, c.foundation, c.generation, c.id, c.ip, c.network, c.port,
                       c.priority, c.protocol, c.tcptype, c.rel_addr, c.rel_port,
                       static_cast<gloox::Jingle::ICEUDP::Type>(c.type)});
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

IMCallSession::~IMCallSession() { qDebug() << __func__ << sId; }

gloox::Jingle::Session* IMCallSession::getSession() const { return session; }

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

void IMCallSession::createOffer(const std::string& peerId) {
    qDebug() << __func__ << "to" << peerId.c_str();
    auto rm = lib::ortc::OkRTCManager::getInstance();
    auto r = rm->getRtc();
    r->CreateOffer(peerId);
}

const gloox::Jingle::Session::Jingle* IMCallSession::getJingle() const { return jingle; }

void IMCallSession::setJingle(const gloox::Jingle::Session::Jingle* jingle_) { jingle = jingle_; }

CallDirection IMCallSession::direction() const {
    auto sender = session->initiator().bareJID();
    auto self = gloox::JID(stdstring(selfId.toString())).bareJID();
    return (sender == self) ? CallDirection::CallOut : CallDirection::CallIn;
}

void IMCallSession::setCallStage(CallStage state) { m_callStage = state; }

void IMCallSession::setContext(const ortc::OJingleContent& jc) { context = jc; }

void IMCallSession::start() {}

void IMCallSession::stop() {}

IMCall::IMCall(IM* im, QObject* parent) : IMJingle(im, parent) {
    qDebug() << __func__ << "...";

    qRegisterMetaType<CallState>("CallState");

    connect(im, &IM::started, this, &IMCall::onImStartedCall);
    connectCall(this);
}

IMCall::~IMCall() {}

void IMCall::onImStartedCall() {
    auto client = _im->getClient();
    assert(client);

    auto disco = client->disco();
    // jingle av
    disco->addFeature(gloox::XMLNS_JINGLE_ICE_UDP);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_DTLS);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP);
    disco->addFeature(gloox::XMLNS_JINGLE_FEATURE_AUDIO);
    disco->addFeature(gloox::XMLNS_JINGLE_FEATURE_VIDEO);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_FB);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_SSMA);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_RTP_HDREXT);
    disco->addFeature(gloox::XMLNS_JINGLE_APPS_GROUP);

    // session manager
    _im->sessionManager()->registerPlugin(new gloox::Jingle::Content());
    _im->sessionManager()->registerPlugin(new gloox::Jingle::ICEUDP());
    _im->sessionManager()->registerPlugin(new gloox::Jingle::Group());
    _im->sessionManager()->registerPlugin(new gloox::Jingle::RTP());
    _im->addSessionHandler(this);

    auto rtcManager = ortc::OkRTCManager::getInstance();

    std::list<gloox::ExtDisco::Service> discos;

    gloox::ExtDisco::Service disco0;
    disco0.type = "turn";
    disco0.host = "chuanshaninfo.com";
    disco0.port = 34780;
    disco0.username = "gaojie";
    disco0.password = "hncs";
    discos.push_back(disco0);

    gloox::ExtDisco::Service disco1;
    disco1.type = "stun";
    disco1.host = "stun.l.google.com";
    disco1.port = 19302;

    discos.push_back(disco1);

    for (const auto& item : discos) {
        ortc::IceServer ice;
        ice.uri = item.type + ":" + item.host + ":" + std::to_string(item.port);
        //              "?transport=" + item.transport;
        ice.username = item.username;
        ice.password = item.password;
        qDebug() << "Add ice:" << ice.uri.c_str();
        rtcManager->addIceServer(ice);
    }
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

    auto resources = _im->getOnlineResources(stdstring(friendId));
    if (resources.empty()) {
        qWarning() << "Can not find online friends" << friendId;
        return false;
    }

    proposeJingleMessage(friendId, sId, video);
    qDebug() << "Sent propose jingle message.";
    return true;
}

bool IMCall::callToPeerId(const IMPeerId& to, const QString& sId, bool video) {
    qDebug() << QString("peerId:%1 video:%2").arg((to.toString())).arg(video);
    auto r = createCall(to, sId, video);
    qDebug() << "createdCall=>" << r;
    return r;
}

bool IMCall::callAnswerToFriend(const IMPeerId& f, const QString& callId, bool video) {
    return answer(f, callId, video);
}

void IMCall::callRetract(const IMContactId& f, const QString& sId) { cancelCall(f, sId); }

void IMCall::callReject(const IMPeerId& f, const QString& sId) { rejectCall(f, sId); }

// startCall
bool IMCall::startCall(const QString& friendId, const QString& sId, bool video) {
    qDebug() << __func__ << "friendId:" << friendId << "video:" << video;

    auto resources = _im->getOnlineResources(stdstring(friendId));
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
    qDebug() << __func__ << "to:" << to.toString() << "sId:" << sId;

    auto ws = createSession(_im->getSelfId(), to, sId, lib::ortc::JingleCallType::av);

    auto rtc = lib::ortc::OkRTCManager::getInstance()->getRtc();
    rtc->addRTCHandler(this);

    bool createdCall = rtc->call(stdstring(to.toString()), stdstring(sId), video);
    if (createdCall) {
        ws->createOffer(stdstring(to.toString()));
    }

    return createdCall;
}

void IMCall::cancel(const QString& friendId) {
    qDebug() << __func__ << friendId;

    auto sId = m_friendSessionMap.value(IMPeerId(friendId));

    auto session = m_sessionMap.value(sId);
    if (session) {
        cancelCall(IMContactId{friendId}, qstring(session->getSession()->sid()));
        clearSessionInfo(sId);
    }
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

    auto rtc = lib::ortc::OkRTCManager::getInstance()->getRtc();
    rtc->addRTCHandler(this);

    acceptJingleMessage(peerId, callId, video);

    return true;
}

void IMCall::setMute(bool mute) { lib::ortc::OkRTCManager::getInstance()->getRtc()->setMute(mute); }

void IMCall::setRemoteMute(bool mute) {
    lib::ortc::OkRTCManager::getInstance()->getRtc()->setRemoteMute(mute);
}

void IMCall::onCreatePeerConnection(const std::string& sId, const std::string& peerId, bool ok) {
    auto p = qstring(peerId);
    auto s = qstring(sId);
    qDebug() << __func__ << "sId:" << s << "peerId:" << p << "isOk=>" << ok;
}

void IMCall::onRTP(const std::string& sid,     //
                   const std::string& peerId,  //
                   const ortc::OJingleContentAv& oContext) {
    auto sId = qstring(sid);
    qDebug() << __func__ << "sId:" << sId << "peerId:" << qstring(peerId);

    gloox::Jingle::PluginList plugins;
    toPlugins(oContext, plugins);

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
             << "mid:" << qstring(oIceUdp.mid) << "mline:" << oIceUdp.mline;

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
void IMCall::onRender(const std::string& peerId, lib::ortc::RendererImage image) {
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

void IMCall::doSessionAccept(gloox::Jingle::Session* session,
                             const gloox::Jingle::Session::Jingle* jingle,
                             const lib::messenger::IMPeerId& peerId) {
    auto sId = qstring(session->sid());

    if (isInvalidSid(sId)) return;

    lib::ortc::OJingleContentAv cav;
    cav.sdpType = lib::ortc::JingleSdpType::Answer;
    parse(jingle, cav);

    if (!cav.isValid()) {
        qWarning() << "Is no call session";
        return;
    }

    auto sess = m_sessionMap.value(sId);
    if (!sess) {
        // 创建session
        // self id
        auto selfId = _im->getSelfId();
        sess = new IMCallSession(sId, session, selfId, peerId, ortc::JingleCallType::av);
        m_sessionMap.insert(sId, sess);
    }

    // RTC 接受会话
    lib::ortc::OkRTCManager::getInstance()
            ->getRtc()  //
            ->setRemoteDescription(stdstring(peerId.toString()), cav);
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
            if (peerId != _im->getSelfPeerId()) {
                emit receiveFriendHangup(friendId, CallState::NONE);
            } else {
                // 自己终端接受，不处理
                //            OkRTCManager::getInstance()->getRtc()->CreateAnswer(peerId.toString());
            }
            break;
        }
        case gloox::Jingle::JingleMessage::proceed: {
            // 对方接受
            auto removed = m_sidVideo.remove(sId);  // 确定发起的是否是视频？
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

    _im->getClient()->send(m);
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

    _im->getClient()->send(m);
}

void IMCall::retractJingleMessage(const QString& friendId, const QString& callId) {
    qDebug() << __func__ << "friend:" << friendId << callId;

    auto* jm = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::retract,
                                                stdstring(callId));

    auto jid = gloox::JID{stdstring(friendId)};
    gloox::Message m(gloox::Message::Chat, jid, {}, {});
    m.addExtension(jm);

    _im->getClient()->send(m);
}

void IMCall::acceptJingleMessage(const IMPeerId& peerId, const QString& callId, bool video) {
    qDebug() << __func__ << "friend:" << peerId.toFriendId() << callId;

    auto proceed = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::proceed,
                                                    stdstring(callId));
    gloox::Message proceedMsg(gloox::Message::Chat, gloox::JID(stdstring(peerId.toString())));
    proceedMsg.addExtension(proceed);
    _im->getClient()->send(proceedMsg);
    qDebug() << "Sent proceed=>" << peerId.toString();

    // 发送给自己其它终端
    auto accept = new gloox::Jingle::JingleMessage(gloox::Jingle::JingleMessage::accept,
                                                   stdstring(callId));

    auto self = _im->self().bareJID();

    gloox::Message msg(gloox::Message::Chat, self);
    msg.addExtension(accept);
    _im->getClient()->send(msg);
    qDebug() << "Sent accept=>" << qstring(self.full());

    // 设置状态为接受
    auto ws = findSession(callId);
    if (!ws) {
        ws = createSession(_im->getSelfId(), peerId, callId, ortc::JingleCallType::av);
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

void IMCall::doSessionInfo(const gloox::Jingle::Session::Jingle* jingle, const IMPeerId& friendId) {
    qDebug() << "jingle:%1 peerId:%2"   //
             << qstring(jingle->sid())  //
             << friendId.toString();
}

void IMCall::doContentAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doContentRemove(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doContentModify(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doContentAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doContentReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doTransportInfo(const gloox::Jingle::Session::Jingle* jingle, const IMPeerId& peerId) {
    auto sid = qstring(jingle->sid());
    qDebug() << __func__ << "sid" << sid;

    if (isInvalidSid(sid)) {
        qWarning() << "Unable to handle the session!";
        return;
    }

    qDebug() << __func__ << "sId:" << sid << "peerId:" << peerId.toString();

    auto s = findSession(sid);
    if (!s) {
        qWarning() << ("Session is no existing.");
        return;
    }

    ortc::OJingleContentAv content;
    parse(jingle, content);

    for (auto& it : content.contents) {
        ortc::OkRTCManager::getInstance()
                ->getRtc()  //
                ->setTransportInfo(stdstring(peerId.toString()), jingle->sid(), it.iceUdp);
    }
}

void IMCall::doTransportAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doTransportReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doTransportReplace(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doSecurityInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doDescriptionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doInvalidAction(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {}

void IMCall::doSessionInitiate(gloox::Jingle::Session* session,
                               const gloox::Jingle::Session::Jingle* jingle,
                               const lib::messenger::IMPeerId& peerId) {
    auto sId = qstring(jingle->sid());
    qDebug() << __func__ << "sid:" << sId;

    ortc::OJingleContentAv cav;
    parse(jingle, cav);
    if (!cav.isValid()) {
        addInvalidSid(sId);
        qDebug() << "Is not av session!";
        return;
    }

    cav.sdpType = lib::ortc::JingleSdpType::Offer;
    ortc::OkRTCManager::getInstance()->getRtc()->CreateAnswer(stdstring(peerId.toString()), cav);
}

void IMCall::doSessionTerminate(gloox::Jingle::Session* session,
                                const gloox::Jingle::Session::Jingle*,
                                const lib::messenger::IMPeerId& peerId) {
    auto sId = qstring(session->sid());
    qDebug() << __func__ << "sId:" << sId;
    if (isInvalidSid(sId)) {
        return;
    }

    auto s = findSession(sId);
    if (s) {
        s->doTerminate();
    }
    clearSessionInfo(sId);
    emit receiveFriendHangup(peerId.toFriendId(), CallState::FINISHED);
}

void IMCall::toPlugins(const ortc::OJingleContentAv& oContext, gloox::Jingle::PluginList& plugins) {
    //<group>
    auto contents = oContext.contents;
    gloox::Jingle::Group::ContentList contentList;
    for (auto& content : contents) {
        auto name = content.name;
        auto desc = content.rtp;

        contentList.push_back(gloox::Jingle::Group::Content{name});

        // description
        gloox::Jingle::PluginList rtpPlugins;

        // rtcp
        gloox::Jingle::RTP::PayloadTypes pts;
        for (auto x : desc.payloadTypes) {
            gloox::Jingle::RTP::PayloadType t;
            t.id = x.id;
            t.name = x.name;
            t.clockrate = x.clockrate;
            t.bitrate = x.bitrate;
            t.channels = x.channels;

            for (auto p : x.parameters) {
                gloox::Jingle::RTP::Parameter p0;
                p0.name = p.name;
                p0.value = p.value;
                t.parameters.push_back(p0);
            }

            for (auto f : x.feedbacks) {
                gloox::Jingle::RTP::Feedback f0;
                f0.type = f.type;
                f0.subtype = f.subtype;
                t.feedbacks.push_back(f0);
            }
            pts.push_back(t);
        }
        auto rtp = new gloox::Jingle::RTP(static_cast<gloox::Jingle::Media>(desc.media), pts);
        rtp->setRtcpMux(desc.rtcpMux);

        // payload-type
        rtp->setPayloadTypes(pts);

        // rtp-hdrExt
        gloox::Jingle::RTP::HdrExts exts;
        for (auto e : desc.hdrExts) {
            exts.push_back({e.id, e.uri});
        }
        rtp->setHdrExts(exts);

        // source
        if (!desc.sources.empty()) {
            gloox::Jingle::RTP::Sources ss;
            for (auto s : desc.sources) {
                gloox::Jingle::RTP::Parameters ps;
                for (auto p : s.parameters) {
                    ps.push_back({p.name, p.value});
                }
                ss.push_back({s.ssrc, ps});
            }

            rtp->setSources(ss);
        }

        // ssrc-group
        if (!desc.ssrcGroup.ssrcs.empty()) {
            gloox::Jingle::RTP::SsrcGroup sg;
            sg.semantics = desc.ssrcGroup.semantics;
            for (auto s : desc.ssrcGroup.ssrcs) {
                sg.ssrcs.push_back(s);
            }
            rtp->setSsrcGroup(sg);
        }

        // rtp
        rtpPlugins.emplace_back(rtp);

        // transport
        lib::ortc::OIceUdp oIceUdp = content.iceUdp;

        gloox::Jingle::ICEUDP::CandidateList cl;
        for (auto c : oIceUdp.candidates) {
            cl.push_front({c.component, c.foundation, c.generation, c.id, c.ip, c.network, c.port,
                           c.priority, c.protocol, c.tcptype, c.rel_addr, c.rel_port,
                           static_cast<gloox::Jingle::ICEUDP::Type>(c.type)});
        }
        auto ice = new gloox::Jingle::ICEUDP(oIceUdp.pwd, oIceUdp.ufrag, cl);
        ice->setDtls({.hash = oIceUdp.dtls.hash,
                      .setup = oIceUdp.dtls.setup,
                      .fingerprint = oIceUdp.dtls.fingerprint});
        rtpPlugins.emplace_back(ice);

        auto* pContent =
                new gloox::Jingle::Content(name, rtpPlugins, gloox::Jingle::Content::CInitiator);
        plugins.emplace_back(pContent);
    }

    auto group = new gloox::Jingle::Group("BUNDLE", contentList);
    plugins.push_back(group);
}

void IMCall::parse(const gloox::Jingle::Session::Jingle* jingle,
                   ortc::OJingleContentAv& oContextAv) {
    oContextAv.sessionId = jingle->sid();
    int mid = 0;
    for (const auto p : jingle->plugins()) {
        gloox::Jingle::JinglePluginType pt = p->pluginType();
        switch (pt) {
            case gloox::Jingle::PluginContent: {
                ortc::OSdp oContent;

                const auto* content = static_cast<const gloox::Jingle::Content*>(p);
                oContent.name = content->name();

                const auto* rtp = content->findPlugin<gloox::Jingle::RTP>(gloox::Jingle::PluginRTP);
                if (rtp) {
                    // 存在rtp则设置类型
                    oContextAv.callType = ortc::JingleCallType::av;

                    ortc::SsrcGroup ssrcGroup = {.semantics = rtp->ssrcGroup().semantics,
                                                 .ssrcs = rtp->ssrcGroup().ssrcs};
                    ortc::ORTP description = {
                            static_cast<ortc::Media>(rtp->media()),                         //
                            (const std::list<lib::ortc::PayloadType>&)rtp->payloadTypes(),  //
                            (const std::list<lib::ortc::HdrExt>&)rtp->hdrExts(),            //
                            (const std::list<lib::ortc::Source>&)rtp->sources(),            //
                            ssrcGroup,                                                      //
                            rtp->rtcpMux()                                                  //
                    };
                    oContent.rtp = description;
                }

                const auto* udp =
                        content->findPlugin<gloox::Jingle::ICEUDP>(gloox::Jingle::PluginICEUDP);
                if (udp) {
                    ortc::OIceUdp iceUdp = {
                            .mid = std::to_string(mid),         //
                            .mline = mid,                       //
                            .ufrag = udp->ufrag(),              //
                            .pwd = udp->pwd(),                  //
                            .dtls = {.hash = udp->dtls().hash,  //
                                     .setup = udp->dtls().setup,
                                     .fingerprint = udp->dtls().fingerprint},
                            .candidates =
                                    (const std::list<lib::ortc::Candidate>&)udp->candidates()  //
                    };
                    oContent.iceUdp = iceUdp;
                }

                oContextAv.contents.push_back(oContent);
                break;
            }
            case gloox::Jingle::PluginNone:
                break;
            case gloox::Jingle::PluginFileTransfer:
                break;
            case gloox::Jingle::PluginICEUDP:
                break;
            case gloox::Jingle::PluginReason:
                break;
            case gloox::Jingle::PluginUser:
                break;
            case gloox::Jingle::PluginGroup:
                break;
            case gloox::Jingle::PluginRTP:
                break;
            case gloox::Jingle::PluginIBB:
                break;
            default:
                break;
        }
        mid++;
    }
}

}  // namespace lib::messenger
