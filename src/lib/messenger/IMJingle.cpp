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

#include "IMJingle.h"

#include <QUuid>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>

#include <capabilities.h>
#include <extdisco.h>
#include <jinglegroup.h>
#include <jinglejsonmessage.h>
#include <jinglesession.h>

#include "IM.h"
#include "base/logs.h"

namespace lib::messenger {

using namespace gloox;
using namespace Jingle;
using namespace lib::ortc;

IMJingle::IMJingle(IM* im, QObject* parent) : QObject(parent), im(im), currentSession(nullptr) {
    qDebug() << __func__ << "Creating";

    qRegisterMetaType<std::string>("std::string");
    connect(im, &IM::started, this, &IMJingle::onImStarted);
    qDebug() << __func__ << ("Created");
}

IMJingle::~IMJingle() {
    auto client = im->getClient();
    client->removeMessageHandler(this);
    qDebug() << __func__ << "Destroyed";
}

void IMJingle::onImStarted() {
    auto client = im->getClient();
    assert(client);

    client->registerMessageHandler(this);
    client->registerStanzaExtension(new Jingle::JingleMessage());

    auto disco = client->disco();
    // jingle
    disco->addFeature(XMLNS_JINGLE);
    disco->addFeature(XMLNS_JINGLE_MESSAGE);
    disco->addFeature(XMLNS_JINGLE_ERRORS);
    disco->addFeature(XMLNS_JIT_MEET);
}

void IMJingle::handleMessageSession(MessageSession* session) {
    //  session->registerMessageHandler(this);
}

void IMJingle::handleMessage(const Message& msg, MessageSession* session) {
    qDebug() << __func__ << "...";

    /**
     * 处理jingle-message消息
     * https://xmpp.org/extensions/xep-0353.html
     */
    auto jm = msg.findExtension<Jingle::JingleMessage>(ExtJingleMessage);
    if (jm) {
        handleJingleMessage(IMPeerId(msg.from().full()), jm);
    }
}

bool IMJingle::handleIq(const IQ& iq) {
    return true;
}

void IMJingle::handleIqID(const IQ& iq, int context) {}

auto txParameter = ranges::views::transform([](const gloox::Jingle::RTP::Parameter& op) {
    return ortc::Parameter{.name = op.name, .value = op.value};
});

bool IMJingle::ParseRTP(const gloox::Jingle::RTP* rtp, ortc::ORTP& ortp) {
    if (!rtp) {
        return false;
    }

    auto pts = rtp->payloadTypes();
    auto payloadTypes =
            ranges::views::all(pts) |
            ranges::views::transform([=](gloox::Jingle::RTP::PayloadType& p) {
                return ortc::PayloadType{
                        .id = p.id,
                        .name = p.name,
                        .clockrate = p.clockrate,
                        .bitrate = p.bitrate,
                        .channels = static_cast<size_t>(p.channels),
                        .parameters = ranges::views::all(p.parameters) | txParameter |
                                      ranges::to<ortc::Parameters>,
                        .feedbacks = ranges::views::all(p.feedbacks) |
                                     ranges::views::transform(
                                             [=](const gloox::Jingle::RTP::Feedback& f) {
                                                 return ortc::Feedback{.type = f.type,
                                                                       .subtype = f.subtype};
                                             }) |
                                     ranges::to<ortc::Feedbacks>};
            }) |
            ranges::to<ortc::PayloadTypes>;

    auto hes = rtp->hdrExts();
    auto hs = ranges::views::all(hes) |
              ranges::views::transform([=](gloox::Jingle::RTP::HdrExt& h) {
                  return ortc::HdrExt{.id = h.id, .uri = h.uri};
              }) |
              ranges::to<ortc::HdrExts>;

    auto oss = rtp->sources();
    auto ss = ranges::views::all(oss) |
              ranges::views::transform([=](gloox::Jingle::RTP::Source& h) {
                  auto ps = h.parameters;
                  return ortc::Source{.ssrc = h.ssrc,
                                      .parameters = ranges::views::all(ps) | txParameter |
                                                    ranges::to<ortc::Parameters>};
              }) |
              ranges::to<ortc::Sources>;
    auto sg = ortc::SsrcGroup{.semantics = rtp->ssrcGroup().semantics,
                              .ssrcs = rtp->ssrcGroup().ssrcs};

    ortp.media = static_cast<ortc::Media>(rtp->media());
    ortp.payloadTypes = payloadTypes;
    ortp.hdrExts = hs;
    ortp.sources = ss;
    ortp.ssrcGroup = sg;
    ortp.rtcpMux = rtp->rtcpMux();
    return true;
}

ortc::OIceUdp IMJingle::ParseIce(const std::string& mid, const gloox::Jingle::ICEUDP* udp) {
    if (!udp) return {};

    auto cs = udp->candidates();
    return ortc::OIceUdp{
            .mid = mid,                                     //
            .ufrag = udp->ufrag(),                          //
            .pwd = udp->pwd(),                              //
            .dtls = ortc::Dtls{.hash = udp->dtls().hash,    //
                               .setup = udp->dtls().setup,  //
                               .fingerprint = udp->dtls().fingerprint},
            .sctp =
                    ortc::Sctp{
                            .protocol = udp->sctp().protocol,
                            .port = udp->sctp().port,
                            .streams = udp->sctp().streams,
                    },
            .candidates = ranges::views::all(cs) |
                          ranges::views::transform([=](gloox::Jingle::ICEUDP::Candidate& c) {
                              return ortc::Candidate{.component = c.component,
                                                     .foundation = c.foundation,
                                                     .generation = c.generation,
                                                     .id = c.id,
                                                     .ip = c.ip,
                                                     .network = c.network,
                                                     .port = c.port,
                                                     .priority = static_cast<uint32_t>(c.priority),
                                                     .protocol = c.protocol,
                                                     .tcptype = c.tcptype,
                                                     .rel_addr = c.rel_addr,
                                                     .rel_port = c.rel_port,
                                                     .type = static_cast<ortc::Type>(c.type)};
                          }) |
                          ranges::to<ortc::CandidateList>};
}

void IMJingle::ParseAV(const gloox::Jingle::Session::Jingle* jingle, OJingleContentAv& contentAv) {
    contentAv.sessionId = jingle->sid();
    for (const auto p : jingle->plugins()) {
        auto pt = p->pluginType();
        switch (pt) {
            case gloox::Jingle::PluginContent: {
                auto content = static_cast<const gloox::Jingle::Content*>(p);
                ortc::OSdp& oSdp = contentAv.load(content->name());

                auto rtp = content->findPlugin<gloox::Jingle::RTP>(gloox::Jingle::PluginRTP);
                if (rtp) {
                    ortc::ORTP oRtp;
                    auto y = ParseRTP(rtp, oRtp);
                    if (y) {
                        oSdp.rtp = oRtp;
                    }
                }

                auto udp = content->findPlugin<gloox::Jingle::ICEUDP>(gloox::Jingle::PluginICEUDP);
                if (udp) {
                    oSdp.iceUdp = ParseIce(content->name(), udp);
                }
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
            case gloox::Jingle::PluginJsonMessage: {
                auto jm = static_cast<const gloox::Jingle::JsonMessage*>(p);
                if (jm) {
                    auto json = jm->json();
                    /**
                     *
                     */
                    qDebug() << json.c_str();
                }
                break;
            }
            default:
                break;
        }
    }
}

void IMJingle::ToPlugins(const OJingleContentAv& av, PluginList& plugins) {
    //<group>
    auto& contents = av.getContents();
    gloox::Jingle::Group::ContentList contentList;
    for (auto& kv : contents) {
        auto& content = kv.second;
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
            cl.push_front(gloox::Jingle::ICEUDP::Candidate{
                    c.component, c.foundation, c.generation, c.id, c.ip, c.network, c.port,
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

}  // namespace lib::messenger
