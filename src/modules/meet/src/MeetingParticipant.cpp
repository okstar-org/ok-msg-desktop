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

#include "MeetingParticipant.h"
#include "MeetingVideoRender.h"

#include <utility>
namespace module::meet {
/**
 * 会议成员
 * @param resource
 * @param email
 * @param nick
 * @param avatarUrl
 * @param jid
 */
MeetingParticipant::MeetingParticipant(QString resource,
                                       QString email,
                                       QString nick,
                                       std::string avatarUrl,
                                       const ok::base::Jid& jid)
        : resource(std::move(resource))
        , email(std::move(email))
        , nick(std::move(nick))
        , avatarUrl(std::move(avatarUrl))
        , jid(jid) {}

void MeetingParticipant::bindVideoRender(MeetingVideoRender* render) {
    std::lock_guard<std::mutex> guard(_mutex);
    _videoRender = render;
}

MeetingVideoRender* MeetingParticipant::videoRender() {
    std::lock_guard<std::mutex> guard(_mutex);
    return _videoRender ? _videoRender : EmptyVideoRender::instance();
}

// MeetingUser::MeetingUser(MeetingParticipant& part)
//         : MeetingParticipant(part.getEmail(), part.getNick(), part.getAvatarUrl(), part.getJid())
//         {}
// uint32_t MeetingUser::removeResource(const QString& res) {
//     resources.remove(res);
//     return resources.size();
// }

}  // namespace module::meet