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

#include <QSet>
#include <QString>
#include "base/jid.h"

namespace module::meet {

// 会议参会者，获取一些基本信息、视频、音频等
class MeetingParticipant {
public:
    explicit MeetingParticipant(const QString& email,
                                const QString& nick,
                                const std::string& avatarUrl,
                                const ok::base::Jid& jid);

    [[nodiscard]] const QString& getEmail() const {
        return email;
    }

    [[nodiscard]] const QString& getNick() const {
        return nick;
    }

    void setNick(const QString& nick_) {
        nick = nick_;
    }

    [[nodiscard]] const std::string& getAvatarUrl() const {
        return avatarUrl;
    }

    void setAvatarUrl(const std::string& a) {
        avatarUrl = a;
    }

    [[nodiscard]] const ok::base::Jid& getJid() const {
        return jid;
    }

private:
    // 一个帐号（用户）一个邮箱
    QString email;
    QString nick;
    std::string avatarUrl;

    // 同一个用户，不同终端(jid.resource)
    ok::base::Jid jid;
};

/**
 * 会议用户
 */
class MeetingUser : public MeetingParticipant {
public:
    explicit MeetingUser(MeetingParticipant& part);
    /***
     *
     * @param res
     * @return 剩余的resource数量
     */
    uint32_t removeResource(const QString& res);

private:
    QSet<QString> resources;
};

}  // namespace module::meet