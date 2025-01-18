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

namespace lib::messenger {

IMContactId::IMContactId(const std::string& jid_) {
    gloox::JID jid((jid_));
    username = (jid.username());
    server = (jid.server());
}

IMContactId::IMContactId(const gloox::JID& jid) {
    username = (jid.username());
    server = (jid.server());
}

IMContactId::IMContactId(const IMContactId& f) : username(f.username), server(f.server) {}

bool IMContactId::operator==(const std::string& friendId) const {
    return toString() == friendId;
}

bool IMContactId::operator==(const IMContactId& friendId) const {
    return friendId.username == username && friendId.server == server;
}

bool IMContactId::operator!=(const IMContactId& friendId) const {
    return friendId.username != username && friendId.server != server;
}

bool IMContactId::operator<(const lib::messenger::IMContactId& friendId) const {
    if (friendId.server.empty()) {
        return username < friendId.username;
    }
    return username < friendId.username  //
           && server < friendId.server;
}

IMPeerId::IMPeerId() = default;

IMPeerId::IMPeerId(const gloox::JID& jid) {
    username = (jid.username());
    server = (jid.server());
    resource = (jid.resource());
}

IMPeerId::IMPeerId(const std::string& peerId) {
    auto jid = gloox::JID(peerId);
    username = (jid.username());
    server = (jid.server());
    resource = (jid.resource());
}

bool IMPeerId::operator==(const IMPeerId& peerId) const {
    return peerId.username == username      //
           && peerId.server == server       //
           && peerId.resource == resource;  //
}

IMFriend::IMFriend(gloox::RosterItem* item)  //
        : id{IMContactId{(item->jid().bare())}}
        , alias{(item->name())}
        , subscription{item->subscription()}
        , online{item->online()}
        , groups{(item->groups())} {}

IMFriend::IMFriend() {}

bool IMFriend::isFriend() const {
    return subscription == gloox::SubscriptionType::S10nBoth;
}

}  // namespace lib::messenger
