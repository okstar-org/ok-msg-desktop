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

FriendMap FriendList::friendMap;

Friend* FriendList::addFriend(const FriendInfo& friendInfo) {
    qDebug() << __func__ << "friendInfo:" << friendInfo.toString();

    auto frnd = findFriend(friendInfo.id);
    if (frnd) {
        qWarning() << "friend:" << friendInfo.toString() << "is existing";
        return frnd;
    }

    Friend* newfriend = new Friend(friendInfo.id, friendInfo.isFriend(), friendInfo.getAlias(), {});
    friendMap[((ContactId&)friendInfo).toString()] = newfriend;

    //  if(friendInfo.resource.isEmpty()){
    //      newfriend->addEnd(friendInfo.resource);
    //  }
    return newfriend;
}

Friend* FriendList::findFriend(const ContactId& cId) { return friendMap.value(cId.toString()); }

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
