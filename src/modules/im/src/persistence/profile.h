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

//
// Created by gaojie on 25-1-9.
//

#pragma once

#include "history.h"
#include "lib/session/profile.h"
#include "lib/storage/db/rawdatabase.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/corefile.h"
#include "src/core/toxencrypt.h"

#include <QPixmap>
#include <memory>

/**
 * IM个人中心（包含Core）
 * 1、对个人帐号的抽象，一个用户登录即产生一个Profile
 * 2、包含基本帐号信息
 * 3、维护聊天核心Core
 */
class Profile : public QObject {
    Q_OBJECT
public:
    Profile(const QString& host, const QString& name, const QString& password, bool newProfile);

    static std::unique_ptr<Profile> createProfile(QString host, QString name, QString password);

    History* getHistory();

    const QPixmap loadAvatar();
    void setAvatar(QByteArray& pic, bool saveToCore);
    void setAvatarOnly(const QPixmap& pic);
    bool removeAvatar(bool saveToCore);

    QByteArray loadAvatarData(const ContactId& friendId);
    const QPixmap loadAvatar(const ContactId& friendId);

    const QString& getUsername();

    /**
     * 获取显示名（优先nick，再用户名）
     */
    const QString& getFullName();
    void setNick(const QString& nick_, bool saveToCore);

    void setVCard(const VCard& v);

    const VCard& getVCard() const {
        return vCard;
    }

    uint addContact(const ContactId& cid);
    void saveContactAlias(const QString& contactId, const QString& alias);
    QString getContactAlias(const QString& contactId);

    void initCore(ICoreSettings* s, bool isNewProfile);

    Core* getCore();

    void startCore();
    void stopCore();

    CoreAV* getCoreAv() {
        assert(coreAv.get());
        return coreAv.get();
    }

    CoreFile* getCoreFile() {
        assert(coreFile.get());
        return coreFile.get();
    }

    QString setPassword(const QString& pwd);
    QString getHost();

    [[nodiscard]] Settings* getSettings() const {
        return s;
    }

    void quit();

private:

    std::unique_ptr<Core> core;
    std::unique_ptr<CoreAV> coreAv;
    std::unique_ptr<CoreFile> coreFile;

    std::unique_ptr<ToxEncrypt> passkey = nullptr;
    std::shared_ptr<lib::db::RawDatabase> database;
    std::shared_ptr<History> history;

    VCard vCard;

    lib::session::Profile* _profile;
    lib::storage::StorageManager* storageManager;

    Settings* s;
    std::unique_ptr<lib::settings::OkSettings> okSettings;
    std::unique_ptr<lib::db::RawDatabase> db;
signals:
    void coreChanged(Core& core);
    // TODO(sudden6): this doesn't seem to be the right place for Core errors
    void failedToStart();
    void badProxy();

    void vCardChanged(const VCard& vCard);
    void selfAvatarChanged(const QPixmap& pixmap);
    void friendAvatarChanged(FriendId friendPk, const QPixmap& pixmap);
    void contactAliasChanged(QString cId, QString alias);

    void nickChanged(const QString& nick);

public slots:
    //    void onRequestSent(const FriendId& friendPk, const QString& message);
    void loadDatabase(QString password);
    void onSaveToxSave();

    void setFriendAvatar(const ContactId& owner, const QByteArray& pic);
    void removeFriendAvatar(const ContactId& owner);
};
