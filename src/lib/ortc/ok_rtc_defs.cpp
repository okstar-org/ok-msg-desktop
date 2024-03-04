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

#include "ok_rtc_defs.h"

namespace lib {
namespace ortc {
JingleContext::JingleContext() {

};

JingleContext::JingleContext(JingleSdpType sdpType_,     //
                             const std::string &peerId_, //
                             const std::string &sId,      //
                             const std::string &sVer,     //
                             const PluginList &plugins)
    : sdpType(sdpType_), peerId(peerId_), sessionId(sId), sessionVersion(sVer) {
  fromJingleSdp(plugins);
}

JingleContext::JingleContext(lib::ortc::JingleSdpType sdpType_, //
                             const std::string &peerId_,        //
                             const std::string &sId,            //
                             const std::string &sVer,           //
                             const lib::ortc::JingleContents &contents_)
: sdpType(sdpType_), peerId(peerId_),
      sessionId(sId), sessionVersion(sVer),
      contents(contents_) {

}

void JingleContext::fromJingleSdp(const PluginList &plugins) {
    for (const auto p : plugins) {
      Jingle::JinglePluginType pt = p->pluginType();
      switch (pt) {
      case JinglePluginType::PluginContent: {

        OContent oContent;

        const auto *content = static_cast<const Content *>(p);
        oContent.name = content->name();

        const auto *file =
            content->findPlugin<FileTransfer>(PluginFileTransfer);
        if (file) {
          OFile oFile = {file->files()};
          const auto *ibb = content->findPlugin<IBB>(PluginIBB);
          if (ibb) {
            oFile.ibb = *ibb;
          }
          oContent.file = oFile;
        }

        const auto *rtp = content->findPlugin<RTP>(PluginRTP);
        if (rtp) {
          ORTP description = {
              rtp->media(),        //
              rtp->payloadTypes(), //
              rtp->hdrExts(),      //
              rtp->sources(),      //
              rtp->ssrcGroup(),    //
              rtp->rtcpMux()       //
          };
          oContent.rtp = description;
        }

        const auto *udp = content->findPlugin<ICEUDP>(PluginICEUDP);
        if (udp) {
          OIceUdp iceUdp = {
              "",
              0,
              udp->ufrag(),     //
              udp->pwd(),       //
              udp->dtls(),      //
              udp->candidates() //
          };
          oContent.iceUdp = iceUdp;
        }

        contents.emplace_back(oContent);
        break;
      }
      default:
        break;
      }
    }
  
}

} // namespace ortc
} // namespace lib