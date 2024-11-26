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

//
// Created by gaojie on 24-11-23.
//

#pragma once

#include <QObject>
#include <memory>

#include <meethandler.h>
#include <meetmanager.h>

#include "base/jid.h"
#include "messenger.h"

namespace gloox {
class ConferenceManager;
}

namespace lib::messenger {

class IM;

class IMMeet : public QObject, public gloox::MeetHandler {
    Q_OBJECT
public:
    explicit IMMeet(IM* im, QObject* parent = nullptr);
    ~IMMeet();
    /**
     * 创建会议
     * @param name
     * @return
     */
    const Meet& create(const QString& name);

    /**
     * 解散会议
     */
    void disband();

    /**
     * 退出会议
     */
    void exit();

protected:
    void handleCreation(const gloox::JID& jid, bool ready,
                        const std::map<std::string, std::string>& props) override;

    void handleParticipant(const gloox::Meet::Participant& participant) override;

    void handleStatsId(const std::string& statsId) override;

private:
    IM* im;
    std::unique_ptr<Meet> conference;
    gloox::MeetManager* manager;
};
}  // namespace lib::messenger
