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

#include "friendlist.h"
#include "src/core/contactid.h"
#include "src/core/toxpk.h"
#include "src/model/friend.h"
#include "src/persistence/settings.h"
#include <QDebug>
#include <QHash>
#include <QMenu>

QHash<ToxPk, Friend *> FriendList::friendList;
QHash<QString, ToxPk> FriendList::id2key;

Friend *FriendList::addFriend(QString friendId, const ToxPk &friendPk, bool isFriend) {
  qDebug() << "addFriend friendId:" << friendId
           << "friendPk:" << friendPk.toString();

  auto friendChecker = friendList.find(friendPk);
  if (friendChecker != friendList.end()) {
    qWarning() << "sendFriendRequest: friendPk already taken";
  }

  QString alias = Settings::getInstance().getFriendAlias(friendPk);
  qDebug()<<"friendId:"<< friendId <<" alias:"<<alias;

  Friend *newfriend = new Friend(friendId, friendPk,isFriend, alias, friendPk.getUsername());

  friendList[friendPk] = newfriend;
  id2key[friendId] = friendPk;

  return newfriend;
}

Friend *FriendList::findFriend(const ToxPk &friendPk) {
  auto f_it = friendList.find(friendPk);
  if (f_it != friendList.end()) {
    return *f_it;
  }
  qWarning()<<"Can not find friend:" << friendPk.toString() <<" from friend list";
  return nullptr;
}

const ToxPk &FriendList::id2Key(QString friendId) {
  return id2key[friendId];
}

void FriendList::removeFriend(const ToxPk &friendPk, bool fake) {
  auto f_it = friendList.find(friendPk);
  if (f_it != friendList.end()) {
    if (!fake)
      Settings::getInstance().removeFriendSettings(
          f_it.value()->getPublicKey());
    friendList.erase(f_it);
  }
}

void FriendList::clear() {
  for (auto friendptr : friendList)
    delete friendptr;
  friendList.clear();
}

QList<Friend *> FriendList::getAllFriends() { return friendList.values(); }

QString FriendList::decideNickname(const ToxPk &friendPk,
                                   const QString &origName) {
  Friend *f = FriendList::findFriend(friendPk);
  if (f != nullptr) {
    return f->getDisplayedName();
  } else if (!origName.isEmpty()) {
    return origName;
  } else {
    return friendPk.toString();
  }
}
