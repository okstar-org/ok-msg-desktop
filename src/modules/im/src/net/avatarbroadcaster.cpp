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

#include "avatarbroadcaster.h"
#include "src/core/core.h"
#include "src/core/corefile.h"
#include "src/model/status.h"
#include <QDebug>
#include <QObject>

/**
 * @class AvatarBroadcaster
 *
 * Takes care of broadcasting avatar changes to our friends in a smart way
 * Cache a copy of our current avatar and friends who have received it
 * so we don't spam avatar transfers to a friend who already has it.
 */

QByteArray AvatarBroadcaster::avatarData;
QMap<QString, bool> AvatarBroadcaster::friendsSentTo;

static QMetaObject::Connection autoBroadcastConn;
static auto autoBroadcast = [](QString friendId, Status::Status) {
  AvatarBroadcaster::sendAvatarTo(friendId);
};

/**
 * @brief Set our current avatar.
 * @param data Byte array on avatar.
 */
void AvatarBroadcaster::setAvatar(QByteArray data) {
  if (avatarData == data)
    return;

  avatarData = data;
  friendsSentTo.clear();

//  QVector<QString> friends = Core::getInstance()->loadFriendList();
//  for (auto &friendId : friends)
//    sendAvatarTo(friendId);
}

/**
 * @brief Send our current avatar to this friend, if not already sent
 * @param friendId Id of friend to send avatar.
 */
void AvatarBroadcaster::sendAvatarTo(QString friendId) {
  if (friendsSentTo.contains(friendId) && friendsSentTo[friendId])
    return;
  if (!Core::getInstance()->isFriendOnline(friendId))
    return;
  CoreFile *coreFile = Core::getInstance()->getCoreFile();
  coreFile->sendAvatarFile(friendId, avatarData);
  friendsSentTo[friendId] = true;
}

/**
 * @brief Setup auto broadcast sending avatar.
 * @param state If true, we automatically broadcast our avatar to friends when
 * they come online.
 */
void AvatarBroadcaster::enableAutoBroadcast(bool state) {
//  QObject::disconnect(autoBroadcastConn);
//  if (state)
//    autoBroadcastConn = QObject::connect(
//        Core::getInstance(), &Core::friendStatusChanged, autoBroadcast);
}
