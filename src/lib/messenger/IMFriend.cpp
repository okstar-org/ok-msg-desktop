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
// Created by gaojie on 24-5-28.
//

#include "IMFriend.h"
#include <jid.h>
#include <rosteritem.h>
#include <rosteritemdata.h>
#include <QDebugStateSaver>
#include "base/basic_types.h"

namespace lib::messenger {

IMContactId::IMContactId(const QString& jid_) {
    gloox::JID jid(stdstring(jid_));
    username = qstring(jid.username());
    server = qstring(jid.server());
}

IMContactId::IMContactId(const gloox::JID& jid) {
    username = qstring(jid.username());
    server = qstring(jid.server());
}

IMContactId::IMContactId(const IMContactId& f) : username(f.username), server(f.server) {}

bool IMContactId::operator==(const QString& friendId) const { return toString() == friendId; }

bool IMContactId::operator==(const IMContactId& friendId) const {
    return friendId.username == username && friendId.server == server;
}

bool IMContactId::operator!=(const IMContactId& friendId) const {
    return friendId.username != username && friendId.server != server;
}

bool IMContactId::operator<(const lib::messenger::IMContactId& friendId) const {
    if (friendId.server.isEmpty()) {
        return username < friendId.username;
    }
    return username < friendId.username  //
           && server < friendId.server;
}

IMPeerId::IMPeerId() = default;

IMPeerId::IMPeerId(const gloox::JID& jid) {
    username = qstring(jid.username());
    server = qstring(jid.server());
    resource = qstring(jid.resource());
}

IMPeerId::IMPeerId(const QString& peerId) {
    assert(peerId.contains("@"));

    auto jid = gloox::JID(peerId.toStdString());
    username = qstring(jid.username());
    server = qstring(jid.server());
    resource = qstring(jid.resource());
}

bool IMPeerId::operator==(const IMPeerId& peerId) const {
    return peerId.username == username      //
           && peerId.server == server       //
           && peerId.resource == resource;  //
}

IMFriend::IMFriend(gloox::RosterItem* item)  //
        : id{IMContactId{qstring(item->jid().bare())}}
        , alias{qstring(item->name())}
        , subscription{item->subscription()}
        , online{item->online()}
        , groups{ok::base::qstringlist(item->groups())} {}

IMFriend::IMFriend() {}

bool IMFriend::isFriend() const { return subscription == gloox::SubscriptionType::S10nBoth; }

}  // namespace lib::messenger
