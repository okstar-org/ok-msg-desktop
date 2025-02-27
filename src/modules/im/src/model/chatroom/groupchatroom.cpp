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

#include "groupchatroom.h"

#include "Bus.h"
#include "application.h"
#include "src/core/core.h"
#include "src/lib/session/profile.h"
#include "src/model/FriendId.h"
#include "src/model/dialogs/idialogsmanager.h"
#include "src/model/friend.h"
#include "src/model/friendlist.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
namespace module::im {

GroupChatroom::GroupChatroom(const GroupId* groupId_, IDialogsManager* dialogsManager)
        : groupId{groupId_}, dialogsManager{dialogsManager}, mProfile{nullptr} {
    connect(ok::Application::Instance()->bus(), &ok::Bus::profileChanged, this,
            [&](Profile* profile) { mProfile = profile; });
}

GroupChatroom::~GroupChatroom() {
    qDebug() << __func__;
}

const ContactId& GroupChatroom::getContactId() {
    return *groupId;
}

bool GroupChatroom::hasNewMessage() const {
    return false;
}

void GroupChatroom::resetEventFlags() {
    //    group->setEventFlag(false);
    //    group->setMentionedFlag(false);
}

bool GroupChatroom::friendExists(const FriendId& pk) {
    return Nexus::getCore()->getFriendList().findFriend(pk) != nullptr;
}

Friend* GroupChatroom::findFriend(const FriendId& pk) {
    return Nexus::getCore()->getFriendList().findFriend(pk);
}

void GroupChatroom::inviteFriend(const FriendId& pk) {
    const Friend* frnd = Nexus::getCore()->getFriendList().findFriend(pk);
    if (!frnd) {
        qWarning() << "Unable to find friend:" << pk.toString();
        return;
    }

    //    const auto friendId = frnd->getId();
    //    const auto canInvite = Status::isOnline(frnd->getStatus());
    //    if (canInvite) {
    //        Core::getInstance()->groupInviteFriend(friendId.toString(), groupId->getId());
    //    }
}
}  // namespace module::im