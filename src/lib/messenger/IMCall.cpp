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
#include <jinglegroup.h>
#include <jingleiceudp.h>
#include <jinglertp.h>

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

IMCall::IMCall(IM* im, QObject* parent) : IMJingle(im, parent) {}

void IMCall::addCallHandler(CallHandler* hdr) { callHandlers.push_back(hdr); }

bool IMCall::callToGroup(const QString& g) { return false; }

void IMCall::connectJingle(IMJingle* _jingle) {
    qDebug() << __func__ << _jingle;

    /**
     * callHandlers
     */
    connect(_jingle, &IMCall ::receiveCallRequest, this,
            [&](IMPeerId peerId, QString callId, bool audio, bool video) {
                for (auto handler : callHandlers) {
                    handler->onCall(peerId, callId, audio, video);
                }
            });

    connect(_jingle, &IMCall::receiveCallRetract, this, [&](QString friendId, int state) {
        for (auto handler : callHandlers) {
            handler->onCallRetract(friendId, state);
        }
    });

    connect(_jingle, &IMCall::receiveCallAcceptByOther, this,
            [&](const QString& callId, const IMPeerId& peerId) {
                for (auto handler : callHandlers) {
                    handler->onCallAcceptByOther(callId, peerId);
                }
            });

    connect(_jingle, &IMCall::receiveCallStateAccepted, this, &IMCall::onCallAccepted);

    connect(_jingle, &IMCall::receiveCallStateRejected, this,
            [&](IMPeerId friendId, QString callId, bool video) {
                for (auto handler : callHandlers) {
                    handler->receiveCallStateRejected(friendId, callId, video);
                }
            });

    connect(_jingle, &IMCall::receiveFriendHangup, this, [&](QString friendId, int state) {
        for (auto handler : callHandlers) {
            handler->onHangup(friendId, (CallState)state);
        }
    });

    connect(_jingle, &IMCall::receiveFriendHangup, this, [&](QString friendId, int state) {
        for (auto handler : callHandlers) {
            handler->onHangup(friendId, (CallState)state);
        }
    });

    //  connect(_jingle, &IMCall::receiveFriendVideoFrame,
    //          this,
    //          [&](const QString &friendId, //
    //              uint16_t w, uint16_t h,  //
    //              const uint8_t *y, const uint8_t *u, const uint8_t *v,
    //              int32_t ystride, int32_t ustride, int32_t vstride) {
    //            emit receiveFriendVideoFrame(friendId, //
    //                                         w, h,     //
    //                                         y, u, v,  //
    //                                         ystride, ustride, vstride);
    //          });
    //
    //  connect(_jingle, &IMCall::receiveSelfVideoFrame, this,
    //          [&](uint16_t w, uint16_t h, //
    //              const uint8_t *y, const uint8_t *u, const uint8_t *v,
    //              int32_t ystride, int32_t ustride, int32_t vstride) {
    //            emit receiveSelfVideoFrame(w, h,    //
    //                                       y, u, v, //
    //                                       ystride, ustride, vstride);
    //          });
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
    qDebug() << __func__ << "to:" << to.toString() << "sId:" << sId;

    auto ws = createSession(im->getSelfId(), to, sId, lib::ortc::JingleCallType::av);

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
        clearSessionInfo(session->getSession());
    }
}

void IMCall::cancelCall(const IMContactId& friendId, const QString& sId) {
    qDebug() << __func__ << friendId.toString() << sId;

    IMJingleSession* s = findSession(sId);
    if (s) {
        s->doTerminate();
        s->setCallStage(CallStage::StageNone);
        clearSessionInfo(s->getSession());
    }
    retractJingleMessage(friendId.toString(), sId);
    //  else {
    // jingle-message
    //    if (s->direction() == CallDirection:: CallOut) {
    //    } else if (s->direction() == CallDirection:: CallIn) {
    //      rejectJingleMessage(friendId.toString(), sId);
    //    }
    //  }
}

void IMCall::rejectCall(const IMPeerId& peerId, const QString& sId) {
    qDebug() << __func__ << peerId.toString() << sId;

    IMJingleSession* s = findSession(sId);
    if (s) {
        s->doTerminate();
        clearSessionInfo(s->getSession());
    } else {
        rejectJingleMessage(peerId.toString(), sId);
    }
}

bool IMCall::answer(const IMPeerId& peerId, const QString& callId, bool video) {
    qDebug() << __func__ << "peer:" << peerId.toString() << "video:" << video;

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

    //  emit call->sig_createPeerConnection(s, p, ok);
}

void IMCall::onRTP(const std::string& sid,     //
                   const std::string& peerId,  //
                   const ortc::OJingleContentAv& oContext) {
    auto sId = qstring(sid);
    qDebug() << __func__ << "sId:" << sId << "peerId:" << qstring(peerId);

    PluginList plugins;
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

    PluginList pluginList;
    pluginList.push_back(iceUdp);
    auto c = new Jingle::Content(oIceUdp.mid, pluginList);
    session->getSession()->transportInfo(c);
}

/**
 * 视频渲染
 * @param peerId
 * @param image
 */
void IMCall::onRender(const std::string& peerId, lib::ortc::RendererImage image) {
    if (peerId.empty()) {
        emit receiveSelfVideoFrame(image.width_, image.height_, image.y, image.u, image.v,
                                   image.ystride, image.ustride, image.vstride);
    } else {
        emit receiveFriendVideoFrame(IMPeerId(peerId).toFriendId(), image.width_, image.height_,
                                     image.y, image.u, image.v, image.ystride, image.ustride,
                                     image.vstride);
    }
}

void IMCall::sessionOnAccept(const QString& sId,
                             Jingle::Session* session,
                             const IMPeerId& peerId,
                             const Jingle::Session::Jingle* jingle) {
    auto sess = m_sessionMap.value(sId);
    if (sess) {
        qWarning() << "Call session is existing, the sId is" << sId;
        return;
    }
    // self id
    auto selfId = im->getSelfId();

    // 创建session
    for (auto& file : m_sessionMap) {
        auto s = new IMJingleSession(sId, session, selfId, peerId, ortc::JingleCallType::av);
        m_sessionMap.insert(sId, s);
    }

    lib::ortc::OJingleContentAv cav;
    cav.sdpType = lib::ortc::JingleSdpType::Answer;
    parse(jingle->plugins(), cav);

    // RTC 接受会话
    lib::ortc::OkRTCManager::getInstance()
            ->getRtc()  //
            ->setRemoteDescription(stdstring(peerId.toString()), cav);
}

void IMCall::doJingleMessage(const IMPeerId& peerId, const gloox::Jingle::JingleMessage* jm) {
    qDebug() << __func__ << "peerId:" << peerId.toString() << "sId:" << qstring(jm->id())
             << "action:" << Jingle::ActionValues[jm->action()];

    auto friendId = peerId.toFriendId();
    qDebug() << "friendId:" << friendId;

    auto sId = qstring(jm->id());

    switch (jm->action()) {
        case Jingle::JingleMessage::reject: {
            /**
             * 对方拒绝
             */
            //      mPeerRequestMedias.clear();
            auto ms = jm->medias();
            emit receiveCallStateRejected(peerId, sId, ms.size() > 1);
            //      emit receiveFriendHangup(friendId, 0);
            break;
        }
        case Jingle::JingleMessage::propose: {
            // 被对方发起呼叫
            qDebug() << "On call from:" << peerId.toString();

            // 获取呼叫类型
            bool audio = false;
            bool video = false;
            for (auto& m : jm->medias()) {
                if (m == Jingle::audio) {
                    audio = true;
                } else if (m == Jingle::video) {
                    video = true;
                }
            }

            emit receiveCallRequest(peerId, sId, audio, video);
            //      emit receiveCallAcceptByOther(sId, peerId);
            break;
        }
        case Jingle::JingleMessage::retract: {
            /**
             * 撤回(需要判断是对方还是自己其它终端)
             */
            emit receiveCallRetract(friendId, 0);
            break;
        }
        case Jingle::JingleMessage::accept: {
            // 自己其它终端接受，挂断自己
            if (peerId != _im->getSelfPeerId()) {
                emit receiveFriendHangup(friendId, 0);
            } else {
                // 自己终端接受，不处理
                //            OkRTCManager::getInstance()->getRtc()->CreateAnswer(peerId.toString());
            }
            break;
        }
        case Jingle::JingleMessage::proceed: {
            // 对方接受
            auto removed = m_sidVideo.remove(sId);  // 确定发起的是否是视频？
            emit receiveCallStateAccepted(peerId, sId, removed == 1);
            break;
        }
        case Jingle::JingleMessage::finish:
            break;
    }
}

void IMCall::proposeJingleMessage(const QString& friendId, const QString& callId, bool video) {
    qDebug() << __func__ << "friend:" << friendId << callId;

    StanzaExtensionList exts;
    auto jm = new JingleMessage(JingleMessage::propose, stdstring(callId));
    jm->addMedia(Jingle::Media::audio);
    if (video) {
        jm->addMedia(Jingle::Media::video);
        m_sidVideo.insert(callId, true);
    }
    exts.push_back(jm);

    auto jid = JID{stdstring(friendId)};
    Message m(Message::Chat, jid, {}, {});
    for (auto ext : exts) m.addExtension(ext);

    _im->getClient()->send(m);
}

void IMCall::rejectJingleMessage(const QString& peerId, const QString& callId) {
    qDebug() << __func__ << "friend:" << peerId << callId;

    StanzaExtensionList exts;
    auto reject = new Jingle::JingleMessage(Jingle::JingleMessage::reject, stdstring(callId));
    exts.push_back(reject);

    auto jid = JID{stdstring(peerId)};
    Message m(Message::Chat, jid, {}, {});
    for (auto ext : exts) m.addExtension(ext);

    _im->getClient()->send(m);
}

void IMCall::retractJingleMessage(const QString& friendId, const QString& callId) {
    qDebug() << __func__ << "friend:" << friendId << callId;

    auto* jm = new Jingle::JingleMessage(Jingle::JingleMessage::retract, stdstring(callId));

    auto jid = JID{stdstring(friendId)};
    Message m(Message::Chat, jid, {}, {});
    m.addExtension(jm);

    _im->getClient()->send(m);
}

void IMCall::acceptJingleMessage(const IMPeerId& peerId, const QString& callId, bool video) {
    qDebug() << __func__ << "friend:" << peerId.toFriendId() << callId;

    auto proceed = new Jingle::JingleMessage(Jingle::JingleMessage::proceed, stdstring(callId));
    Message proceedMsg(gloox::Message::Chat, JID(stdstring(peerId.toString())));
    proceedMsg.addExtension(proceed);
    _im->getClient()->send(proceedMsg);
    qDebug() << "Sent proceed=>" << peerId.toString();

    // 发送给自己其它终端
    auto accept = new Jingle::JingleMessage(Jingle::JingleMessage::accept, stdstring(callId));

    auto self = _im->self().bareJID();

    Message msg(gloox::Message::Chat, self);
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

IMJingleSession* IMCall::cacheSessionInfo(const IMContactId& from,
                                          const IMPeerId& to,
                                          const QString& sId,
                                          lib::ortc::JingleCallType callType) {
    qDebug() << __func__ << "target:" << to.toString();

    auto session = _sessionManager  //
                           ->createSession(JID(stdstring(to.toString())), this, stdstring(sId));

    m_friendSessionMap.insert(to, sId);

    auto ws = new IMJingleSession(sId, session, from, to, ortc::JingleCallType::av);
    m_sessionMap.insert(sId, ws);

    return ws;
}

void IMCall::clearSessionInfo(Jingle::Session* session) {
    auto sId = qstring(session->sid());

    qDebug() << __func__ << sId;

    auto& responder = session->remote();
    m_friendSessionMap.remove(IMPeerId(responder));
    m_sessionMap.remove(sId);

    _sessionManager->discardSession(session);
}

IMJingleSession* IMCall::createSession(const IMContactId& self,
                                       const IMPeerId& peer,
                                       const QString& sId,
                                       ortc::JingleCallType ct) {
    return cacheSessionInfo(self, peer, sId, ct);
}

void IMCall::handleJingleMessage(const IMPeerId& peerId, const Jingle::JingleMessage* jm) {
    doJingleMessage(peerId, jm);
}

void IMCall::doSessionInfo(const Session::Jingle* jingle, const IMPeerId& friendId) {
    qDebug() << "jingle:%1 peerId:%2"   //
             << qstring(jingle->sid())  //
             << friendId.toString();
}

void IMCall::doContentAdd(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doContentRemove(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doContentModify(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doContentAccept(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doContentReject(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doTransportInfo(const Session::Jingle* jingle, const IMPeerId& peerId) {
    auto sid = qstring(jingle->sid());
    qDebug() << __func__ << "sId:" << sid << "peerId:" << peerId.toString();

    auto s = findSession(sid);
    if (!s) {
        qWarning() << ("Session is no existing.");
        return;
    }

    ortc::OJingleContentAv content;
    parse(jingle->plugins(), content);

    for (auto& it : content.contents) {
        ortc::OkRTCManager::getInstance()
                ->getRtc()  //
                ->setTransportInfo(stdstring(peerId.toString()), jingle->sid(), it.iceUdp);
    }
}

void IMCall::doTransportAccept(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doTransportReject(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doTransportReplace(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doSecurityInfo(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doDescriptionInfo(const Session::Jingle*, const IMPeerId&) {}

void IMCall::doInvalidAction(const Session::Jingle*, const IMPeerId&) {}

void IMCall::sessionOnInitiate(const QString& sId,
                               Jingle::Session* session,
                               const Jingle::Session::Jingle* jingle,
                               const IMPeerId& peerId) {
    ortc::OJingleContentAv cav;
    parse(jingle->plugins(), cav);
    cav.sdpType = lib::ortc::JingleSdpType::Offer;
    ortc::OkRTCManager::getInstance()->getRtc()->CreateAnswer(stdstring(peerId.toString()), cav);
}

void IMCall::sessionOnTerminate(const QString& sId, const IMPeerId& peerId) {
    // rtc
    qDebug() << __func__;
    auto s = findSession(sId);
    if (s) {
        s->doTerminate();
    }
}
void IMCall::toPlugins(const ortc::OJingleContentAv& oContext, gloox::Jingle::PluginList& plugins) {
    //<group>
    auto contents = oContext.contents;
    gloox::Jingle::Group::ContentList contentList;
    for (auto& content : contents) {
        auto name = content.name;
        auto desc = content.rtp;

        contentList.push_back(Jingle::Group::Content{name});

        // description
        Jingle::PluginList rtpPlugins;

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

        auto* pContent = new Jingle::Content(name, rtpPlugins, Jingle::Content::CInitiator);
        plugins.emplace_back(pContent);
    }

    auto group = new Jingle::Group("BUNDLE", contentList);
    plugins.push_back(group);
}

void IMCall::parse(const PluginList& plugins, ortc::OJingleContentAv& oContextAv) {
    oContextAv.callType = ortc::JingleCallType::av;
    int mid = 0;
    for (const auto p : plugins) {
        Jingle::JinglePluginType pt = p->pluginType();
        switch (pt) {
            case JinglePluginType::PluginContent: {
                ortc::OSdp oContent;

                const auto* content = static_cast<const Content*>(p);
                oContent.name = content->name();

                const auto* rtp = content->findPlugin<RTP>(PluginRTP);
                if (rtp) {
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

                const auto* udp = content->findPlugin<ICEUDP>(PluginICEUDP);
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
            default:
                break;
            case PluginNone:
                break;
            case PluginFileTransfer:
                break;
            case PluginICEUDP:
                break;
            case PluginReason:
                break;
            case PluginUser:
                break;
            case PluginGroup:
                break;
            case PluginRTP:
                break;
            case PluginIBB:
                break;
        }
        mid++;
    }
}

}  // namespace lib::messenger
