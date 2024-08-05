/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

#pragma once

#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QString>

namespace gloox {
class JID;
class RosterItem;
}  // namespace gloox

namespace lib::messenger {

enum class MsgType {
    Chat = 1,
    Groupchat = 4,
};

struct IMMessage {
    MsgType type;
    QString id;
    QString from;
    QString to;
    QString body;
    QDateTime timestamp;
};

}  // namespace lib::messenger
