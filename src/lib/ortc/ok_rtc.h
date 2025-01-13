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
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace lib::ortc {

enum class Media { invalid = -1, audio = 0, video = 1, application = 2 };
typedef std::vector<Media> Medias;

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
typedef std::vector<Candidate> CandidateList;

enum class TransportType {
    /**
     * <transport xmlns='urn:xmpp:jingle:transports:ice-udp:1'
     *  pwd='Cpf1J7KA6x+ejHOTd/VMGMTd' ufrag='KsLU'>
            <fingerprint xmlns='urn:xmpp:jingle:apps:dtls:0' setup='actpass'
     hash='sha-256'>8D:BB:7F:FC:E0:EF:FA:AE:3E:4B:95:70:21:D1:E6:85:75:F4:30:D2:35:26:FE:61:FB:A8:06:56:18:7E:92:97</fingerprint>
        </transport>
     */
    iceUdp,

    /**
     * <transport xmlns='urn:xmpp:jingle:transports:ibb:1'
            sid='+loOfHolkhIq2szVpQ039A'
            block-size='8192'
            />
     */
    ibb
};

struct OTransport {
    TransportType type;
};

struct OIceUdp : OTransport {
    std::string mid;
    std::string ufrag;
    std::string pwd;
    Dtls dtls;
    Sctp sctp;
    CandidateList candidates;
};

struct Feedback {
    std::string type;
    std::string subtype;
};
typedef std::vector<Feedback> Feedbacks;

struct Parameter {
    std::string name;
    std::string value;
};

typedef std::vector<Parameter> Parameters;

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
typedef std::vector<PayloadType> PayloadTypes;

struct HdrExt {
    int id;          /**< The type's id */
    std::string uri; /**< The type's name. */
};
typedef std::vector<HdrExt> HdrExts;

enum class VideoType { Camera, Desktop };
struct Source {
    std::string ssrc;
    std::string name;

    std::string videoType;
    //    Parameters parameters;
    std::string cname;
    // 格式： "d11a153b-audio-0-1 3f32f7da-2665-4321-8335-868bf394797c-1"
    std::string msid;
};
typedef std::vector<Source> Sources;

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
    Media media;
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

// struct OMeetSource {
//     std::string ssrc;
//     // resource of meet participant
//     std::string name;
//     // 格式： "d11a153b-audio-0-1 3f32f7da-2665-4321-8335-868bf394797c-1"
//     std::string msid;
// };

struct OMeetSSRCBundle {
    std::vector<Source> videoSources;
    SsrcGroup videoSsrcGroups;
    std::vector<Source> audioSources;
    SsrcGroup audioSsrcGroups;
};

struct OContent {
    std::string name;
};

struct OFileIBB : public OTransport {
    std::string sId;
    size_t blockSize;
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

// struct OJingleContentFile : public OJingleContent {
//     std::vector<OFile> contents;
//     [[nodiscard]] inline bool isValid() const {
//         for (const auto& c : contents)
//             if (!c.name.empty() && c.size > 0) return true;
//         return false;
//     }
// };

struct OSdp {
    std::string name;
    ORTP rtp;
    OIceUdp iceUdp;
    OFile file;
    OFileIBB ibb;

public:
    bool isFile() const {
        return !file.name.empty();
    }

    bool isAV() const {
        return !rtp.payloadTypes.empty();
    }

    bool isInvalid() const {
        return !isFile() && !isAV();
    }
};

struct OJingleContentMap : public OJingleContent {
public:
    [[nodiscard]] inline bool isValid() const {
        return !contents.empty();
    }

    [[nodiscard]] inline bool isVideo() const {
        for (auto& c : contents)
            if (c.second.rtp.media == Media::video) return true;
        return false;
    };

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

/**
 * ICE 收集状态
 * https://w3c.github.io/webrtc-pc/#dom-rtcicegatheringstate
 */
enum class IceGatheringState { New, Gathering, Complete };
std::string IceGatheringStateAsStr(IceGatheringState state);

/**
 * onIceConnectionChange
 */
enum class IceConnectionState {
    New,           //
    Checking,      //
    Connected,     //
    Completed,     //
    Failed,        //
    Disconnected,  //
    Closed,        //
    Max,           //
};

std::string IceConnectionStateAsStr(IceConnectionState state);

enum class PeerConnectionState {
    New,           //
    Connecting,    //
    Connected,     //
    Disconnected,  //
    Failed,        //
    Closed         //
};

std::string PeerConnectionStateAsStr(PeerConnectionState state);

/**
 * WebRTC信号状态
 * https://w3c.github.io/webrtc-pc/#dom-rtcsignalingstate
 */
enum SignalingState {
    Stable,              // 已经建立，或者初始状态
    HaveLocalOffer,      // 本地发起，调用setLocalDescription之后
    HaveLocalPrAnswer,   // 创建了本地应答，调用setLocalDescription()之后
    HaveRemoteOffer,     // 对方发起，调用setRemoteDescription之后
    HaveRemotePrAnswer,  // 对端的临时应答（pranswer），并成功地调用了setRemoteDescription()方法
    Closed,              // 表示RTCPeerConnection已经关闭。
};

std::string SignalingStateAsStr(SignalingState state);

/**
 * 视频帧
 */
struct RendererImage {
    size_t width_;
    size_t height_;
    uint8_t* y;       //
    uint8_t* u;       //
    uint8_t* v;       //
    int32_t ystride;  //
    int32_t ustride;  //
    int32_t vstride;  //
};

struct IceServer {
    std::string uri;
    std::string username;
    std::string password;

    [[nodiscard]] std::string toString() const {
        std::stringstream ss;
        ss << "{uri:" << uri << ",  username:" << username << ", password:" << password << "}";
        return ss.str();
    }
};

typedef struct join_options {
    std::string conference;
    std::string conference_id;
    std::string peer_name;
    long peer_id;
} JoinOptions;

class OkRTCHandler {
public:
    virtual void onCreatePeerConnection(const std::string& sId,
                                        const std::string& peerId,
                                        bool ok) = 0;

    virtual void onLocalDescriptionSet(const std::string& sId,
                                       const std::string& peerId,
                                       const OJingleContentMap* av) = 0;

    virtual void onFailure(const std::string& sId,
                           const std::string& peerId,
                           const std::string& error) = 0;

    virtual void onIceGatheringChange(const std::string& sId,
                                      const std::string& peerId,
                                      IceGatheringState state) = 0;

    virtual void onIceConnectionChange(const std::string& sId,
                                       const std::string& peerId,
                                       IceConnectionState state) = 0;

    virtual void onPeerConnectionChange(const std::string& sId,
                                        const std::string& peerId,
                                        PeerConnectionState state) = 0;

    virtual void onSignalingChange(const std::string& sId,
                                   const std::string& peerId,
                                   SignalingState state) = 0;

    virtual void onIce(const std::string& sId,
                       const std::string& peerId,
                       const OIceUdp& iceUdp) = 0;

    virtual void onRender(const RendererImage& image,
                          const std::string& peerId,
                          const std::string& resource) = 0;
};

/**
 * RTC 接口
 */
enum class Mode {
    p2p,
    meet,
};

/**
 * 控制状态
 */
struct CtrlState {
    bool enableMic;
    bool enableCam;
    bool enableSpk;
};

class OkRTC {
public:
    virtual ~OkRTC() = default;

    // 启动rtc实例
    virtual bool start() = 0;
    // 停止rtc实例
    virtual bool stop() = 0;

    virtual bool isStarted() = 0;

    virtual bool ensureStart() = 0;

    virtual void setIceServers(const std::vector<IceServer>& ices) = 0;

    virtual void addRTCHandler(OkRTCHandler* hand) = 0;
    virtual void removeRTCHandler(OkRTCHandler* hand) = 0;
    virtual const std::vector<OkRTCHandler*>& getHandlers() = 0;

    virtual bool CreateOffer(const std::string& peerId, const std::string& sId, bool video) = 0;

    virtual void CreateAnswer(const std::string& peerId, const OJingleContentMap& av) = 0;

    virtual void setRemoteDescription(const std::string& peerId, const OJingleContentMap& av) = 0;

    virtual void SessionTerminate(const std::string& peerId) = 0;

    virtual void setTransportInfo(const std::string& peerId,
                                  const std::string& sId,
                                  const OIceUdp& oIceUdp) = 0;

    virtual void setEnable(CtrlState state) = 0;

    // 0-100
    virtual void setSpeakerVolume(uint32_t vol) = 0;

    virtual bool quit(const std::string& peerId) = 0;

    // 获取视频设备数量
    virtual size_t getVideoSize() = 0;

    virtual std::map<std::string, OIceUdp> getCandidates(const std::string& peerId) = 0;

    virtual std::unique_ptr<OJingleContentMap> getLocalSdp(const std::string& peerId) = 0;

    virtual void addSource(const std::string& peerId,
                           const std::map<std::string, OMeetSSRCBundle>& map) = 0;

    virtual void switchVideoDevice(const std::string& deviceId) = 0;

    virtual void switchVideoDevice(int selected) = 0;

    virtual std::vector<std::string> getVideoDeviceList() = 0;
};

}  // namespace lib::ortc
