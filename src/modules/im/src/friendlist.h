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

#include <QMap>
#include <cstdint>

#include <src/core/FriendId.h>
#include <src/core/contactid.h>
#include "src/model/message.h"

template <class T> class QList;
template <class A, class B> class QHash;
class Friend;
class QByteArray;
class QString;
class FriendId;

using FriendMap = QMap<QString, Friend*>;

class FriendList : public QObject {
    Q_OBJECT
public:
    explicit FriendList(QObject* parent = nullptr);
    ~FriendList() override;

    Friend* addFriend(const FriendInfo& friendInfo);
    Friend* findFriend(const ContactId& cId);
    QList<Friend*> getAllFriends();
    void removeFriend(const FriendId& friendPk, bool fake = false);
    void clear();
    QString decideNickname(const FriendId& friendPk, const QString& origName);

private:
    FriendMap friendMap;

signals:
    void friendAdded(Friend* f);
};

#endif  // FRIENDLIST_H
