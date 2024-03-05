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

#ifndef FRIEND_H
#define FRIEND_H

#include "contact.h"
#include "src/core/contactid.h"
#include "src/core/core.h"
#include "src/core/toxid.h"
#include "src/model/status.h"
#include <QObject>
#include <QString>


class Friend : public Contact {
  Q_OBJECT
public:
  Friend(QString friendId,
         const ToxPk &friendPk,
         bool isFriend,
         const QString &userAlias = {},
         const QString &userName = {});

  Friend(const Friend &other) = delete;
  Friend &operator=(const Friend &other) = delete;

  void setName(const QString &name) override;
  void setAlias(const QString &name);
  QString getDisplayedName() const override;
  bool hasAlias() const;
  QString getUserName() const;
  void setStatusMessage(const QString &message);
  QString getStatusMessage() const;

  void setEventFlag(bool f) override;
  bool getEventFlag() const override;

  const ToxPk &getPublicKey() const;
  QString getId() const override;
  const ContactId &getPersistentId() const override;

  void setStatus(Status::Status s);
  Status::Status getStatus() const;

  bool isFriend()const {return  isFriend_;}
signals:
  void nameChanged(const ToxPk &friendId, const QString &name);
  void aliasChanged(const ToxPk &friendId, QString alias);
  void statusChanged(const ToxPk &friendId, Status::Status status);
  void onlineOfflineChanged(const ToxPk &friendId, bool isOnline);
  void statusMessageChanged(const ToxPk &friendId, const QString &message);
  void loadChatHistory();

public slots:

private:
  QString userName;
  QString userAlias;
  QString statusMessage;
  ToxPk friendPk;
  QString friendId;
  bool hasNewEvents;
  Status::Status friendStatus;
  bool isFriend_;
};

#endif // FRIEND_H
