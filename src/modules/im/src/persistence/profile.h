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

#ifndef PROFILE_H
#define PROFILE_H

#include "src/core/core.h"
#include "src/core/toxencrypt.h"
#include "src/core/toxid.h"

#include "src/persistence/history.h"

#include <QByteArray>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QVector>
#include <memory>

class Settings;
class QCommandLineParser;

class Profile : public QObject {
  Q_OBJECT

public:
  static Profile *loadProfile(QString name, //
                              const QCommandLineParser *parser,//
                              const QString &password = QString());
  static Profile *createProfile(QString name,//
                                const QCommandLineParser *parser,//
                                QString password);
  ~Profile();

  Core *getCore();
  QString getName() const;

  void startCore();
  bool isEncrypted() const;
  QString setPassword(const QString &newPassword);
  const ToxEncrypt *getPasskey() const;

  QPixmap loadAvatar();
  QPixmap loadAvatar(const ToxPk &owner);
  QByteArray loadAvatarData(const ToxPk &owner);
  void setAvatar(QByteArray pic);
  void setAvatarOnly(QPixmap pic);

  QByteArray getAvatarHash(const ToxPk &owner);
  void removeSelfAvatar();
  void removeFriendAvatar(const ToxPk &owner);
  bool isHistoryEnabled();
  History *getHistory();

  QStringList remove();

  bool rename(QString newName);

  static const QStringList getAllProfileNames();

  static bool exists(QString name);
  static bool isEncrypted(QString name);
  static QString getDbPath(const QString &profileName);

  void saveAvatar(const ToxPk &owner, const QByteArray &avatar);
  void saveFriendAlias(const QString& friendPk, const QString& alias);
  QString getFriendAlias(const QString& friendPk);

signals:
  void selfAvatarChanged(const QPixmap &pixmap);
  // emit on any change, including default avatar. Used by those that don't care
  // about active on default avatar.
  void friendAvatarChanged(const ToxPk &friendPk, const QPixmap &pixmap);
  // emit on a set of avatar, including identicon, used by those two care about
  // active for default, so can't use friendAvatarChanged
  void friendAvatarSet(const ToxPk &friendPk, const QPixmap &pixmap);
  // emit on set to default, used by those that modify on active
  void friendAvatarRemoved(const ToxPk &friendPk);
  // TODO(sudden6): this doesn't seem to be the right place for Core errors
  void failedToStart();
  void badProxy();
  void coreChanged(Core &core);

public slots:
  void onRequestSent(const ToxPk &friendPk, const QString &message);

private slots:
  void loadDatabase(QString password);
  void removeAvatar(const ToxPk &owner);
  void onSaveToxSave();
  // TODO(sudden6): use ToxPk instead of friendId
  void onAvatarOfferReceived(QString friendId, QString fileId,
                             const QByteArray &avatarHash);
  void setFriendAvatar(const ToxPk owner, const QByteArray &pic);

private:
  Profile(QString name, const QString &password, bool newProfile,
          const QByteArray &toxsave, std::unique_ptr<ToxEncrypt> passKey);
  static QStringList getFilesByExt(QString extension);
  QString avatarPath(const ToxPk &owner, bool forceUnencrypted = false);
  bool saveToxSave(QByteArray data);
  void initCore(const QByteArray &toxsave, ICoreSettings &s, bool isNewProfile);

private:
  std::unique_ptr<Core> core = nullptr;
  QString name;
  std::unique_ptr<ToxEncrypt> passkey = nullptr;
  std::shared_ptr<RawDatabase> database;
  std::shared_ptr<History> history;
  bool isRemoved;
  bool encrypted = false;
  static QStringList profiles;
};

#endif // PROFILE_H
