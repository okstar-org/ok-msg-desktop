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

#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace lib::ortc {

#define SESSION_VERSION "3"

enum class Media { invalid = -1, audio = 0, video = 1, application = 2 };
typedef std::list<Media> Medias;

enum class Type {
    Host,           /**< A host candidate. */
    PeerReflexive,  /**< A peer reflexive candidate. */
    Relayed,        /**< A relayed candidate. */
    ServerReflexive /**< A server reflexive candidate. */
};

struct Dtls {
    std::string hash;
    std::string setup;
    std::string fingerprint;
};

struct Sctp {
    std::string protocol;
    uint32_t port;
    uint32_t streams;
};

/**
 * Describes a single transport candidate.
 */
struct Candidate {
    int component;          /**< A Component ID as defined in ICE-CORE. */
    std::string foundation; /**< A Foundation as defined in ICE-CORE.*/
    uint32_t generation;    /**< An index, starting at 0, that enables the parties to keep track of
                                  updates to the candidate throughout the life of the session. */
    std::string id;         /**< A unique identifier for the candidate. */
    std::string ip;         /**< The IP address for the candidate transport mechanism. */
    uint32_t network;     /**< An index, starting at 0, referencing which network this candidate is
                                on for a given peer. */
    uint32_t port;        /**< The port at the candidate IP address. */
    uint32_t priority;    /**< A Priority as defined in ICE-CORE. */
    std::string protocol; /**< The protocol to be used. Should be @b udp. */
    std::string tcptype;
    std::string rel_addr; /**< A related address as defined in ICE-CORE. */
    uint32_t rel_port;    /**< A related port as defined in ICE-CORE. */
    Type type;            /**< A Candidate Type as defined in ICE-CORE. */
};

/** A list of transport candidates. */
typedef std::list<Candidate> CandidateList;

struct OIceUdp {
    std::string mid;
    //    int mline = 0;
    std::string ufrag;
    std::string pwd;
    Dtls dtls;
    Sctp sctp;
    CandidateList candidates;

    //    OIceUdp() = default;
    //    OIceUdp(std::string mid_,
    //            int mline_,
    //            std::string ufrag_,
    //            std::string pwd_,
    //            Dtls dtls_,
    //            CandidateList candidates_)
    //            : mid{mid_}
    //            , mline{mline_}
    //            , ufrag(ufrag_)
    //            , pwd(pwd_)
    //            , dtls(dtls_)
    //            , candidates(candidates_) {}
};

struct Feedback {
    std::string type;
    std::string subtype;
};
typedef std::list<Feedback> Feedbacks;

struct Parameter {
    std::string name;
    std::string value;
};

typedef std::list<Parameter> Parameters;

/**
 * A struct holding information about a PayloadType.
 */
struct PayloadType {
    int id;           /**< The type's id */
    std::string name; /**< The type's name. */
    int clockrate;    /**< The clockrate. */
    int bitrate;
    size_t channels;
    Parameters parameters;
    Feedbacks feedbacks;
};
typedef std::list<PayloadType> PayloadTypes;

struct HdrExt {
    int id;          /**< The type's id */
    std::string uri; /**< The type's name. */
};
typedef std::list<HdrExt> HdrExts;

struct Source {
    std::string ssrc;
    Parameters parameters;
};
typedef std::list<Source> Sources;

struct SsrcGroup {
    // FID（Flow Identification，流识别）：用于表示同一媒体流的不同部分或变体，如原始流和重传流。
    //      在这种关系中，一个FID组内的SSRC共享相同的媒体源，但可能具有不同的编码参数或传输特性。
    // FEC（Forward Error Correction，前向纠错）：用于表示与特定媒体流相关联的前向纠错流。
    //      在这种关系中，FEC流用于为原始媒体流提供错误恢复能力。
    // SIM（Simulcast，联播）：用于表示同一媒体源的不同质量或分辨率的流。
    //      在这种关系中，SIM组内的SSRC代表同一摄像头或麦克风捕获的媒体的不同编码版本，如高清和低分辨率版本。
    //      这些流可以同时传输，并根据接收方的带宽和性能进行动态选择。
    std::string semantics;
    std::vector<std::string> ssrcs;
};

struct ORTP {
    Media media = Media::invalid;
    PayloadTypes payloadTypes;
    HdrExts hdrExts;
    Sources sources;
    SsrcGroup ssrcGroup;
    bool rtcpMux;

    ORTP() = default;

    ORTP(Media media,
         PayloadTypes payloadTypes,
         HdrExts hdrExts,
         Sources sources,
         SsrcGroup ssrcGroup,
         bool rtcpMux)
            : media{media}
            , payloadTypes(payloadTypes)
            , hdrExts(hdrExts)
            , sources(sources)
            , ssrcGroup(ssrcGroup)
            , rtcpMux{rtcpMux} {}
};

struct OMeetSource {
    std::string ssrc;
    // resource of meet participant
    std::string name;
    // 格式： "d11a153b-audio-0-1 3f32f7da-2665-4321-8335-868bf394797c-1"
    std::string msid;
};

struct OMeetSSRCBundle {
    std::vector<OMeetSource> videoSources;
    SsrcGroup videoSsrcGroups;
    std::vector<OMeetSource> audioSources;
    SsrcGroup audioSsrcGroups;
};

struct OContent {
    std::string name;
};

struct OFile : public OContent {
    std::string id;
    std::string sId;

    std::string date;
    std::string desc;
    std::string hash;
    std::string hash_algo;
    long int size = 0;
    bool range = false;
    long int offset = 0;
};

enum class JingleSdpType {
    Offer,
    Answer,
    Rollback,
};

// 呼叫类型
enum class JingleCallType {
    none,  // none
    file,  // file
    av,    // audio & video
};

struct OJingleContent {
public:
    [[nodiscard]] inline JingleCallType getCallType() const {
        return callType;
    }

    [[nodiscard]] inline JingleSdpType getSdpType() const {
        return sdpType;
    }

    JingleSdpType sdpType;

    std::string sessionId;
    std::string sessionVersion;

    JingleCallType callType;
};

struct OJingleContentFile : public OJingleContent {
    std::vector<OFile> contents;
    bool isValid();
};

struct OSdp : public OContent {
    ORTP rtp;
    OIceUdp iceUdp;
};

struct OJingleContentAv : public OJingleContent {
public:
    bool isValid();

    [[nodiscard]] bool isVideo() const;

    [[nodiscard]] const std::map<std::string, OMeetSSRCBundle>& getSsrcBundle() const {
        return ssrcBundle;
    }

    [[nodiscard]] std::map<std::string, OMeetSSRCBundle>& getSsrcBundle() {
        return ssrcBundle;
    }

    [[nodiscard]] const std::map<std::string, OSdp>& getContents() const {
        return contents;
    };

    void put(const std::string& name, const OSdp& sdp) {
        contents[name] = sdp;
    }

    OSdp& load(const std::string& name) {
        auto find = contents.find(name);
        if (find != contents.end()) {
            return find->second;
        } else {
            OSdp oSdp;
            oSdp.name = name;
            contents[name] = oSdp;
        }
        return contents[name];
    }

private:
    std::map<std::string, OSdp> contents;
    std::map<std::string, OMeetSSRCBundle> ssrcBundle;
};

enum class IceGatheringState { New, Gathering, Complete };

enum class IceConnectionState {
    New,
    Checking,
    Connected,
    Completed,
    Failed,
    Disconnected,
    Closed,
    Max,
};

enum class PeerConnectionState {
    New,
    Connecting,
    Connected,
    Disconnected,
    Failed, Closed };

std::string PeerConnectionStateAsStr(PeerConnectionState state);

enum SignalingState {
    Stable,
    HaveLocalOffer,
    HaveLocalPrAnswer,
    HaveRemoteOffer,
    HaveRemotePrAnswer,
    Closed,
};

}  // namespace lib::ortc
