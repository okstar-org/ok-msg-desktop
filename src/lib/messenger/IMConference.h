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
#include "base/jid.h"

namespace lib::messenger {

class IM;

struct Conference {
    ok::base::Jid jid;
    QString uid;
    uint32_t startAudioMuted;
    uint32_t startVideoMuted;
    bool rtcstatsEnabled;
};

class IMConference : public QObject {
    Q_OBJECT
public:
    explicit IMConference(IM* im, QObject* parent = nullptr);
    ~IMConference();
    const Conference& create(const QString& name);

private:
    IM* im;
    std::unique_ptr<Conference> conference;
};
}  // namespace lib::messenger
