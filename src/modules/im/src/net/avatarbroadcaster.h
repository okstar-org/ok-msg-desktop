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

#ifndef AVATARBROADCASTER_H
#define AVATARBROADCASTER_H

#include <QByteArray>
#include <QMap>

class AvatarBroadcaster {
private:
  AvatarBroadcaster() = delete;

public:
  static void setAvatar(QByteArray data);
  static void sendAvatarTo(QString friendId);
  static void enableAutoBroadcast(bool state = true);

private:
  static QByteArray avatarData;
  static QMap<QString, bool> friendsSentTo;
};

#endif // AVATARBROADCASTER_H
