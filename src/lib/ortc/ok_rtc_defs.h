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

#include <list>
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
typedef std::list<Dtls> DtlsList;

/**
 * Describes a single transport candidate.
 */
struct Candidate {
    std::string component;  /**< A Component ID as defined in ICE-CORE. */
    std::string foundation; /**< A Foundation as defined in ICE-CORE.*/
    std::string generation; /**< An index, starting at 0, that enables the parties to keep track of
                               updates to the candidate throughout the life of the session. */
    std::string id;         /**< A unique identifier for the candidate. */
    std::string ip;         /**< The IP address for the candidate transport mechanism. */
    std::string network;  /**< An index, starting at 0, referencing which network this candidate is
                             on for a given peer. */
    int port;             /**< The port at the candidate IP address. */
    int priority;         /**< A Priority as defined in ICE-CORE. */
    std::string protocol; /**< The protocol to be used. Should be @b udp. */
    std::string tcptype;
    std::string rel_addr; /**< A related address as defined in ICE-CORE. */
    int rel_port;         /**< A related port as defined in ICE-CORE. */
    Type type;            /**< A Candidate Type as defined in ICE-CORE. */
};

/** A list of transport candidates. */
typedef std::list<Candidate> CandidateList;

struct OIceUdp {
    std::string mid;
    int mline = 0;
    std::string ufrag;
    std::string pwd;
    Dtls dtls;
    CandidateList candidates;
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
    int channels;
    Parameters parameters;
    Feedbacks feedbacks;
};

/** A list of file information structs. */
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
};

// struct OFile {
//     Jingle::FileTransfer::FileList files;
//     Jingle::IBB ibb;
// };

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

struct OSdp : public OContent {
    ORTP rtp;
    OIceUdp iceUdp;
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
    [[nodiscard]] inline JingleCallType getCallType() const { return callType; }

    [[nodiscard]] inline JingleSdpType getSdpType() const { return sdpType; }

    JingleSdpType sdpType;

    std::string sessionId;
    std::string sessionVersion;

    JingleCallType callType;

};

struct OJingleContentFile : public OJingleContent {
    std::vector<OFile> contents;
    bool isValid();
};

struct OJingleContentAv : public OJingleContent {
public:
    std::vector<OSdp> contents;

    bool isValid();

    [[nodiscard]] bool isVideo() const;
};

}  // namespace lib::ortc
