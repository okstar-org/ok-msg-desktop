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

#include <map>
#include <string>
#include <vector>

#include <jinglecontent.h>
#include <jinglefiletransfer.h>
#include <jinglegroup.h>
#include <jingleibb.h>
#include <jingleiceudp.h>
#include <jinglertp.h>
#include <jinglesession.h>

namespace lib {
namespace ortc {

#define SESSION_VERSION "3"

using namespace std;
using namespace gloox;
using namespace gloox::Jingle;

struct OIceUdp {
    std::string mid;
    int mline;
    std::string ufrag;
    std::string pwd;
    ICEUDP::Dtls dtls;
    ICEUDP::CandidateList candidates;
};

struct ORTP {
    Media media;
    RTP::PayloadTypes payloadTypes;
    RTP::HdrExts hdrExts;
    RTP::Sources sources;
    RTP::SsrcGroup ssrcGroup;
    bool rtcpMux;
};

struct OFile {
    Jingle::FileTransfer::FileList files;
    Jingle::IBB ibb;
};

struct OContent {
    std::string name;
    OFile file;
};

struct OSdp {
    std::string name;
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

    [[nodiscard]] inline JingleSdpType getSdpType() { return sdpType; }

    JingleSdpType sdpType;

    std::string sessionId;
    std::string sessionVersion;

    JingleCallType callType;

    virtual void toPlugins(PluginList& plugins) const;
    virtual void parse(const Jingle::Session::Jingle* jingle);
};

struct OJingleContentFile : public OJingleContent {
    void toPlugins(PluginList& plugins) const override;
    void parse(const Jingle::Session::Jingle* jingle) override;
    std::vector<OContent> contents;
};

struct OJingleContentAv : public OJingleContent {
public:
    void toPlugins(PluginList& plugins) const override;
    void parse(const Jingle::Session::Jingle* jingle) override;
    std::vector<OSdp> contents;

    inline bool isVideo() const {
        for (auto s : contents) {
            if (s.rtp.media == Jingle::Media::video) {
                return true;
            }
        }
        return false;
    }
};

}  // namespace ortc
}  // namespace lib
