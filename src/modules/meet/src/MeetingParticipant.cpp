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
namespace module::meet {
MeetingParticipant::MeetingParticipant(const QString& email,
                                       const QString& nick,
                                       const std::string& avatarUrl,
                                       const ok::base::Jid& jid)
        : email(email), nick(nick), avatarUrl(avatarUrl), jid(jid) {}

MeetingUser::MeetingUser(MeetingParticipant& part)
        : MeetingParticipant(part.getEmail(), part.getNick(), part.getAvatarUrl(), part.getJid()) {}
uint32_t MeetingUser::removeResource(const QString& res) {
    resources.remove(res);
    return resources.size();
}

}  // namespace module::meet