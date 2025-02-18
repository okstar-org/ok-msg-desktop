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

#include "contact.h"
#include <QVariant>
#include "base/images.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"

namespace module::im {

Contact::Contact() {}

Contact::Contact(const ContactId& id_, const QString& name_, const QString& alias_, bool isGroup_)
        : id(id_), name{name_}, alias{alias_}, group(isGroup_), avatarSetStatus{AvatarSet::None} {
    auto profile = Nexus::getProfile();

    uint dbId = profile->addContact(id);
    qDebug() << __func__ << "Add contact" << id.toString() << " saved to db dbId=>" << dbId;

    //    auto alias0 = profile->getFriendAlias(id.toString());
    //    if(!alias0.isEmpty()){
    //        alias = alias0;
    //    }

    // 此时构造函数还未走完，信号触发应该也不存在连接
    reloadAvatar();
}

Contact::~Contact() = default;

QString Contact::getDisplayedName() const {
    if (!alias.isEmpty()) {
        return alias;
    }
    if (!name.isEmpty()) {
        return name;
    }
    return id.username;
}

const QPixmap& Contact::setDefaultAvatar() {
    auto name = !group ? "contact" : "group";
    auto uri = QString(":img/%1_dark.svg").arg(name);
    avatar = QPixmap(uri);
    avatarSetStatus = AvatarSet::DefaultSet;
    return avatar;
}

void Contact::setAvatar(const QByteArray& avatar) {
    QPixmap pix;
    ok::base::Images::putToPixmap(avatar, pix);
    setAvatar(pix);
}

void Contact::setAvatar(const QPixmap& pix) {
    if (pix.isNull()) {
        return;
    }

    avatar = pix;
    avatarSetStatus = AvatarSet::UserSet;

    // save to profile
    //    auto profile = Nexus::getProfile();
    //    QByteArray buf;
    //    avatar.save(buf);
    //    profile->saveFriendAvatar(FriendId{id}, buf);

    emit avatarChanged(avatar);
}

void Contact::reloadAvatar() {
    auto profile = Nexus::getProfile();
    auto avt = profile->loadAvatarData(FriendId{id});
    if (!avt.isNull()) {
        avatar.loadFromData(avt);
        avatarSetStatus = AvatarSet::UserSet;
    } else {
        setDefaultAvatar();
    }
    emit avatarChanged(avatar);
}

void Contact::clearAvatar() {
    avatar = QPixmap{};
    emit avatarChanged(avatar);
}

const QPixmap& Contact::getAvatar() const {
    return avatar;
}

void Contact::setName(const QString& _name) {
    if (_name == name) {
        return;
    }

    name = _name;
    emit nameChanged(name);

    if (alias.isEmpty()) {
        emit displayedNameChanged(name);
    }
}

void Contact::setAlias(const QString& alias_) {
    qDebug() << __func__ << alias_;

    if (alias_.isEmpty()) {
        alias = QString();
        emit displayedNameChanged(getDisplayedName());
    } else {
        if (alias_ != getDisplayedName()) emit displayedNameChanged(alias_);
    }

    if (alias_ != alias) {
        alias = alias_;
        emit aliasChanged(alias);
    }
}

void Contact::setEventFlag(bool flag) {}

bool Contact::getEventFlag() const {
    return false;
}
}  // namespace module::im
