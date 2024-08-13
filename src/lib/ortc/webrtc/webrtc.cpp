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
#include "StaticThreads.h"
#include "ok_conductor.h"

#include <memory>
#include <string>
#include <utility>

#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/create_peerconnection_factory.h>
#include <api/peer_connection_interface.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <modules/video_capture/video_capture_factory.h>
#include <pc/video_track_source.h>
#include <rtc_base/logging.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/string_encode.h>
#include <rtc_base/thread.h>

namespace lib {
namespace ortc {

WebRTC::WebRTC() : peer_connection_factory{nullptr}, _rtcHandler{nullptr} {
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
    start_mtx.lock();

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

    peer_connection_factory =
            webrtc::CreatePeerConnectionFactory(network_thread.get(),   /* network_thread */
                                                worker_thread.get(),    /* worker_thread */
                                                signaling_thread.get(), /* signaling_thread */
                                                nullptr,                /* default_adm */
                                                webrtc::CreateBuiltinAudioEncoderFactory(),  //
                                                webrtc::CreateBuiltinAudioDecoderFactory(),  //
                                                webrtc::CreateBuiltinVideoEncoderFactory(),  //
                                                webrtc::CreateBuiltinVideoDecoderFactory(),  //
                                                nullptr /* audio_mixer */,                   //
                                                nullptr /* audio_processing */);

    RTC_LOG(LS_INFO) << "peer_connection_factory:" << peer_connection_factory.get();

    webrtc::PeerConnectionFactoryInterface::Options options;
    options.disable_encryption = false;
    peer_connection_factory->SetOptions(options);

    RTC_LOG(LS_INFO) << "Create audio source...";
    audioSource = peer_connection_factory->CreateAudioSource(::cricket::AudioOptions());
    RTC_LOG(LS_INFO) << "Audio source is:" << audioSource.get();

    RTC_LOG(LS_INFO) << "Create video device...";
    auto vdi = webrtc::VideoCaptureFactory::CreateDeviceInfo();
    RTC_LOG(LS_INFO) << "Video capture numbers:" << vdi->NumberOfDevices();

    start_mtx.unlock();

    RTC_LOG(LS_INFO) << "WebRTC has be started.";
    return true;
}

bool WebRTC::stop() {
    RTC_LOG(LS_INFO) << "WebRTC will be destroy...";
    std::lock_guard<std::recursive_mutex> lock(start_mtx);

    // 销毁factory
    peer_connection_factory = nullptr;

    // 清除ssl
    rtc::CleanupSSL();
    RTC_LOG(LS_INFO) << "WebRTC has be destroyed.";
    return true;
}

bool WebRTC::isStarted() { return peer_connection_factory.get(); }

bool WebRTC::ensureStart() {
    std::lock_guard<std::recursive_mutex> lock(start_mtx);
    return isStarted() ? true : start();
}

void WebRTC::addRTCHandler(OkRTCHandler* hand) { _rtcHandler = hand; }

bool WebRTC::call(const std::string& peerId, const std::string& sId, bool video) {
    RTC_LOG(LS_INFO) << "peerId:" << peerId;
    return createConductor(peerId, sId, video);
}

bool WebRTC::quit(const std::string& peerId) { return false; }

void WebRTC::setIceOptions(std::list<IceServer>& ices) {
    for (auto ice : ices) {
        addIceServer(ice);
    }
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
        const OJingleContentAv& context) {
    auto sdpType = convertToSdpType(context.sdpType);
    auto sessionDescription = std::make_unique<::cricket::SessionDescription>();
    auto& contents = context.contents;

    cricket::ContentGroup group(cricket::GROUP_TYPE_BUNDLE);

    for (const auto& content : contents) {
        group.AddContentName(content.name);

        auto& rtp = content.rtp;
        auto& iceUdp = content.iceUdp;

        // iceUdp
        ::cricket::TransportInfo ti;
        ti.content_name = content.name;
        ti.description.ice_ufrag = content.iceUdp.ufrag;
        ti.description.ice_pwd = content.iceUdp.pwd;

        ti.description.identity_fingerprint.reset(::cricket::TransportDescription::CopyFingerprint(
                rtc::SSLFingerprint::CreateFromRfc4572(content.iceUdp.dtls.hash,
                                                       content.iceUdp.dtls.fingerprint)));

        if (iceUdp.dtls.setup == "actpass") {
            ti.description.connection_role = ::cricket::CONNECTIONROLE_ACTPASS;
        } else if (iceUdp.dtls.setup == "active") {
            ti.description.connection_role = ::cricket::CONNECTIONROLE_ACTIVE;
        } else if (iceUdp.dtls.setup == "passive") {
            ti.description.connection_role = ::cricket::CONNECTIONROLE_PASSIVE;
        } else if (iceUdp.dtls.setup == "holdconn") {
            ti.description.connection_role = ::cricket::CONNECTIONROLE_HOLDCONN;
        } else {
            ti.description.connection_role = ::cricket::CONNECTIONROLE_NONE;
        }

        sessionDescription->AddTransportInfo(ti);

        switch (rtp.media) {
            case Media::audio: {
                auto acd = std::make_unique<::cricket::AudioContentDescription>();
                for (auto& pt : rtp.payloadTypes) {
                    ::cricket::AudioCodec ac(pt.id, pt.name, pt.clockrate, pt.bitrate, pt.channels);
                    for (auto& e : pt.parameters) {
                        ac.SetParam(e.name, e.value);
                    }
                    for (auto& e : pt.feedbacks) {
                        ::cricket::FeedbackParam fb(e.type, e.subtype);
                        ac.AddFeedbackParam(fb);
                    }
                    acd->AddCodec(ac);
                }

                for (auto& hdrext : rtp.hdrExts) {
                    webrtc::RtpExtension ext(hdrext.uri, hdrext.id);
                    acd->AddRtpHeaderExtension(ext);
                }

                ::cricket::StreamParams streamParams;
                for (auto& src : rtp.sources) {
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

                auto g = rtp.ssrcGroup;
                std::vector<uint32_t> ssrcs;
                std::transform(g.ssrcs.begin(), g.ssrcs.end(), std::back_inserter(ssrcs),
                               [](auto s) -> uint32_t { return std::stoul(s); });
                ::cricket::SsrcGroup ssrcGroup(g.semantics, ssrcs);

                // ssrc-groups
                streamParams.ssrc_groups.emplace_back(ssrcGroup);

                // ssrc
                acd->AddStream(streamParams);

                // rtcp-mux
                acd->set_rtcp_mux(rtp.rtcpMux);

                sessionDescription->AddContent(content.name, ::cricket::MediaProtocolType::kRtp,
                                               std::move(acd));
                break;
            }
            case Media::video: {
                auto vcd = std::make_unique<::cricket::VideoContentDescription>();
                for (auto& pt : rtp.payloadTypes) {
                    auto vc = ::cricket::VideoCodec(pt.id, pt.name);
                    for (auto& e : pt.parameters) {
                        vc.SetParam(e.name, e.value);
                    }
                    for (auto& e : pt.feedbacks) {
                        ::cricket::FeedbackParam fb(e.type, e.subtype);
                        vc.AddFeedbackParam(fb);
                    }
                    vcd->AddCodec(vc);
                }
                for (auto& hdrExt : rtp.hdrExts) {
                    webrtc::RtpExtension ext(hdrExt.uri, hdrExt.id);
                    vcd->AddRtpHeaderExtension(ext);
                }
                vcd->set_rtcp_mux(rtp.rtcpMux);

                ::cricket::StreamParams streamParams;
                for (auto& src : rtp.sources) {
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

                // ssrc-group
                auto g = rtp.ssrcGroup;
                std::vector<uint32_t> ssrcs;
                std::transform(g.ssrcs.begin(), g.ssrcs.end(), std::back_inserter(ssrcs),
                               [](auto s) -> uint32_t { return std::stoul(s); });
                ::cricket::SsrcGroup ssrcGroup(g.semantics, ssrcs);
                streamParams.ssrc_groups.emplace_back(ssrcGroup);
                vcd->AddStream(streamParams);

                sessionDescription->AddContent(content.name, ::cricket::MediaProtocolType::kRtp,
                                               std::move(vcd));
                break;
            }
            default:
                break;
        }
    }

    sessionDescription->AddGroup(group);
    return webrtc::CreateSessionDescription(sdpType, context.sessionId, context.sessionVersion,
                                            std::move(sessionDescription));
}

OJingleContentAv WebRTC::convertFromSdp(webrtc::SessionDescriptionInterface* desc) {
    OJingleContentAv osdp;
    osdp.sessionId = desc->session_id();
    osdp.sessionVersion = desc->session_version();

    // ContentGroup
    ::cricket::ContentGroup group(::cricket::GROUP_TYPE_BUNDLE);

    auto sd = desc->description();
    for (auto rtcContent : sd->contents()) {
        OSdp oContent;

        const std::string& name = rtcContent.mid();
        // qDebug(("Content name: %1").arg(qstring(name)));

        oContent.name = name;

        auto mediaDescription = rtcContent.media_description();
        // media type
        auto mt = mediaDescription->type();

        // rtcp_mux
        oContent.rtp.rtcpMux = mediaDescription->rtcp_mux();

        // Transport
        auto ti = sd->GetTransportInfoByName(name);

        // pwd ufrag
        oContent.iceUdp.pwd = ti->description.ice_pwd;
        oContent.iceUdp.ufrag = ti->description.ice_ufrag;

        // fingerprint
        if (ti->description.identity_fingerprint) {
            oContent.iceUdp.dtls.fingerprint =
                    ti->description.identity_fingerprint->GetRfc4572Fingerprint();
            oContent.iceUdp.dtls.hash = ti->description.identity_fingerprint->algorithm;

            // connection_role
            std::string setup;
            ::cricket::ConnectionRoleToString(ti->description.connection_role, &setup);
            oContent.iceUdp.dtls.setup = setup;
        }

        // hdrext
        auto hdrs = mediaDescription->rtp_header_extensions();

        for (auto& hdr : hdrs) {
            HdrExt hdrExt = {hdr.id, hdr.uri};
            oContent.rtp.hdrExts.push_back(hdrExt);
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
                oContent.rtp.sources.emplace_back(source);
            }
        }

        // ssrc-group
        if (oContent.rtp.sources.size() >= 2) {
            oContent.rtp.ssrcGroup.semantics = "FID";
            for (auto& ssrc : oContent.rtp.sources) {
                oContent.rtp.ssrcGroup.ssrcs.emplace_back(ssrc.ssrc);
            }
        }

        // codecs
        switch (mt) {
            case ::cricket::MediaType::MEDIA_TYPE_AUDIO: {
                oContent.rtp.media = Media::audio;
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

                    oContent.rtp.payloadTypes.emplace_back(type);
                }

                break;
            }
            case ::cricket::MediaType::MEDIA_TYPE_VIDEO: {
                oContent.rtp.media = Media::video;
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

                    oContent.rtp.payloadTypes.emplace_back(type);
                }
                break;
            }
            case ::cricket::MediaType::MEDIA_TYPE_DATA: {
                break;
            }
            case cricket::MEDIA_TYPE_UNSUPPORTED:
                break;
        }

        osdp.contents.push_back(oContent);
    }

    return osdp;
}

void WebRTC::addIceServer(const IceServer& ice) {
    // Add the ice server.
    webrtc::PeerConnectionInterface::IceServer ss;

    ss.urls.push_back(ice.uri);
    ss.tls_cert_policy = webrtc::PeerConnectionInterface::kTlsCertPolicyInsecureNoCheck;
    if (!ice.username.empty()) {
        ss.username = ice.username;
        ss.password = ice.password;
    }
    _rtcConfig.servers.push_back(ss);
}

Conductor* WebRTC::getConductor(const std::string& peerId) { return _pcMap[peerId]; }

Conductor* WebRTC::createConductor(const std::string& peerId, const std::string& sId, bool video) {
    RTC_LOG_F(LS_INFO) << "peer:" << peerId << " sid:" << sId << " video:" << video;

    auto conductor = _pcMap[peerId];
    if (conductor) {
        return conductor;
    }

    conductor = new Conductor(this, peerId, sId);
    _pcMap[peerId] = conductor;

    if (!video) {
        // audio
        RTC_DLOG_V(rtc::LS_INFO) << "AddTrack audio..." << audioSource.get();
        conductor->AddAudioTrack(audioSource.get());
    } else {
        // video
        RTC_DLOG_V(rtc::LS_INFO) << "AddTrack audio..." << audioSource.get();
        conductor->AddAudioTrack(audioSource.get());

        auto vdi = webrtc::VideoCaptureFactory::CreateDeviceInfo();
        int num_devices = vdi->NumberOfDevices();
        RTC_LOG(LS_INFO) << "Get number of video devices:" << num_devices;

        if (0 < num_devices) {
            // 获取第一个视频设备
            int selected = 0;

            char name[50] = {0};
            char uid[50] = {0};
            char puid[50] = {0};
            vdi->GetDeviceName(selected, name, 50, uid, 50, puid, 50);
            RTC_LOG(LS_INFO) << "Video device:" << name;
            RTC_LOG(LS_INFO) << "Video device uid:" << uid;

            videoCapture = createVideoCapture(uid);
            conductor->AddVideoTrack(videoCapture->source().get());

            sink = std::make_shared<VideoSink>(_rtcHandler);
            videoCapture->setOutput(sink);
        }
    }

    return conductor;
}

void WebRTC::setRemoteDescription(const std::string& peerId,
                                  const OJingleContentAv& jingleContext) {
    auto conductor = getConductor(peerId);
    conductor->SetRemoteDescription(std::move(convertToSdp(jingleContext)));
}

void WebRTC::setTransportInfo(const std::string& peerId,
                              const std::string& sId,
                              const ortc::OIceUdp& iceUdp) {
    RTC_LOG_F(LS_INFO) << "peerId:" << peerId;

    Conductor* conductor = createConductor(peerId, sId, false);
    if (!conductor) {
        RTC_LOG_F(LS_WARNING) << "conductor is null!";
        return;
    }

    int i = 0;
    for (auto& _candidate : iceUdp.candidates) {
        ::cricket::Candidate candidate;
        if (!_candidate.id.empty()) {
            candidate.set_id(_candidate.id);
        } else {
            candidate.set_id(std::to_string(i));
        }
        if (_candidate.component.empty()) {
            return;
        }
        if (_candidate.generation.empty()) {
            return;
        }

        candidate.set_foundation(_candidate.foundation);
        candidate.set_component(std::stoi(_candidate.component));
        candidate.set_tcptype(_candidate.tcptype);    // passive
        candidate.set_protocol(_candidate.protocol);  // udp,ssltcp
        candidate.set_priority(_candidate.priority);
        candidate.set_generation(std::stoi(_candidate.generation));

        if (!_candidate.network.empty()) {
            candidate.set_network_id(std::stoi(_candidate.network));
        } else {
            candidate.set_network_id(i);
        }

        candidate.set_address(::rtc::SocketAddress(_candidate.ip, _candidate.port));

        /**
         *  const auto& host = LOCAL_PORT_TYPE;
            const auto& srflx = STUN_PORT_TYPE;
            const auto& relay = RELAY_PORT_TYPE;
            const auto& prflx = PRFLX_PORT_TYPE;
         */

        switch (_candidate.type) {
            case Type::Host:
                candidate.set_type(::cricket::LOCAL_PORT_TYPE);
                break;
            case Type::PeerReflexive:
                candidate.set_type(::cricket::PRFLX_PORT_TYPE);
                break;
            case Type::Relayed:
                candidate.set_type(::cricket::RELAY_PORT_TYPE);
                break;
            case Type::ServerReflexive:
                candidate.set_type(::cricket::STUN_PORT_TYPE);
                break;
        }
        if (!_candidate.rel_addr.empty()) {
            rtc::SocketAddress raddr;
            raddr.SetIP(_candidate.rel_addr);
            raddr.SetPort(_candidate.rel_port);
            candidate.set_related_address(raddr);
        }

        auto jsep_candidate = webrtc::CreateIceCandidate(iceUdp.mid, iceUdp.mline, candidate);
        conductor->setTransportInfo(std::move(jsep_candidate));
        i++;
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

void WebRTC::CreateOffer(const std::string& peerId) {
    Conductor* conductor = getConductor(peerId);
    conductor->CreateOffer();
}

void WebRTC::SessionTerminate(const std::string& peerId) {
    //    quit(peerId);
}

void WebRTC::CreateAnswer(const std::string& peerId, const OJingleContentAv& ca) {
    RTC_LOG_F(LS_INFO) << "peerId:" << peerId;

    Conductor* conductor = createConductor(peerId, ca.sessionId, ca.isVideo());
    if (!conductor) {
        RTC_LOG_F(LS_WARNING) << "conductor is null!";
        return;
    }
    // webrtc::SdpType::kOffer,
    auto sdp = convertToSdp(ca);
    conductor->SetRemoteDescription(std::move(sdp));
    conductor->CreateAnswer();
}

size_t WebRTC::getVideoSize() {
    RTC_LOG(LS_INFO) << "Create video device...";
    auto vdi = webrtc::VideoCaptureFactory::CreateDeviceInfo();
    RTC_LOG(LS_INFO) << "Video capture numbers:" << vdi->NumberOfDevices();
    return vdi->NumberOfDevices();
}

std::shared_ptr<VideoCaptureInterface> WebRTC::createVideoCapture(
        std::optional<std::string> deviceId, bool isScreenCapture) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " deviceId:" << *deviceId;

    if (deviceId->empty()) {
        return {};
    }

    //  if (auto result = videoCapture.get()) {
    //    if (deviceId) {
    //      result->switchToDevice(*deviceId,  isScreenCapture);
    //    }
    //    return videoCapture;
    //  }

    return VideoCaptureInterface::Create(signaling_thread.get(), worker_thread.get(), *deviceId);
}

}  // namespace ortc
}  // namespace lib
