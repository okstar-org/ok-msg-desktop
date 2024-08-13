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

// void OJingleContent::parse(const Session::Jingle* jingle) {
//     sessionId = jingle->sid();
//     sessionVersion = SESSION_VERSION;
// }

// void OJingleContentFile::toPlugins(PluginList& plugins) const {}
//
// void OJingleContentFile::parse(const Jingle::Session::Jingle* jingle) {
//     OJingleContent::parse(jingle);
//     callType = JingleCallType::file;
//
//     for (const auto p : jingle->plugins()) {
//         Jingle::JinglePluginType pt = p->pluginType();
//         switch (pt) {
//             case JinglePluginType::PluginContent: {
//                 OContent oContent;
//                 const auto* content = static_cast<const Content*>(p);
//                 oContent.name = content->name();
//
//                 const auto* file = content->findPlugin<FileTransfer>(PluginFileTransfer);
//                 if (file) {
//                     OFile oFile = {file->files()};
//                     const auto* ibb = content->findPlugin<IBB>(PluginIBB);
//                     if (ibb) {
//                         oFile.ibb = *ibb;
//                     }
//                     oContent.file = oFile;
//                 }
//                 contents.emplace_back(oContent);
//                 break;
//             }
//             default:
//                 break;
//         }
//     }
// }
//
// void OJingleContentAv::toPlugins(PluginList& plugins) const {
//     //<group>
//     Jingle::Group::ContentList contentList;
//     for (auto& content : contents) {
//         auto name = content.name;
//         auto desc = content.rtp;
//
//         contentList.push_back(Jingle::Group::Content{name});
//
//         // description
//         Jingle::PluginList rtpPlugins;
//
//         // rtcp
//         auto rtp = new Jingle::RTP(desc.media, desc.payloadTypes);
//         rtp->setRtcpMux(desc.rtcpMux);
//
//         // payload-type
//         rtp->setPayloadTypes(desc.payloadTypes);
//
//         // rtp-hdrExt
//         rtp->setHdrExts(desc.hdrExts);
//
//         // source
//         if (!desc.sources.empty()) {
//             rtp->setSources(desc.sources);
//         }
//
//         // ssrc-group
//         if (!desc.ssrcGroup.ssrcs.empty()) {
//             rtp->setSsrcGroup(desc.ssrcGroup);
//         }
//
//         // rtp
//         rtpPlugins.emplace_back(rtp);
//
//         // transport
//         OIceUdp oIceUdp = content.iceUdp;
//         auto ice = new Jingle::ICEUDP(oIceUdp.pwd, oIceUdp.ufrag, oIceUdp.candidates);
//         ice->setDtls(oIceUdp.dtls);
//         rtpPlugins.emplace_back(ice);
//
//         auto* pContent = new Jingle::Content(name, rtpPlugins, Jingle::Content::CInitiator);
//         plugins.emplace_back(pContent);
//     }
//
//     auto group = new Jingle::Group("BUNDLE", contentList);
//     plugins.push_back(group);
// }
//
// void OJingleContentAv::parse(const Jingle::Session::Jingle* jingle) {
//     OJingleContent::parse(jingle);
//     callType = JingleCallType::av;
//
//     int mid = 0;
//     for (const auto p : jingle->plugins()) {
//         Jingle::JinglePluginType pt = p->pluginType();
//         switch (pt) {
//             case JinglePluginType::PluginContent: {
//                 OSdp oContent;
//
//                 const auto* content = static_cast<const Content*>(p);
//                 oContent.name = content->name();
//
//                 const auto* rtp = content->findPlugin<RTP>(PluginRTP);
//                 if (rtp) {
//                     ORTP description = {
//                             rtp->media(),         //
//                             rtp->payloadTypes(),  //
//                             rtp->hdrExts(),       //
//                             rtp->sources(),       //
//                             rtp->ssrcGroup(),     //
//                             rtp->rtcpMux()        //
//                     };
//                     oContent.rtp = description;
//                 }
//
//                 const auto* udp = content->findPlugin<ICEUDP>(PluginICEUDP);
//                 if (udp) {
//                     OIceUdp iceUdp = {
//                             std::to_string(mid),  //
//                             mid,                  //
//                             udp->ufrag(),         //
//                             udp->pwd(),           //
//                             udp->dtls(),          //
//                             udp->candidates()     //
//                     };
//                     oContent.iceUdp = iceUdp;
//                 }
//
//                 contents.emplace_back(oContent);
//                 break;
//             }
//             default:
//                 break;
//         }
//         mid++;
//     }
// }

}  // namespace ortc
}  // namespace lib
