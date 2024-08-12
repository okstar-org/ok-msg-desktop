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

#ifndef CONTACT_H
#define CONTACT_H

#include <QObject>
#include <QPixmap>
#include <QString>
#include "src/core/contactid.h"
#include "src/model/status.h"

/**
 * 联系人（朋友和群的基类）
 */
class Contact : public QObject {
    Q_OBJECT
public:
    Contact();
    Contact(const ContactId& id, const QString& name, const QString& alias = "",
            bool isGroup = false);
    ~Contact() override;

    bool isGroup() const { return group; }

    void setName(const QString& name);
    const QString& getName() const { return name; };

    void setAlias(const QString& name);
    const QString& getAlias() const { return alias; };
    bool hasAlias() const { return !alias.isEmpty(); }

    QString getDisplayedName() const;

    const ContactId& getPersistentId() const { return id; };
    QString getId() const { return id.toString(); };

    const QPixmap& setDefaultAvatar();
    void setAvatar(const QPixmap& pix);
    void clearAvatar();
    const QPixmap& getAvatar() const;

    virtual void setEventFlag(bool flag);
    virtual bool getEventFlag() const;

signals:
    // 用户名称
    void nameChanged(const QString& name);
    // 备注名称
    void aliasChanged(QString alias);
    // 显示名称（备注名》用户姓名》用户名）
    void displayedNameChanged(const QString& newName);
    // 头像
    void avatarChanged(const QPixmap& avatar);

protected:
    // 是否群聊
    bool group;
    // 联系人Id
    ContactId id;

    // 名称(nick)： https://xmpp.org/extensions/xep-0172.html
    QString name;
    // 别名(自己备注，即书签名称)https://xmpp.org/extensions/xep-0048.html
    QString alias;

    // 头像
    QPixmap avatar;
    Status::AvatarSet avatarSetStatus;
};

#endif  // CONTACT_H
