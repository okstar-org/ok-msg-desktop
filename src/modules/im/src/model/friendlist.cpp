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
#include <QHash>
#include <QMenu>
#include "src/core/FriendId.h"
#include "src/model/friend.h"
#include "src/persistence/settings.h"

FriendList::FriendList(QObject* parent) : QObject(parent) {}

FriendList::~FriendList() { clear(); }

Friend* FriendList::addFriend(const FriendInfo& friendInfo) {
    qDebug() << __func__ << "friendInfo:" << friendInfo.toString();

    auto frnd = findFriend(friendInfo.id);
    if (frnd) {
        qWarning() << "friend:" << friendInfo.toString() << "is existing";
        return frnd;
    }

    auto* newfriend = new Friend(friendInfo.id, friendInfo.isFriend(), friendInfo.getAlias(), {});
    friendMap[((ContactId&)friendInfo).toString()] = newfriend;

    emit friendAdded(newfriend);

    //  if(friendInfo.resource.isEmpty()){
    //      newfriend->addEnd(friendInfo.resource);
    //  }
    return newfriend;
}

Friend* FriendList::findFriend(const ContactId& cId) {
    auto f = friendMap.value(cId.toString());
    if (!f) {
        f = new Friend(FriendId(cId));
        friendMap.insert(cId.toString(), f);
    }
    return f;
}

void FriendList::removeFriend(const FriendId& friendPk, bool fake) {
    auto f = findFriend(friendPk);
    if (f) {
        friendMap.remove(((ContactId&)friendPk).toString());
        f->deleteLater();
    }
}

void FriendList::clear() {
    for (auto friendptr : friendMap) delete friendptr;
    friendMap.clear();
}

QList<Friend*> FriendList::getAllFriends() { return friendMap.values(); }

QString FriendList::decideNickname(const FriendId& friendPk, const QString& origName) {
    Friend* f = FriendList::findFriend(friendPk);
    if (f != nullptr) {
        return f->getDisplayedName();
    } else if (!origName.isEmpty()) {
        return origName;
    } else {
        return friendPk.toString();
    }
}
