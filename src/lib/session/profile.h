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

#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVector>
#include <memory>

#include "AuthSession.h"
#include "lib/messenger/IMFriend.h"
#include "lib/messenger/Messenger.h"
#include "lib/storage/StorageManager.h"

class Settings;
class QCommandLineParser;
namespace lib::session {

/**
 * 个人中心
 */
class Profile : public QObject {
    Q_OBJECT
public:
    explicit Profile(storage::StorageManager* sm, const AuthSession* authSession, QObject* parent = nullptr);
    ~Profile() override;

    // 获取用户名
    [[nodiscard]] const QString& getUsername() const;
    [[nodiscard]] const SignInInfo& getSignIn() const;

    //    bool isEncrypted() const;
    //    QString setPassword(const QString& newPassword);
    //    const ToxEncrypt* getPasskey() const;

    bool setAvatar(QByteArray& pic);
    bool removeAvatar();
    const QByteArray& getAvatar();

    QByteArray loadAvatarData(const ok::base::Jid& owner);
    //    QPixmap loadAvatar(const QString& owner);
    //    QString avatarPath(const QString& owner, bool forceUnencrypted = false);

    QByteArray getFriendAvatarHash(const ok::base::Jid& owner);
    bool removeFriendAvatar(const ok::base::Jid& owner);
    bool saveFriendAvatar(const ok::base::Jid& owner, const QByteArray& avatar);

    //    bool rename(QString newName);

    storage::StorageManager* create(const QString& profile);

    // profile
    QStringList getFilesByExt(const QString& extension);
    const QStringList& getAllProfileNames();
    bool exists();
    bool remove();

    /**
     * 保存昵称
     **/
    void setNickname(const QString& nickname_);

    [[nodiscard]] const QString& getNickname() const {
        return nickname;
    }

    [[nodiscard]] const QDir& getDir() const {
        assert(storageManager);
        return storageManager->getDir();
    }

    messenger::Messenger* getMessenger();

private:
    const AuthSession* authSession;

    bool encrypted = false;
    QStringList profiles;

    QString nickname;
    ok::base::Jid selfId;
    storage::StorageManager* storageManager;
    messenger::Messenger* messenger;
    QByteArray avatar;

signals:
    void selfAvatarChanged(const QByteArray& pixmap);

    // about active on default avatar.
    void friendAvatarChanged(const messenger::IMFriend& imFriend, const QPixmap& pixmap);
    // emit on a set of avatar, including identicon, used by those two care about
    // active for default, so can't use friendAvatarChanged
    void friendAvatarSet(const messenger::IMFriend& imFriend, const QPixmap& pixmap);
    // emit on set to default, used by those that modify on active
    void friendAvatarRemoved(const messenger::IMFriend& imFriend);

    void nickChanged(const QString& nick);

public slots:

    //    void onAvatarOfferReceived(const QString& friendId,
    //                               const QString &fileId,
    //                               const QByteArray& avatarHash);
};
}  // namespace lib::session
