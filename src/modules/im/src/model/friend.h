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
  Friend(const ToxPk &friendPk,
         bool isFriend,
         const QString &userAlias = {},
         const QString &userName = {});

  Friend(const Friend &other) = delete;
  Friend &operator=(const Friend &other) = delete;



  bool hasAlias() const;

  void setStatusMessage(const QString &message);
  QString getStatusMessage() const;

  void setEventFlag(bool f) override;
  bool getEventFlag() const override;

  const ToxPk getPublicKey() const{return ToxPk{ Contact::getId()};};


  void setStatus(Status::Status s);
  Status::Status getStatus() const;

  bool isFriend()const {return  isFriend_;}
  void addEnd(const QString& end){ends.append(end);}
signals:
  void nameChanged(const ToxPk &friendId, const QString &name);
  void aliasChanged(const ToxPk &friendId, QString alias);
  void statusChanged(Status::Status status);
  void onlineOfflineChanged( bool isOnline);
  void statusMessageChanged( const QString &message);
  void loadChatHistory();

public slots:

private:
  bool hasNewEvents;
  bool isFriend_;
  QString statusMessage;
  Status::Status friendStatus;
  QList<QString> ends;//终端列表
};

#endif // FRIEND_H
