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
#include <vector>

#include <capabilities.h>
#include <extdisco.h>
#include <jinglegroup.h>
#include <jingleibb.h>
#include <jinglejsonmessage.h>
#include <jinglesession.h>

#include "IM.h"

namespace lib::messenger {

void setSsrcGroup(ortc::SsrcGroup& group, const QJsonValueRef& item) {
    int i = 0;
    for (const auto& ssrc : item.toArray()) {
        auto x = ssrc.toVariant().toString();
        if (i == 0) {
            // 第一个为ssrc group类型
            if (x == "f")
                group.semantics = "FID";
            else if (x == "s") {
                group.semantics = "SIM";
            } else
                group.semantics = "FEC";
        } else {
            group.ssrcs.emplace_back(x.toStdString());
        }
        i++;
    }
}

auto txParameter = ranges::views::transform([](const gloox::Jingle::RTP::Parameter& op) {
    return ortc::Parameter{.name = op.name, .value = op.value};
});

IMJingle::IMJingle(IM* im, QObject* parent) : QObject(parent), im(im), currentSession(nullptr) {
    qDebug() << __func__ << "Creating";

    qRegisterMetaType<std::string>("std::string");
    connect(im, &IM::started, this, &IMJingle::onImStarted);
    qDebug() << __func__ << ("Created");
}

IMJingle::~IMJingle() {
    auto client = im->getClient();
    client->removeMessageHandler(this);
    qDebug() << __func__;
}

void IMJingle::onImStarted() {
    auto client = im->getClient();
    assert(client);

    client->registerMessageHandler(this);
    client->registerStanzaExtension(new gloox::Jingle::JingleMessage());

    auto disco = client->disco();
    // jingle
    disco->addFeature(gloox::XMLNS_JINGLE);
    disco->addFeature(gloox::XMLNS_JINGLE_MESSAGE);
    disco->addFeature(gloox::XMLNS_JINGLE_ERRORS);
    disco->addFeature(gloox::XMLNS_JIT_MEET);
}

void IMJingle::handleMessageSession(gloox::MessageSession* session) {
    //  session->registerMessageHandler(this);
}

void IMJingle::handleMessage(const gloox::Message& msg, gloox::MessageSession* session) {
    qDebug() << __func__;
    /**
     * 处理jingle-message消息
     * https://xmpp.org/extensions/xep-0353.html
     */
    auto jm = msg.findExtension<gloox::Jingle::JingleMessage>(gloox::ExtJingleMessage);
    if (jm) {
        handleJingleMessage(IMPeerId(msg.from().full()), jm);
    }
}

bool IMJingle::handleIq(const gloox::IQ& iq) {
    return true;
}

void IMJingle::handleIqID(const gloox::IQ& iq, int context) {}

ortc::Source IMJingle::ParseSource(const gloox::Jingle::RTP::Source& h) {
    return ortc::Source{.ssrc = h.ssrc,
                        .name = h.name,
                        .videoType = h.videoType,
                        .cname = h.cname,
                        .msid = h.msid};
}

gloox::Jingle::RTP::Source IMJingle::ToSource(const ortc::Source& s) {
    return gloox::Jingle::RTP::Source{.ssrc = s.ssrc,
                                      .name = s.name,
                                      .videoType = s.videoType,
                                      .cname = s.cname,
                                      .msid = s.msid};
}

ortc::Candidate IMJingle::ParseCandidate(gloox::Jingle::ICEUDP::Candidate& c) {
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
}

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

    ortp.media = static_cast<ortc::Media>(rtp->media());
    ortp.payloadTypes = payloadTypes;
    ortp.hdrExts = hs;
    ortp.sources = ranges::views::all(rtp->sources()) |
                   ranges::views::transform([=, this](const gloox::Jingle::RTP::Source& h) {
                       return ParseSource(h);
                   }) |
                   ranges::to<ortc::Sources>;
    ;
    ortp.ssrcGroup = ortc::SsrcGroup{.semantics = rtp->ssrcGroup().semantics,
                                     .ssrcs = rtp->ssrcGroup().ssrcs};
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
                          ranges::views::transform([this](gloox::Jingle::ICEUDP::Candidate& c) {
                              return ParseCandidate(c);
                          }) |
                          ranges::to<ortc::CandidateList>};
}

void IMJingle::ParseAV(const gloox::Jingle::Session::Jingle* jingle,
                       ortc::OJingleContentMap& contentAv) {
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

                auto fileTransfer = content->findPlugin<gloox::Jingle::FileTransfer>(
                        gloox::Jingle::PluginFileTransfer);
                if (fileTransfer) {
                    oSdp.file = ParseFile(fileTransfer);
                }

                auto ibb = content->findPlugin<gloox::Jingle::IBB>(gloox::Jingle::PluginIBB);
                if (ibb) {
                    oSdp.ibb = ParseFileIBB(ibb);
                }

                break;
            }
            case gloox::Jingle::PluginJsonMessage: {
                auto jm = dynamic_cast<const gloox::Jingle::JsonMessage*>(p);
                ParseOMeetSSRCBundle(jm->json(), contentAv.getSsrcBundle());
                break;
            }
            default:
                qWarning() << "Unsupported content:" << p;
                break;
        }
    }
}

ortc::Source parseMeetSource(const QJsonObject& o) {
    return ortc::Source{
            .ssrc = o.value("s").toVariant().toString().toStdString(),
            .videoType = o.value("v").toVariant().toString() == "d" ? "desktop" : "camera",
            .cname = o.value("n").toVariant().toString().toStdString(),
            .msid = o.value("m").toVariant().toString().toStdString(),
    };
};

QJsonObject toMeetSource(const ortc::Source& e) {
    QJsonObject jv;
    jv.insert("s", QJsonValue(qstring(e.ssrc)));
    jv.insert("n", QJsonValue(qstring(e.cname)));
    jv.insert("m", QJsonValue(qstring(e.msid)));
    if (!e.videoType.empty()) {
        jv.insert("v", QJsonValue(qstring(e.videoType)));
    }
    return jv;
};

void IMJingle::ParseOMeetSSRCBundle(const std::string& json,
                                    std::map<std::string, ortc::OMeetSSRCBundle>& map) {
    if (json.empty()) {
        qWarning() << "empty!";
        return;
    }

    QJsonDocument doc = ok::base::Jsons::toJSON(QByteArray::fromStdString(json));
    if (!doc.isObject()) {
        qWarning() << __func__ << "JSON is not an object";
        return;
    }

    QJsonObject jsonObj = doc.object();
    if (!jsonObj.contains("sources")) {
        qWarning() << __func__ << "JSON object does not contain 'sources' key";
        return;
    }

    QJsonObject sourcesObj = jsonObj["sources"].toObject();
    for (auto it = sourcesObj.begin(); it != sourcesObj.end(); ++it) {
        QString sourceKey = it.key();
        QJsonArray sourceArray = it.value().toArray();
        qDebug() << __func__ << "Parse participant:" << sourceKey;

        ortc::OMeetSSRCBundle bundle;
        if (!sourceArray.empty()) {
            // index 0 as video source
            for (const auto& item : sourceArray.at(0).toArray()) {
                bundle.videoSources.emplace_back(parseMeetSource(item.toObject()));
            }
        }
        if (sourceArray.size() > 1) {
            // index 1 as video source group
            for (const auto& item : sourceArray.at(1).toArray()) {
                setSsrcGroup(bundle.videoSsrcGroups, item);
            }
        }
        if (sourceArray.size() > 2) {
            // index 2 as audio source
            for (const auto& item : sourceArray.at(2).toArray()) {
                bundle.audioSources.emplace_back(parseMeetSource(item.toObject()));
            }
        }
        if (sourceArray.size() > 3) {
            for (const auto& item : sourceArray.at(3).toArray()) {
                setSsrcGroup(bundle.audioSsrcGroups, item);
            }
        }
        map.emplace(stdstring(sourceKey), bundle);
    }
}

void IMJingle::FormatOMeetSSRCBundle(const std::map<std::string, ortc::OMeetSSRCBundle>& ssrcBundle,
                                     std::string& json) {
    QJsonDocument doc;
    QJsonObject root;
    QJsonObject s;
    for (const auto& item : ssrcBundle) {
        QJsonArray array;

        // 0 video ssrc list;
        QJsonArray vs;
        for (const auto& e : item.second.videoSources) {
            vs.push_back(toMeetSource(e));
        }
        array.push_back(vs);

        // 2 video ssrc group
        QJsonArray vg;
        auto& vgroup = item.second.videoSsrcGroups;
        if (!vgroup.ssrcs.empty()) {
            vg.push_back(qstring(vgroup.semantics));
            std::for_each(vgroup.ssrcs.begin(), vgroup.ssrcs.end(),
                          [&](auto& s) { vg.push_back(QJsonValue(qstring(s))); });
        }
        array.push_back(vg);

        // 3 audio ssrc list
        QJsonArray as;
        for (const auto& e : item.second.audioSources) {
            vs.push_back(toMeetSource(e));
        }
        array.push_back(as);

        // 4 audio ssrc group (ignore it)
        s.insert(qstring(item.first), array);
    }
    root.insert("sources", s);
    doc.setObject(root);
    auto str = ok::base::Jsons::toString(doc);
    json.assign(str.toStdString());
}

gloox::Jingle::RTP::Feedback IMJingle::toFeedback(const ortc::Feedback& p) {
    return gloox::Jingle::RTP::Feedback{.type = p.type, .subtype = p.subtype};
}

gloox::Jingle::RTP::Parameter IMJingle::toParameter(const ortc::Parameter& p) {
    return gloox::Jingle::RTP::Parameter{.name = p.name, .value = p.value};
}

gloox::Jingle::RTP::PayloadType IMJingle::ToPayloadType(const ortc::PayloadType& x) {
    return gloox::Jingle::RTP::PayloadType{
            .id = x.id,
            .name = x.name,
            .clockrate = x.clockrate,
            .bitrate = x.bitrate,
            .channels = static_cast<int>(x.channels),
            .parameters = ranges::views::all(x.parameters) |
                          ranges::views::transform([&](const auto& e) { return toParameter(e); }) |
                          ranges::to<std::list<gloox::Jingle::RTP::Parameter>>(),
            .feedbacks = ranges::views::all(x.feedbacks) |
                         ranges::views::transform([&](const auto& e) { return toFeedback(e); }) |
                         ranges::to<std::list<gloox::Jingle::RTP::Feedback>>()};
}

std::unique_ptr<gloox::Jingle::RTP> IMJingle::ToRTP(const ortc::ORTP& desc) {
    // rtcp
    auto pts = ranges::views::all(desc.payloadTypes) |
               ranges::views::transform([&](auto& e) { return ToPayloadType(e); }) |
               ranges::to<gloox::Jingle::RTP::PayloadTypes>();

    auto rtp = new gloox::Jingle::RTP(static_cast<gloox::Jingle::Media>(desc.media), pts);
    rtp->setRtcpMux(desc.rtcpMux);

    // payload-type
    rtp->setPayloadTypes(pts);

    // rtp-hdrExt
    gloox::Jingle::RTP::HdrExts exts;
    for (auto& e : desc.hdrExts) {
        exts.push_back({e.id, e.uri});
    }
    rtp->setHdrExts(exts);

    // source
    if (!desc.sources.empty()) {
        auto ss = ranges::views::all(desc.sources) |
                  ranges::views::transform([&](const auto& s) { return ToSource(s); }) |
                  ranges::to_vector;
        rtp->setSources(ss);
    }

    // ssrc-group
    if (!desc.ssrcGroup.ssrcs.empty()) {
        gloox::Jingle::RTP::SsrcGroup sg;
        sg.semantics = desc.ssrcGroup.semantics;
        for (auto& s : desc.ssrcGroup.ssrcs) {
            sg.ssrcs.push_back(s);
        }
        rtp->setSsrcGroup(sg);
    }

    return std::unique_ptr<gloox::Jingle::RTP>(rtp);
}

std::unique_ptr<gloox::Jingle::ICEUDP> IMJingle::ToICEUDP(
        const ortc::OIceUdp& oIceUdp, const gloox::Jingle::ICEUDP::CandidateList& cl) {
    auto ice = new gloox::Jingle::ICEUDP(oIceUdp.pwd, oIceUdp.ufrag, cl);
    ice->setDtls({.hash = oIceUdp.dtls.hash,
                  .setup = oIceUdp.dtls.setup,
                  .fingerprint = oIceUdp.dtls.fingerprint});
    return std::unique_ptr<gloox::Jingle::ICEUDP>(ice);
}

gloox::Jingle::ICEUDP::Candidate IMJingle::ToCandidate(const ortc::Candidate& c) {
    return gloox::Jingle::ICEUDP::Candidate{c.component,
                                            c.foundation,
                                            c.generation,
                                            c.id,
                                            c.ip,
                                            c.network,
                                            c.port,
                                            c.priority,
                                            c.protocol,
                                            c.tcptype,
                                            c.rel_addr,
                                            c.rel_port,
                                            static_cast<gloox::Jingle::ICEUDP::Type>(c.type)};
}

std::unique_ptr<gloox::Jingle::Content> IMJingle::ToContent(const std::string& mid,
                                                            const ortc::OSdp& sdp,
                                                            gloox::Jingle::Content::Creator creator,
                                                            bool candidate) {
    // description
    gloox::Jingle::PluginList rtpPlugins;
    // rtp
    auto rtp = ToRTP(sdp.rtp);
    rtpPlugins.emplace_back(rtp.release());

    // transport
    if (candidate) {
        auto cl = (ranges::views::all(sdp.iceUdp.candidates) |
                   ranges::views::transform([&](const auto& e) { return ToCandidate(e); }) |
                   ranges::to<gloox::Jingle::ICEUDP::CandidateList>());
        rtpPlugins.emplace_back(ToICEUDP(sdp.iceUdp, cl).release());
    } else {
        rtpPlugins.emplace_back(ToICEUDP(sdp.iceUdp, {}).release());
    }

    auto* pContent = new gloox::Jingle::Content(mid, rtpPlugins, creator);
    return std::unique_ptr<gloox::Jingle::Content>(pContent);
}

void IMJingle::ToPlugins(const ortc::OJingleContentMap* av, gloox::Jingle::PluginList& plugins) {
    //<group>
    auto& contents = av->getContents();
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
        for (auto& x : desc.payloadTypes) {
            gloox::Jingle::RTP::PayloadType t;
            t.id = x.id;
            t.name = x.name;
            t.clockrate = x.clockrate;
            t.bitrate = x.bitrate;
            t.channels = x.channels;

            for (auto& p : x.parameters) {
                gloox::Jingle::RTP::Parameter p0;
                p0.name = p.name;
                p0.value = p.value;
                t.parameters.push_back(p0);
            }

            for (auto& f : x.feedbacks) {
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
        for (auto& e : desc.hdrExts) {
            exts.push_back({e.id, e.uri});
        }
        rtp->setHdrExts(exts);

        // source
        if (!desc.sources.empty()) {
            auto ss = ranges::views::all(desc.sources) |
                      ranges::views::transform([&](const auto& s) { return ToSource(s); }) |
                      ranges::to_vector;
            rtp->setSources(ss);
        }

        // ssrc-group
        if (!desc.ssrcGroup.ssrcs.empty()) {
            gloox::Jingle::RTP::SsrcGroup sg;
            sg.semantics = desc.ssrcGroup.semantics;
            for (auto& s : desc.ssrcGroup.ssrcs) {
                sg.ssrcs.push_back(s);
            }
            rtp->setSsrcGroup(sg);
        }

        // rtp
        rtpPlugins.emplace_back(rtp);

        // transport
        lib::ortc::OIceUdp oIceUdp = content.iceUdp;

        gloox::Jingle::ICEUDP::CandidateList cl;

        for (auto& c : oIceUdp.candidates) {
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

void IMJingle::ParseCandidates(gloox::Jingle::ICEUDP::CandidateList& src, ortc::CandidateList& to) {
    for (auto& c : src) {
        to.push_back(ParseCandidate(c));
    }
}
ortc::OFile IMJingle::ParseFile(const gloox::Jingle::FileTransfer* file) {
    for (auto& f : file->files()) {
        ortc::OFile file0 = {//                .id = id,
                             //                             .sId = sId,
                             .date = f.date, .hash = f.hash,   .hash_algo = f.hash_algo,
                             .size = f.size, .range = f.range, .offset = f.offset};
        file0.name = f.name;
        return file0;
    };
    return {};
}

ortc::OFileIBB IMJingle::ParseFileIBB(const gloox::Jingle::IBB* ibb) {
    return ortc::OFileIBB{.sId = ibb->sid(), .blockSize = ibb->blockSize()};
}

}  // namespace lib::messenger
