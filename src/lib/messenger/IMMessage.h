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

#pragma once

#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QString>

namespace gloox {
class JID;
class RosterItem;
}

namespace lib {
namespace messenger {

/**
 * @brief 连接状态
 *
 */
enum class IMStatus { CONNECTING, AUTH_FAILED, CONNECTED, DISCONNECTED, TIMEOUT, CONN_ERROR, TLS_ERROR, OUT_OF_RESOURCE, NO_SUPPORT };

std::string IMStatusToString(IMStatus status);

enum class MsgType {
  Chat = 1,
  Groupchat = 4,
};

struct IMMessage {
  MsgType type;
  QString id;
  QString from;
  QString to;
  QString body;
  QDateTime timestamp;
};

struct SelfInfo {
  QString nickname;
  QByteArray selfAvatar;
};

struct FriendId {
  /**
   * [username]@[server]
   */
  QString username;
  QString server;
  bool operator==(const QString &friendId) const;
  bool operator==(const FriendId &friendId) const;
  bool operator!=(const FriendId &friendId) const;
  bool operator<(const FriendId &friendId) const;

  FriendId();
  FriendId(const FriendId &);
  FriendId(const QString &jid);
  FriendId(const gloox::JID &jid);

  [[nodiscard]] QString getUsername() const { return username; }
  [[nodiscard]] QString getServer() const { return server; }

  [[nodiscard]] QString toString() const {
    if (username.isEmpty()) {
      return {};
    }

    if (server.isEmpty()) {
      return username;
    }

    return username + "@" + server;
  }
};

struct PeerId : public FriendId {
  /**
   * [username]@[server]/[resource]
   */
  QString resource;

  PeerId();
  explicit PeerId(const QString &peerId);
  explicit PeerId(const gloox::JID &jid);
  bool operator==(const PeerId &peerId) const;
  bool operator==(const QString &username) const;

  [[nodiscard]] inline const QString toFriendId() const { return username + "@" + server; }

  [[nodiscard]] inline const QString toString() const {
    if (resource.isEmpty())
      return toFriendId();
    return toFriendId() + "/" + resource;
  }
};

class Friend {
public:
  Friend();
  explicit Friend(gloox::RosterItem *pItem);

  FriendId id;
  QString alias;
  int subscription;
  bool online;
  QStringList groups;

  //互相关注才是朋友
  [[nodiscard]] bool isFriend() const;

  [[nodiscard]] QString toString() const {
    return QString("{id: %1, alias: %2, subscription:%3, online:%4, groups:[%5]}")  //
            .arg(id.toString()).arg(alias).arg(subscription).arg(online).arg(groups.join(","));
  }

  friend std::ostream &operator<<(std::ostream &os, const Friend &f);
  friend QDebug& operator<<(QDebug& debug, const Friend &f);
};

} // namespace messenger
} // namespace lib
