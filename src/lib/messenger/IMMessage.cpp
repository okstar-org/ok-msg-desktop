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

#include "IMMessage.h"
#include "base/basic_types.h"
#include <QDebug>
#include <QString>
#include <gloox.h>
#include <jid.h>
#include <rosteritem.h>

namespace lib::messenger {

FriendId::FriendId() {}

FriendId::FriendId(const QString &jid_) {
  gloox::JID jid(stdstring(jid_));
  username = qstring(jid.username());
  server = qstring(jid.server());
}

FriendId::FriendId(const gloox::JID &jid) {
  username = qstring(jid.username());
  server = qstring(jid.server());
}

FriendId::FriendId(const FriendId &f) : username(f.username), server(f.server) {}

bool FriendId::operator==(const QString &friendId) const { return toString() == friendId; }

bool FriendId::operator==(const FriendId &friendId) const { return friendId.username == username && friendId.server == server; }

bool FriendId::operator!=(const FriendId &friendId) const { return friendId.username != username && friendId.server != server; }

bool FriendId::operator<(const lib::messenger::FriendId &friendId) const {
  if (friendId.server.isEmpty()) {
    return username < friendId.username;
  }
  return username < friendId.username //
         && server < friendId.server;
}

PeerId::PeerId() = default;

PeerId::PeerId(const gloox::JID &jid) {
  username = qstring(jid.username());
  server = qstring(jid.server());
  resource = qstring(jid.resource());
}

PeerId::PeerId(const QString &peerId) {
  if (peerId.contains("@")) {
    auto jid = gloox::JID(peerId.toStdString());
    username = qstring(jid.username());
    server = qstring(jid.server());
    resource = qstring(jid.resource());
  } else {
    username = peerId;
  }
}

bool PeerId::operator==(const PeerId &peerId) const {
  return peerId.username == username     //
         && peerId.server == server      //
         && peerId.resource == resource; //
}

bool PeerId::operator==(const QString &username_) const {
  return username == username_; //
}

const std::string arr[9] = {
    "CONNECTING", "AUTH_FAILED", "CONNECTED", "DISCONNECTED", "TIMEOUT", "ERROR", "TLS_ERROR", "OUT_OF_RESOURCE", "NO_SUPPORT",
};

std::string IMStatusToString(IMStatus status) { return arr[(int)status]; }

std::ostream &operator<<(std::ostream &os, const Friend &f) {
  os << f.toString().toStdString();
  return os;
}

Friend::Friend(gloox::RosterItem *item) //
    : id{FriendId{qstring(item->jid().bare())}},  //
      name{qstring(item->name())},  //
      subscription{item->subscription()},//
      online{item->online()}, //
      groups{qstringlist(item->groups())}//
{}

Friend::Friend() {}

bool Friend::isFriend() const { return subscription == gloox::SubscriptionType::S10nBoth; }

QDebug &operator<<(QDebug &debug, const Friend &f) {
  QDebugStateSaver saver(debug);
  debug.nospace() << f.toString();
  return debug;
}


} // namespace lib::messenger

