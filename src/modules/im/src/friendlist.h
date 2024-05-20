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

#ifndef FRIENDLIST_H
#define FRIENDLIST_H

#include <cstdint>
#include <QMap>

#include <src/core/contactid.h>
#include <src/core/toxpk.h>

template <class T> class QList;
template <class A, class B> class QHash;
class Friend;
class QByteArray;
class QString;
class ToxPk;

using FriendMap = QMap<QString, Friend *>;

class FriendList {
public:
  static Friend *addFriend(const ToxPk &friendPk, bool isFriend);
  static Friend *findFriend(const ContactId &cId);
  static QList<Friend *> getAllFriends();
  static void removeFriend(const ToxPk &friendPk, bool fake = false);
  static void clear();
  static QString decideNickname(const ToxPk &friendPk, const QString &origName);

private:
  static FriendMap friendMap;
};

#endif // FRIENDLIST_H
