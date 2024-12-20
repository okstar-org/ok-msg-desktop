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
#include "webrtc.h"
#include "ok_conductor.h"

#include <memory>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>
#include <string>
#include <utility>

#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/create_peerconnection_factory.h>
#include <api/peer_connection_interface.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <media/base/codec.h>
#include <modules/video_capture/video_capture_factory.h>
#include <pc/video_track_source.h>
#include <rtc_base/logging.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/string_encode.h>
#include <rtc_base/thread.h>

namespace lib::ortc {

cricket::TransportInfo toTransportInfo(const std::string& name, const OIceUdp& iceUdp) {
    cricket::TransportInfo ti;
    ti.content_name = name;
    ti.description.ice_ufrag = iceUdp.ufrag;
    ti.description.ice_pwd = iceUdp.pwd;
    ti.description.identity_fingerprint.reset(cricket::TransportDescription::CopyFingerprint(
            rtc::SSLFingerprint::CreateFromRfc4572(iceUdp.dtls.hash, iceUdp.dtls.fingerprint)));

    if (iceUdp.dtls.setup == "actpass") {
        ti.description.connection_role = cricket::CONNECTIONROLE_ACTPASS;
    } else if (iceUdp.dtls.setup == "active") {
        ti.description.connection_role = cricket::CONNECTIONROLE_ACTIVE;
    } else if (iceUdp.dtls.setup == "passive") {
        ti.description.connection_role = cricket::CONNECTIONROLE_PASSIVE;
    } else if (iceUdp.dtls.setup == "holdconn") {
        ti.description.connection_role = cricket::CONNECTIONROLE_HOLDCONN;
    } else {
        ti.description.connection_role = cricket::CONNECTIONROLE_NONE;
    }
    return ti;
}

Dtls getDtls(const cricket::TransportInfo& info) {
    Dtls dtls;
    if (info.description.identity_fingerprint) {
        dtls.hash = info.description.identity_fingerprint->algorithm;
        dtls.fingerprint = info.description.identity_fingerprint->GetRfc4572Fingerprint();
    }
    switch (info.description.connection_role) {
        case cricket::CONNECTIONROLE_ACTIVE:
            dtls.setup = cricket::CONNECTIONROLE_ACTIVE_STR;
            break;
        case cricket::CONNECTIONROLE_ACTPASS:
            dtls.setup = cricket::CONNECTIONROLE_ACTPASS_STR;
            break;
        case cricket::CONNECTIONROLE_HOLDCONN:
            dtls.setup = cricket::CONNECTIONROLE_HOLDCONN_STR;
            break;
        case cricket::CONNECTIONROLE_PASSIVE:
            dtls.setup = cricket::CONNECTIONROLE_PASSIVE_STR;
            break;
        case cricket::CONNECTIONROLE_NONE:
            break;
    }

    return dtls;
}

Dtls fromDtls(const webrtc::SessionDescriptionInterface* sdp, const std::string& mid) {
    auto transportInfos = sdp->description()->transport_infos();
    auto s = ranges::views::all(transportInfos) |
             ranges::views::filter(
                     [=](cricket::TransportInfo& info) { return info.content_name == mid; }) |
             ranges::views::transform([=](cricket::TransportInfo& info) { return getDtls(info); }) |
             ranges::to_vector;
    return s.front();
}

ortc::Candidate fromCandidate(const cricket::Candidate& cand) {
    auto c = Candidate{
            .component = cand.component(),
            .foundation = cand.foundation(),
            .generation = cand.generation(),
            .id = cand.id(),
            .ip = cand.address().ipaddr().ToString(),
            .network = cand.network_id(),
            .port = cand.address().port(),
            .priority = cand.priority(),
            .protocol = cand.protocol(),
            .tcptype = cand.tcptype(),

    };
    if (cand.type() == cricket::LOCAL_PORT_TYPE) {
        c.type = Type::Host;
    } else if (cand.type() == cricket::STUN_PORT_TYPE) {
        c.type = Type::ServerReflexive;
    } else if (cand.type() == cricket::PRFLX_PORT_TYPE) {
        c.type = Type::PeerReflexive;
    } else if (cand.type() == cricket::RELAY_PORT_TYPE) {
        c.type = Type::Relayed;
    };
    if (c.type != Type::Host && 0 < cand.related_address().port()) {
        c.rel_addr = cand.related_address().ipaddr().ToString();
        c.rel_port = cand.related_address().port();
    }
    return c;
}

cricket::Candidate toCandidate(const Candidate& item, const OIceUdp& iceUdp) {
    // "host" / "srflx" / "prflx" / "relay" / token @
    // http://tools.ietf.org/html/rfc5245#section-15.1
    //            if (item.type == Type::Host) {
    //                continue;
    //            }
    std::string type;
    switch (item.type) {
        case Type::Host:
            type = cricket::LOCAL_PORT_TYPE;
            break;
        case Type::PeerReflexive:
            type = cricket::PRFLX_PORT_TYPE;
            break;
        case Type::Relayed:
            type = cricket::RELAY_PORT_TYPE;
            break;
        case Type::ServerReflexive:
            type = cricket::STUN_PORT_TYPE;
            break;
    }
    assert(!type.empty());
    cricket::Candidate candidate(item.component,
                                 item.protocol,
                                 rtc::SocketAddress{item.ip, (int)item.port},
                                 item.priority,
                                 iceUdp.ufrag,
                                 iceUdp.pwd,
                                 type,
                                 item.generation,
                                 item.foundation,
                                 item.network);
    return candidate;
}

std::map<std::string, ortc::OIceUdp> fromIce(const webrtc::SessionDescriptionInterface* sdp) {
    std::list<const webrtc::IceCandidateInterface*> iceList;
    for (int i = 0; i < sdp->number_of_mediasections(); i++) {
        auto col = sdp->candidates(i);
        if (col) {
            for (int j = 0; j < col->count(); ++j) {
                iceList.push_back(col->at(j));
            }
        }
    }

    std::map<std::string, ortc::OIceUdp> iceUdps;
    for (auto item : iceList) {
        auto mid = item->sdp_mid();
        const std::map<std::string, OIceUdp>::iterator& find = iceUdps.find(mid);
        if (find == iceUdps.end()) {
            auto ti = sdp->description()->GetTransportInfoByName(mid);

            auto u = OIceUdp{
                    .mid = item->sdp_mid(),
                    .ufrag = ti->description.ice_ufrag,
                    .pwd = ti->description.ice_pwd,
                    .dtls = fromDtls(sdp, mid),
            };

            u.candidates.push_back(fromCandidate(item->candidate()));
            iceUdps.emplace(mid, u);
        } else {
            auto& u = find->second;
            u.candidates.push_back(fromCandidate(item->candidate()));
        }
    }

    return iceUdps;
}

void fromSdp(const webrtc::SessionDescriptionInterface* desc, OJingleContentAv& av) {
    av.sessionId = desc->session_id();
    av.sessionVersion = desc->session_version();

    // ContentGroup
    cricket::ContentGroup group(cricket::GROUP_TYPE_BUNDLE);

    int i = 0;
    auto sd = desc->description();
    for (auto rtcContent : sd->contents()) {
        i++;
        const std::string& name = rtcContent.mid();

        OSdp oSdp;
        oSdp.name = name;

        auto map = fromIce(desc);
        auto find = map.find(name);
        if (find != map.end()) oSdp.iceUdp = find->second;

        auto mediaDescription = rtcContent.media_description();
        // media type
        auto mt = mediaDescription->type();

        // rtcp_mux
        oSdp.rtp.rtcpMux = mediaDescription->rtcp_mux();
        //
        //        auto ti = sd->GetTransportInfoByName(name);
        //
        //        oContent.iceUdp.pwd = ti->description.ice_pwd;
        //        oContent.iceUdp.ufrag = ti->description.ice_ufrag;
        //
        //        // fingerprint
        //        if (ti->description.identity_fingerprint) {
        //            oContent.iceUdp.dtls.fingerprint =
        //                    ti->description.identity_fingerprint->GetRfc4572Fingerprint();
        //            oContent.iceUdp.dtls.hash = ti->description.identity_fingerprint->algorithm;
        //
        //            // connection_role
        //            std::string setup;
        //            cricket::ConnectionRoleToString(ti->description.connection_role, &setup);
        //            oContent.iceUdp.dtls.setup = setup;
        //        }

        // hdrext
        auto hdrs = mediaDescription->rtp_header_extensions();

        for (auto& hdr : hdrs) {
            HdrExt hdrExt = {hdr.id, hdr.uri};
            oSdp.rtp.hdrExts.push_back(hdrExt);
        }

        // ssrc
        for (auto& stream : mediaDescription->streams()) {
            //      "{id:5e9a64d8-b9d3-4fc7-a8eb-0ee6dec72138;  //track id
            //      ssrcs:[1679428189,751024037];
            //      ssrc_groups:{semantics:FID; ssrcs:[1679428189,751024037]};
            //      cname:dBhnE4FRSAUq1FZp;
            //      stream_ids:okedu-video-id;
            // }"
            RTC_LOG(LS_INFO) << "stream: " << (stream.ToString());

            // label
            const std::string& first_stream_id = stream.first_stream_id();

            for (auto& ssrc : stream.ssrcs) {
                RTC_LOG(LS_INFO) << "stream_id:" << first_stream_id << " label:" << stream.id
                                 << " ssrc:" << ssrc;

                Parameter cname = {"cname", stream.cname};
                Parameter label = {"label", stream.id};
                Parameter mslabel = {"mslabel", first_stream_id};
                Parameter msid = {"msid", first_stream_id + " " + stream.id};

                // msid = mslabel+ label(stream.id)
                Parameters parameters;
                parameters.emplace_back(cname);
                parameters.emplace_back(msid);
                parameters.emplace_back(mslabel);
                parameters.emplace_back(label);

                Source source = {std::to_string(ssrc), parameters};
                oSdp.rtp.sources.emplace_back(source);
            }
        }

        // ssrc-group
        if (oSdp.rtp.sources.size() >= 2) {
            oSdp.rtp.ssrcGroup.semantics = "FID";
            for (auto& ssrc : oSdp.rtp.sources) {
                oSdp.rtp.ssrcGroup.ssrcs.emplace_back(ssrc.ssrc);
            }
        }

        // codecs
        switch (mt) {
            case cricket::MediaType::MEDIA_TYPE_AUDIO: {
                oSdp.rtp.media = Media::audio;
                auto audio_desc = mediaDescription->as_audio();
                auto codecs = audio_desc->codecs();

                for (auto& codec : codecs) {
                    PayloadType type;
                    type.id = codec.id;
                    type.name = codec.name;
                    type.channels = codec.channels;
                    type.clockrate = codec.clockrate;
                    type.bitrate = codec.bitrate;

                    auto cps = codec.ToCodecParameters();
                    for (auto& it : cps.parameters) {
                        Parameter parameter;
                        if (parameter.name.empty()) continue;
                        parameter.name = it.first;
                        parameter.value = it.second;
                        type.parameters.emplace_back(parameter);
                    }

                    // rtcp-fb
                    for (auto& it : codec.feedback_params.params()) {
                        Feedback fb = {it.id(), it.param()};
                        type.feedbacks.push_back(fb);
                    }

                    oSdp.rtp.payloadTypes.emplace_back(type);
                }

                break;
            }
            case cricket::MediaType::MEDIA_TYPE_VIDEO: {
                oSdp.rtp.media = Media::video;
                auto video_desc = mediaDescription->as_video();
                for (auto& codec : video_desc->codecs()) {
                    // PayloadType
                    PayloadType type;
                    type.id = codec.id;
                    type.name = codec.name;
                    type.clockrate = codec.clockrate;

                    // PayloadType parameter
                    auto cps = codec.ToCodecParameters();
                    for (auto& it : cps.parameters) {
                        Parameter parameter;
                        parameter.name = it.first;
                        parameter.value = it.second;
                        type.parameters.emplace_back(parameter);
                    }

                    // rtcp-fb
                    for (auto& it : codec.feedback_params.params()) {
                        Feedback fb = {it.id(), it.param()};
                        type.feedbacks.push_back(fb);
                    }

                    oSdp.rtp.payloadTypes.emplace_back(type);
                }
                break;
            }
            case cricket::MediaType::MEDIA_TYPE_DATA: {
                break;
            }
            case cricket::MEDIA_TYPE_UNSUPPORTED:
                break;
        }

        av.put(name, oSdp);
    }
}

void setSsrc(const SsrcGroup& ssrcGroup,
             const std::vector<OMeetSource>& sources,
             cricket::RtpMediaContentDescription* content) {
    cricket::StreamParams streamParams;
    for (auto& src : sources) {
        streamParams.ssrcs.push_back(std::stoul(src.ssrc));
        streamParams.cname = src.name;
        streamParams.set_stream_ids({src.msid});
    };

    // ssrc-groups
    if (!ssrcGroup.ssrcs.empty()) {
        auto ssrcs = ranges::views::all(ssrcGroup.ssrcs) |
                     ranges::views::transform([](const std::string& s) {
                         return static_cast<uint32_t>(std::stoul(s));
                     }) |
                     ranges::to_vector;
        streamParams.ssrc_groups.emplace_back(cricket::SsrcGroup(ssrcGroup.semantics, ssrcs));
    }

    if (streamParams.has_ssrcs()) {
        content->AddStream(streamParams);
    }
}

void setSsrc2(  //
        const Sources& sources, const SsrcGroup& g, cricket::RtpMediaContentDescription* ptr
        //        ,const std::map<std::string, OMeetSSRCBundle>& ssrcBundleMap
) {
    //    auto ptr = std::make_unique<cricket::AudioContentDescription>();
    if (!sources.empty()) {
        cricket::StreamParams streamParams;
        for (auto& src : sources) {
            streamParams.ssrcs.push_back(std::stoul(src.ssrc));
            for (auto& p : src.parameters) {
                if (p.name == "cname") {
                    streamParams.cname = p.value;
                } else if (p.name == "label") {
                    streamParams.id = p.value;
                } else if (p.name == "mslabel") {
                    streamParams.set_stream_ids({p.value});
                }
            };
        }

        if (!g.ssrcs.empty()) {
            std::vector<uint32_t> ssrcs;
            std::transform(g.ssrcs.begin(), g.ssrcs.end(), std::back_inserter(ssrcs),
                           [](auto& s) -> uint32_t { return std::stoul(s); });
            cricket::SsrcGroup ssrcGroup(g.semantics, ssrcs);

            // ssrc-groups
            streamParams.ssrc_groups.emplace_back(ssrcGroup);
        }

        // ssrc
        ptr->AddStream(streamParams);
    }
    //    else {
    //        for (const auto& item : ssrcBundleMap) {
    //            cricket::StreamParams streamParams;
    //            for (auto& src : item.second.audioSources) {
    //                streamParams.ssrcs.push_back(std::stoul(src.ssrc));
    //                streamParams.cname = src.name;
    //                streamParams.set_stream_ids({src.msid});
    //            };
    //
    //            if (!item.second.audioSsrcGroups.ssrcs.empty()) {
    //                auto ssrcs = ranges::views::all(item.second.audioSsrcGroups.ssrcs) |
    //                             ranges::views::transform([](const std::string& s) {
    //                                 return static_cast<uint32_t>(std::stoul(s));
    //                             }) |
    //                             ranges::to_vector;
    //                // ssrc-groups
    //                streamParams.ssrc_groups.emplace_back(
    //                        cricket::SsrcGroup(item.second.audioSsrcGroups.semantics, ssrcs));
    //            }
    //            if (streamParams.has_ssrcs()) ptr->AddStream(streamParams);
    //        }
    //    }

    // rtcp-mux

    //    return std::move(ptr);
}

void setCodec(const PayloadType& pt, cricket::Codec& codec) {
    for (auto& e : pt.parameters) {
        codec.SetParam(e.name, e.value);
    }
    for (auto& e : pt.feedbacks) {
        cricket::FeedbackParam fb(e.type, e.subtype);
        codec.AddFeedbackParam(fb);
    }
}

void setRptExtensions(const HdrExts& hdrExts, cricket::RtpMediaContentDescription* ptr) {
    for (auto& hdrExt : hdrExts) {
        webrtc::RtpExtension ext(hdrExt.uri, hdrExt.id);
        ptr->AddRtpHeaderExtension(ext);
    }
}

void addSsrcBundle(const OMeetSSRCBundle& ssrcBundle, cricket::SessionDescription* d) {
    // audio
    auto audioPtr = std::make_unique<cricket::AudioContentDescription>();
    setSsrc(ssrcBundle.audioSsrcGroups, ssrcBundle.audioSources, audioPtr.get());
    audioPtr->set_rtcp_mux(true);
    d->AddContent(ssrcBundle.audioSources[0].name, cricket::MediaProtocolType::kRtp,
                  std::move(audioPtr));

    // video
    auto videoPtr = std::make_unique<cricket::VideoContentDescription>();
    setSsrc(ssrcBundle.videoSsrcGroups, ssrcBundle.videoSources, videoPtr.get());
    videoPtr->set_rtcp_mux(true);
    d->AddContent(ssrcBundle.videoSources[0].name, cricket::MediaProtocolType::kRtp,
                  std::move(videoPtr));
}

std::unique_ptr<cricket::AudioContentDescription> addAudioSsrcBundle(
        const OMeetSSRCBundle& ssrcBundle) {
    // audio
    auto audioPtr = std::make_unique<cricket::AudioContentDescription>();
    setSsrc(ssrcBundle.audioSsrcGroups, ssrcBundle.audioSources, audioPtr.get());
    return std::move(audioPtr);
}

std::unique_ptr<cricket::VideoContentDescription> addVideoSsrcBundle(
        const OMeetSSRCBundle& ssrcBundle) {
    // video
    auto videoPtr = std::make_unique<cricket::VideoContentDescription>();
    setSsrc(ssrcBundle.videoSsrcGroups, ssrcBundle.videoSources, videoPtr.get());
    return std::move(videoPtr);
}

WebRTC::WebRTC() : peer_connection_factory(nullptr) {
    _rtcConfig.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    _rtcConfig.enable_implicit_rollback = false;
    _rtcConfig.enable_ice_renomination = true;
}

WebRTC::~WebRTC() {
    for (auto it : _pcMap) {
        auto c = it.second;
        delete c;
    }

    if (isStarted()) {
        stop();
    }
}

bool WebRTC::start() {
    RTC_LOG(LS_INFO) << "Starting the WebRTC...";

    // lock
    std::lock_guard<std::recursive_mutex> lock(mutex);

    //  _logSink(std::make_unique<LogSinkImpl>())
    //    rtc::LogMessage::AddLogToStream(_logSink.get(), rtc::LS_INFO);
    rtc::LogMessage::LogToDebug(rtc::LS_INFO);
    //    rtc::LogMessage::SetLogToStderr(false);

    RTC_LOG(LS_INFO) << "InitializeSSL=>" << rtc::InitializeSSL();
    ;

    //   threads = StaticThreads::getThreads();

    RTC_LOG(LS_INFO) << "Creating network thread";
    network_thread = rtc::Thread::CreateWithSocketServer();
    RTC_LOG(LS_INFO) << "Network thread=>" << network_thread;
    network_thread->SetName("network_thread", this);
    RTC_LOG(LS_INFO) << "Network thread is started=>" << network_thread->Start();

    //    network_thread = std::unique_ptr<rtc::Thread>( threads->getNetworkThread());

    RTC_LOG(LS_INFO) << "Creating worker thread";
    worker_thread = rtc::Thread::Create();
    RTC_LOG(LS_INFO) << "Worker thread=>" << worker_thread;
    worker_thread->SetName("worker_thread", this);
    RTC_LOG(LS_INFO) << "Worker thread is started=>" << worker_thread->Start();
    //    network_thread = std::unique_ptr<rtc::Thread>( threads->getWorkerThread());

    RTC_LOG(LS_INFO) << "Creating signaling thread";
    signaling_thread = rtc::Thread::Create();
    RTC_LOG(LS_INFO) << "Signaling thread=>" << signaling_thread;
    ;
    signaling_thread->SetName("signaling_thread", this);
    RTC_LOG(LS_INFO) << "Signaling thread is started=>" << signaling_thread->Start();

    //    signaling_thread  = std::unique_ptr<rtc::Thread>( threads->getMediaThread() );

    auto audioEncoderFactory = webrtc::CreateBuiltinAudioEncoderFactory();
    auto audioEncoderCodecs = audioEncoderFactory->GetSupportedEncoders();
    RTC_LOG(LS_INFO) << "WebRTC BuiltIn audio supported encoders:";
    for (auto& c : audioEncoderCodecs) {
        RTC_LOG(LS_INFO) << "codec:" << c.format.name << "/" << c.format.clockrate_hz << "/"
                         << c.format.num_channels;
    }
    auto audioDecoderFactory = webrtc::CreateBuiltinAudioDecoderFactory();
    auto audioDecoderCodecs = audioDecoderFactory->GetSupportedDecoders();
    RTC_LOG(LS_INFO) << "WebRTC BuiltIn audio supported decoders:";
    for (auto& c : audioDecoderCodecs) {
        RTC_LOG(LS_INFO) << "codec:" << c.format.name << "/" << c.format.clockrate_hz << "/"
                         << c.format.num_channels;
    }

    auto videoEncoderFactory = webrtc::CreateBuiltinVideoEncoderFactory();
    RTC_LOG(LS_INFO) << "WebRTC BuiltIn video supported encoders:";
    for (auto& c : videoEncoderFactory->GetSupportedFormats()) {
        RTC_LOG(LS_INFO) << "codec:" << c.ToString();
    }

    auto videoDecoderFactory = webrtc::CreateBuiltinVideoDecoderFactory();
    RTC_LOG(LS_INFO) << "WebRTC BuiltIn video supported decoders:";
    for (auto& c : videoEncoderFactory->GetSupportedFormats()) {
        RTC_LOG(LS_INFO) << "codec:" << c.ToString();
    }

    peer_connection_factory =
            webrtc::CreatePeerConnectionFactory(network_thread.get(),   /* network_thread */
                                                worker_thread.get(),    /* worker_thread */
                                                signaling_thread.get(), /* signaling_thread */
                                                nullptr,                /* default_adm */
                                                audioEncoderFactory,    //
                                                audioDecoderFactory,    //
                                                std::move(videoEncoderFactory),  //
                                                std::move(videoDecoderFactory),  //
                                                nullptr /* audio_mixer */,       //
                                                nullptr /* audio_processing */);

    RTC_LOG(LS_INFO) << "peer_connection_factory:" << peer_connection_factory.get();

    webrtc::PeerConnectionFactoryInterface::Options options;
    options.disable_encryption = false;
    peer_connection_factory->SetOptions(options);

    RTC_LOG(LS_INFO) << "Create audio source...";
    audioSource = peer_connection_factory->CreateAudioSource(cricket::AudioOptions());
    RTC_LOG(LS_INFO) << "Audio source is:" << audioSource.get();

    RTC_LOG(LS_INFO) << "Create video device...";
    auto vdi = webrtc::VideoCaptureFactory::CreateDeviceInfo();
    RTC_LOG(LS_INFO) << "Video capture numbers:" << vdi->NumberOfDevices();

    RTC_LOG(LS_INFO) << "WebRTC has be started.";
    return true;
}

bool WebRTC::stop() {
    RTC_LOG(LS_INFO) << "WebRTC will be destroy...";
    std::lock_guard<std::recursive_mutex> lock(mutex);

    // 销毁factory
    peer_connection_factory = nullptr;

    // 清除ssl
    rtc::CleanupSSL();

    _handlers.erase(_handlers.begin(), _handlers.end());

    RTC_LOG(LS_INFO) << "WebRTC has be destroyed.";
    return true;
}

bool WebRTC::isStarted() {
    return peer_connection_factory.get();
}

bool WebRTC::ensureStart() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return isStarted() ? true : start();
}

void WebRTC::addRTCHandler(OkRTCHandler* hand) {
    assert(hand);
    std::lock_guard<std::recursive_mutex> lock(mutex);
    _handlers.push_back(hand);
}

void WebRTC::removeRTCHandler(OkRTCHandler* hand) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    _handlers.erase(std::remove_if(_handlers.begin(), _handlers.end(),
                                   [&](OkRTCHandler* h) { return h == hand; }),
                    _handlers.end());
}

std::vector<OkRTCHandler*> WebRTC::getHandlers() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return _handlers;
}

bool WebRTC::quit(const std::string& peerId) {
    return false;
}

void WebRTC::setIceOptions(std::list<IceServer>& ices) {
    for (const auto& ice : ices) {
        addIceServer(ice);
    }
}

std::map<std::string, OIceUdp> WebRTC::getCandidates(const std::string& peerId) {
    auto conductor = getConductor(peerId);
    if (!conductor) return {};
    return fromIce(conductor->getLocalSdp());
}

webrtc::SdpType WebRTC::convertToSdpType(JingleSdpType sdpType_) {
    webrtc::SdpType t;
    switch (sdpType_) {
        case JingleSdpType::Answer: {
            t = webrtc::SdpType::kAnswer;
            break;
        }
        case JingleSdpType::Offer: {
            t = webrtc::SdpType::kOffer;
            break;
        }
        case JingleSdpType::Rollback: {
            t = webrtc::SdpType::kRollback;
            break;
        }
    }
    return t;
}

JingleSdpType WebRTC::convertFromSdpType(webrtc::SdpType sdpType) {
    JingleSdpType jingleSdpType;
    switch (sdpType) {
        case webrtc::SdpType::kAnswer:
            jingleSdpType = JingleSdpType::Answer;
            break;
        case webrtc::SdpType::kOffer:
            jingleSdpType = JingleSdpType::Offer;
            break;
        case webrtc::SdpType::kPrAnswer:
            jingleSdpType = JingleSdpType::Answer;
            break;
        case webrtc::SdpType::kRollback:
            jingleSdpType = JingleSdpType::Rollback;
            break;
    }
    return jingleSdpType;
}

std::unique_ptr<webrtc::SessionDescriptionInterface> WebRTC::convertToSdp(
        const OJingleContentAv& av) {
    auto sessionDescription = std::make_unique<cricket::SessionDescription>();
    cricket::ContentGroup group(cricket::GROUP_TYPE_BUNDLE);
    auto sdpType = convertToSdpType(av.sdpType);

    if (av.getSsrcBundle().empty()) {
        for (const auto& kv : av.getContents()) {
            auto& oSdp = kv.second;
            group.AddContentName(oSdp.name);

            auto& rtp = oSdp.rtp;
            auto& iceUdp = oSdp.iceUdp;

            // iceUdp
            cricket::TransportInfo ti = toTransportInfo(oSdp.name, iceUdp);
            sessionDescription->AddTransportInfo(ti);

            switch (rtp.media) {
                case Media::audio: {
                    auto ptr = std::make_unique<cricket::AudioContentDescription>();
                    setSsrc2(rtp.sources, rtp.ssrcGroup, ptr.get());
                    setRptExtensions(rtp.hdrExts, ptr.get());
                    ptr->set_rtcp_mux(rtp.rtcpMux);

                    for (auto& pt : rtp.payloadTypes) {
                        auto codec = cricket::CreateAudioCodec(pt.id, pt.name, pt.clockrate,
                                                               pt.channels);
                        setCodec(pt, codec);
                        ptr->AddCodec(codec);
                    }

                    sessionDescription->AddContent(oSdp.name, cricket::MediaProtocolType::kRtp,
                                                   std::move(ptr));
                    break;
                }
                case Media::video: {
                    auto ptr = std::make_unique<cricket::VideoContentDescription>();
                    setSsrc2(rtp.sources, rtp.ssrcGroup, ptr.get());
                    setRptExtensions(rtp.hdrExts, ptr.get());
                    ptr->set_rtcp_mux(rtp.rtcpMux);

                    for (auto& pt : rtp.payloadTypes) {
                        auto codec = cricket::CreateVideoCodec(pt.id, pt.name);
                        setCodec(pt, codec);
                        ptr->AddCodec(codec);
                    }

                    sessionDescription->AddContent(oSdp.name, cricket::MediaProtocolType::kRtp,
                                                   std::move(ptr));
                    break;
                }
                case Media::application: {
                    auto description = createDataDescription(oSdp);
                    sessionDescription->AddContent(oSdp.name, cricket::MediaProtocolType::kSctp,
                                                   std::move(description));
                    break;
                }
                default:
                    break;
            }
        }

        sessionDescription->AddGroup(group);

        auto ptr = webrtc::CreateSessionDescription(sdpType, av.sessionId, av.sessionVersion,
                                                    std::move(sessionDescription));
        int mline = 0;
        for (const auto& kv : av.getContents()) {
            auto& oSdp = kv.second;
            auto& iceUdp = oSdp.iceUdp;
            for (const auto& item : oSdp.iceUdp.candidates) {
                auto candidate = toCandidate(item, iceUdp);
                auto c = webrtc::CreateIceCandidate(iceUdp.mid, mline, candidate);
                ptr->AddCandidate(c.release());
                mline++;
            }
        }
        return ptr;
    } else {
        sessionDescription->AddGroup(group);
        std::vector<webrtc::IceCandidateInterface*> candidates;
        int mline = -1;
        for (auto& k : av.getSsrcBundle()) {
            auto participant = k.first;
            auto& ssrcBundle = k.second;

            for (auto& c : av.getContents()) {
                auto& sdp = c.second;
                auto& iceUdp = sdp.iceUdp;
                auto& rtp = sdp.rtp;

                if (rtp.media == Media::audio && !ssrcBundle.audioSources.empty()) {
                    auto& mid = ssrcBundle.audioSources[0].name;
                    group.AddContentName(mid);

                    auto audioPtr = std::make_unique<cricket::AudioContentDescription>();
                    setSsrc(ssrcBundle.audioSsrcGroups, ssrcBundle.audioSources, audioPtr.get());

                    setRptExtensions(rtp.hdrExts, audioPtr.get());
                    audioPtr->set_rtcp_mux(rtp.rtcpMux);
                    for (auto& pt : rtp.payloadTypes) {
                        auto codec = cricket::CreateAudioCodec(pt.id, pt.name, pt.clockrate,
                                                               pt.channels);
                        setCodec(pt, codec);
                        audioPtr->AddCodec(codec);
                    }

                    auto ti = toTransportInfo(mid, iceUdp);
                    sessionDescription->AddTransportInfo(ti);
                    sessionDescription->AddContent(mid, cricket::MediaProtocolType::kRtp,
                                                   std::move(audioPtr));

                    mline++;
                    for (const auto& item : iceUdp.candidates) {
                        auto candidate = toCandidate(item, iceUdp);
                        auto c = webrtc::CreateIceCandidate(mid, mline, candidate);
                        candidates.push_back(c.release());
                    }

                } else if (rtp.media == Media::video && !ssrcBundle.videoSources.empty()) {
                    auto& mid = ssrcBundle.videoSources[0].name;
                    group.AddContentName(mid);

                    auto videoPtr = std::make_unique<cricket::VideoContentDescription>();
                    setSsrc(ssrcBundle.videoSsrcGroups, ssrcBundle.videoSources, videoPtr.get());

                    setRptExtensions(rtp.hdrExts, videoPtr.get());
                    videoPtr->set_rtcp_mux(rtp.rtcpMux);
                    for (auto& pt : rtp.payloadTypes) {
                        auto codec = cricket::CreateVideoCodec(pt.id, pt.name);
                        setCodec(pt, codec);
                        videoPtr->AddCodec(codec);
                    }
                    auto ti = toTransportInfo(mid, iceUdp);
                    sessionDescription->AddTransportInfo(ti);
                    sessionDescription->AddContent(mid, cricket::MediaProtocolType::kRtp,
                                                   std::move(videoPtr));
                    mline++;
                    for (const auto& item : iceUdp.candidates) {
                        auto candidate = toCandidate(item, iceUdp);
                        auto c = webrtc::CreateIceCandidate(mid, mline, candidate);
                        candidates.push_back(c.release());
                    }
                } else if (rtp.media == Media::application) {
                    auto& mid = "data";
                    auto data = createDataDescription(sdp);
                    auto ti = toTransportInfo(mid, iceUdp);
                    sessionDescription->AddTransportInfo(ti);
                    sessionDescription->AddContent(mid, cricket::MediaProtocolType::kSctp,
                                                   std::move(data));
                    mline++;
                    for (const auto& item : iceUdp.candidates) {
                        auto candidate = toCandidate(item, iceUdp);
                        auto c = webrtc::CreateIceCandidate(mid, mline, candidate);
                        candidates.push_back(c.release());
                    }
                }
            }
        }

        auto ptr = webrtc::CreateSessionDescription(sdpType, av.sessionId, av.sessionVersion,
                                                    std::move(sessionDescription));
        for (auto c : candidates) {
            if (!ptr->AddCandidate(c)) {
                RTC_LOG(LS_WARNING) << " Can not add candidate: " << c->sdp_mid();
            }
        }
        return ptr;
    }
}

std::unique_ptr<cricket::SctpDataContentDescription> createDataDescription(const OSdp& sdp) {
    auto ptr = std::make_unique<cricket::SctpDataContentDescription>();
    // rtcp-mux
    ptr->set_rtcp_mux(sdp.rtp.rtcpMux);

    if (!sdp.iceUdp.sctp.protocol.empty()) {
        ptr->set_port(sdp.iceUdp.sctp.port);
        ptr->set_protocol(cricket::kMediaProtocolDtlsSctp);
        ptr->set_use_sctpmap(true);
    }
    return std::move(ptr);
}

void WebRTC::addIceServer(const IceServer& ice) {
    // Add the ice server.
    webrtc::PeerConnectionInterface::IceServer ss;

    ss.urls.push_back(ice.uri);
    ss.tls_cert_policy = webrtc::PeerConnectionInterface::kTlsCertPolicyInsecureNoCheck;
    ss.username = ice.username;
    ss.password = ice.password;

    _rtcConfig.servers.push_back(ss);
}

Conductor* WebRTC::getConductor(const std::string& peerId) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return _pcMap[peerId];
}

constexpr int LEN = 255;

Conductor* WebRTC::createConductor(const std::string& peerId, const std::string& sId, bool video) {
    std::lock_guard<std::recursive_mutex> lock(mutex);

    RTC_LOG(LS_INFO) << __FUNCTION__ << "peer:" << peerId << " sid:" << sId << " video:" << video;

    auto conductor = new Conductor(this, peerId, sId);
    if (!video) {
        // audio
        RTC_DLOG_V(rtc::LS_INFO) << "AddTrack audio source:" << audioSource.get();
        conductor->AddAudioTrack(audioSource.get());
    } else {
        // video
        RTC_DLOG_V(rtc::LS_INFO) << "AddTrack audio source:" << audioSource.get();
        conductor->AddAudioTrack(audioSource.get());

        auto deviceInfo = webrtc::VideoCaptureFactory::CreateDeviceInfo();
        int num_devices = deviceInfo->NumberOfDevices();
        RTC_LOG(LS_INFO) << "Get number of video devices:" << num_devices;

        if (0 < num_devices) {
            // TODO 默认获取第一个视频设备
            int selected = 0;

            char name[LEN] = {};
            char uid[LEN] = {};
            char puid[LEN] = {};
            deviceInfo->GetDeviceName(selected, name, LEN, uid, LEN, puid, LEN);

            RTC_LOG(LS_INFO) << "Get video device {name:" << name << ", uid:" << uid
                             << ", productUid:" << puid << "}";

            videoCapture = createVideoCapture(uid);
            conductor->AddVideoTrack(videoCapture->source().get());

            sink = std::make_shared<VideoSink>(_handlers);
            videoCapture->setOutput(sink);
        }
    }

    return conductor;
}

void WebRTC::setRemoteDescription(const std::string& peerId, const OJingleContentAv& av) {
    auto conductor = getConductor(peerId);
    auto desc = convertToSdp(av);
    conductor->setRemoteDescription(std::move(desc));
}

void WebRTC::setTransportInfo(const std::string& peerId,
                              const std::string& sId,
                              const ortc::OIceUdp& iceUdp) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " peerId:" << peerId << " sId:" << sId;

    Conductor* conductor = getConductor(peerId);
    if (!conductor) {
        RTC_LOG(LS_WARNING) << "Unable to find conductor.";
        return;
    }

    int mline = 0;
    for (auto& _candidate : iceUdp.candidates) {
        if (_candidate.ip.empty() || _candidate.port <= 0) continue;

        std::string type;
        switch (_candidate.type) {
            case Type::Host:
                type = cricket::LOCAL_PORT_TYPE;
                break;
            case Type::PeerReflexive:
                type = cricket::PRFLX_PORT_TYPE;
                break;
            case Type::Relayed:
                type = cricket::RELAY_PORT_TYPE;
                break;
            case Type::ServerReflexive:
                type = cricket::STUN_PORT_TYPE;
                break;
        }

        cricket::Candidate candidate(_candidate.component,
                                     _candidate.protocol,
                                     ::rtc::SocketAddress(_candidate.ip, _candidate.port),
                                     _candidate.priority,
                                     iceUdp.ufrag,
                                     iceUdp.pwd,
                                     type,
                                     _candidate.generation,
                                     _candidate.foundation,
                                     _candidate.network);

        if (!_candidate.rel_addr.empty()) {
            ::rtc::SocketAddress raddr(_candidate.rel_addr, _candidate.rel_port);
            candidate.set_related_address(raddr);
        }

        auto jsep_candidate = webrtc::CreateIceCandidate(iceUdp.mid, mline, candidate);
        conductor->setTransportInfo(std::move(jsep_candidate));
        mline++;
    }
}

void WebRTC::setMute(bool mute) {
    for (auto it : _pcMap) {
        it.second->setMute(mute);
    }
}

void WebRTC::setRemoteMute(bool mute) {
    for (auto it : _pcMap) {
        it.second->setRemoteMute(mute);
    }
}

void WebRTC::addSource(const std::string& peerId,
                       const std::map<std::string, ortc::OMeetSSRCBundle>& map) {
    auto c = getConductor(peerId);
    if (!c) {
        RTC_LOG(LS_WARNING) << "No existing conductor!";
        return;
    }
    auto remoteDescription = c->getRemoteDescription();
    if (!remoteDescription) {
        RTC_LOG(LS_WARNING) << "No remote description!";
        return;
    }
    auto d = remoteDescription->Clone();
    for (const auto& item : map) {
        auto k = item.first;
        auto& ssrcBundle = item.second;
        RTC_LOG(LS_INFO) << " participant: " << k;

        if (!ssrcBundle.audioSources.empty()) {
            auto& mid = ssrcBundle.audioSources[0].name;
            std::string jvb_a0 = "jvb-a0";
            const cricket::ContentInfo* pContentInfo = d->description()->GetContentByName(jvb_a0);
            if (pContentInfo) {
                auto jvba = pContentInfo->media_description()->as_audio();
                auto ti0 = d->description()->GetTransportInfoByName(jvb_a0);
                cricket::TransportInfo ti(mid, ti0->description);
                d->description()->AddTransportInfo(ti);

                auto audioPtr = addAudioSsrcBundle(ssrcBundle);
                audioPtr->set_codecs(jvba->codecs());
                audioPtr->set_rtcp_mux(jvba->rtcp_mux());
                audioPtr->set_rtp_header_extensions(jvba->rtp_header_extensions());

                d->description()->AddContent(mid, cricket::MediaProtocolType::kRtp,
                                             std::move(audioPtr));
            }
        }

        if (!ssrcBundle.videoSources.empty()) {
            auto& mid = ssrcBundle.videoSources[0].name;
            std::string jvb_v0 = "jvb-v0";
            const cricket::ContentInfo* pContentInfo = d->description()->GetContentByName(jvb_v0);
            if (pContentInfo) {
                auto jvbv = pContentInfo->media_description()->as_video();
                auto ti0 = d->description()->GetTransportInfoByName(jvb_v0);

                cricket::TransportInfo ti(mid, ti0->description);
                d->description()->AddTransportInfo(ti);

                auto videoPtr = addVideoSsrcBundle(ssrcBundle);
                videoPtr->set_codecs(jvbv->codecs());
                videoPtr->set_rtcp_mux(jvbv->rtcp_mux());
                videoPtr->set_rtp_header_extensions(jvbv->rtp_header_extensions());

                d->description()->AddContent(mid, cricket::MediaProtocolType::kRtp,
                                             std::move(videoPtr));
            }
        }
    }

    std::unique_ptr<cricket::SessionDescription> x(d->description());
    auto d1 = webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, d->session_id(),
                                               d->session_version(), std::move(x));
    for (const auto& item : map) {
        auto& ssrcBundle = item.second;

        auto ci = d->candidates(0);

        for (auto i = 0; i < ci->count(); i++) {
            std::string sdp;
            ci->at(i)->ToString(&sdp);

            webrtc::SdpParseError error;
            auto amid = ssrcBundle.audioSources[0].name;

            int mline = -1;
            for (auto& content : d->description()->contents()) {
                ++mline;
                if (content.mid() == amid) {
                    auto c = webrtc::CreateIceCandidate(amid, mline, sdp, &error);
                    if (!d1->AddCandidate(c)) {
                        RTC_LOG(LS_WARNING) << " Can not add candidate: " << c->sdp_mid();
                    }
                }

                if (!ssrcBundle.videoSources.empty()) {
                    auto vmid = ssrcBundle.videoSources[0].name;
                    if (content.mid() == vmid) {
                        auto c1 = webrtc::CreateIceCandidate(vmid, mline, sdp, &error);
                        if (!d1->AddCandidate(c1)) {
                            RTC_LOG(LS_WARNING) << " Can not add candidate: " << c1->sdp_mid();
                        }
                    }
                }
            }
        }
    }
    c->setRemoteDescription(std::move(d1));
}

bool WebRTC::CreateOffer(const std::string& peerId, const std::string& sId, bool video) {
    auto conductor = _pcMap[peerId];
    if (conductor) {
        RTC_LOG(LS_WARNING) << "Exist conductor.";
        return false;
    }

    conductor = createConductor(peerId, sId, video);
    conductor->CreateOffer();
    _pcMap[peerId] = conductor;
    return true;
}

void WebRTC::SessionTerminate(const std::string& peerId) {
    //    quit(peerId);
}

void WebRTC::CreateAnswer(const std::string& peerId, const OJingleContentAv& av) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " peerId:" << peerId;
    Conductor* conductor = createConductor(peerId, av.sessionId, av.isVideo());
    auto sdp = convertToSdp(av);
    conductor->setRemoteDescription(std::move(sdp));
    conductor->CreateAnswer();
    _pcMap[peerId] = conductor;
}

void WebRTC::getLocalSdp(const std::string& peerId, OJingleContentAv& av) {
    auto conductor = getConductor(peerId);
    assert(conductor);

    fromSdp(conductor->getLocalSdp(), av);
}

size_t WebRTC::getVideoSize() {
    RTC_LOG(LS_INFO) << "Create video device...";
    auto vdi = webrtc::VideoCaptureFactory::CreateDeviceInfo();
    RTC_LOG(LS_INFO) << "Video capture numbers:" << vdi->NumberOfDevices();
    return vdi->NumberOfDevices();
}

std::shared_ptr<VideoCaptureInterface> WebRTC::createVideoCapture(const std::string& deviceId) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " deviceId: " << deviceId;

    if (deviceId.empty()) {
        RTC_LOG(LS_WARNING) << "Empty deviceId!";
        return {};
    }

    //  if (auto result = videoCapture.get()) {
    //    if (deviceId) {
    //      result->switchToDevice(*deviceId,  isScreenCapture);
    //    }
    //    return videoCapture;
    //  }

    return VideoCaptureInterface::Create(signaling_thread.get(), worker_thread.get(), deviceId);
}

}  // namespace lib::ortc
