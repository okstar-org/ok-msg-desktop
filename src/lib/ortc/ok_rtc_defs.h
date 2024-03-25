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

#include <gloox/src/jinglecontent.h>
#include <gloox/src/jinglefiletransfer.h>
#include <gloox/src/jinglegroup.h>
#include <gloox/src/jingleibb.h>
#include <gloox/src/jingleiceudp.h>
#include <gloox/src/jinglertp.h>
#include <gloox/src/jinglesession.h>

namespace lib {
namespace ortc {

#define SESSION_VERSION "1"
using namespace std;
using namespace gloox;
using namespace gloox::Jingle;

struct OIceUdp {

  // key: mid
  typedef std::map<std::string, OIceUdp> OIceUdpMap;

  std::string mid;
  int mline;
  std::string ufrag;
  std::string pwd;
  ICEUDP::Dtls dtls;
  ICEUDP::CandidateList candidates;

  static inline OIceUdp fromJingle(const ICEUDP *ice) {
    return {"", 0, ice->ufrag(), ice->pwd(), ice->dtls(), ice->candidates()};
  }

  static void fromJingle(const PluginList &plugins, OIceUdpMap &iceUdpMap) {

    for (auto &p : plugins) {
      JinglePluginType pt = p->pluginType();
      if (pt == PluginContent) {
        auto c = static_cast<const Content *>(p);
        int x = 0;
        for (auto &i : c->plugins()) {
          if (i->pluginType() == PluginICEUDP) {
            auto ice = static_cast<const ICEUDP *>(i);
            OIceUdp oIceUdp = OIceUdp::fromJingle(ice);
            oIceUdp.mid = c->name();
            oIceUdp.mline = x;
            iceUdpMap.emplace(c->name(), oIceUdp);
            x++;
          }
        }
      }
    }
  }
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
  ORTP rtp;
  OIceUdp iceUdp;
  OFile file;
};

typedef std::list<OContent> JingleContents;

enum class JingleSdpType {
  Offer,
  Answer,
};

// 呼叫类型
enum JingleCallType {
  none,
  audio,    // audio
  video,    // video
  file      // file
};

inline static std::string JingleCallTypeToString (JingleCallType type){
  std::string arr[4]={"none", "audio", "video", "file"};
  return arr[type];
}

struct JingleContext {

public:

  JingleContext();

  JingleContext(JingleSdpType sdpType,
                const std::string &peerId,
                const std::string &sId,
                const std::string &sVer,
                const PluginList &plugins);

  JingleContext(JingleSdpType sdpType,
                const std::string &peerId,
                const std::string &sId,
                const std::string &sVer,
                const JingleContents& contents);
  /**
   * 获取呼叫类型
   * @return
   */
  inline JingleCallType callType() const {
    for (auto &c : contents) {
      if (!c.file.files.empty()) {
        return JingleCallType::file;
      }
      if (c.rtp.media == Media::video) {
        return JingleCallType::video;
      }
      if(c.rtp.media == Media::audio){
        return JingleCallType::audio;
      }
    }
    return JingleCallType::none;
  }

  inline bool hasVideo() { return callType() == JingleCallType::video; }

  inline const JingleContents& getContents() const {
    return contents;
  }

  [[nodiscard]] PluginList toJingleSdp() const {
    PluginList plugins;
    //<group>
    Jingle::Group::ContentList contentList;
    for (auto &content : contents) {
      auto name = content.name;
      auto desc = content.rtp;

      contentList.push_back(Jingle::Group::Content{name});

      // description
      Jingle::PluginList rtpPlugins;

      // rtcp
      auto rtp = new Jingle::RTP(desc.media, desc.payloadTypes);
      rtp->setRtcpMux(desc.rtcpMux);

      // payload-type
      rtp->setPayloadTypes(desc.payloadTypes);

      // rtp-hdrExt
      rtp->setHdrExts(desc.hdrExts);

      // source
      if (!desc.sources.empty()) {
        rtp->setSources(desc.sources);
      }

      // ssrc-group
      if (!desc.ssrcGroup.ssrcs.empty()) {
        rtp->setSsrcGroup(desc.ssrcGroup);
      }

      // rtp
      rtpPlugins.emplace_back(rtp);

      // transport
      OIceUdp oIceUdp = content.iceUdp;
      auto ice =
          new Jingle::ICEUDP(oIceUdp.pwd, oIceUdp.ufrag, oIceUdp.candidates);
      ice->setDtls(oIceUdp.dtls);
      rtpPlugins.emplace_back(ice);

      auto *pContent =
          new Jingle::Content(name, rtpPlugins, Jingle::Content::CInitiator);
      plugins.emplace_back(pContent);
    }

    auto group = new Jingle::Group("BUNDLE", contentList);
    plugins.push_back(group);
    return plugins;
  }

  [[nodiscard]] JingleSdpType getSdpType(){
    return sdpType;
  }


  JingleSdpType sdpType;
  std::string peerId;
  std::string sessionId;
  std::string sessionVersion;
  JingleContents contents;

  void fromJingleSdp(const PluginList &plugins);

};

} // namespace ortc
} // namespace lib
