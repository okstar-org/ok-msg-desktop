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

#include "src/core/FriendId.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/dialogs/idialogsmanager.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"

GroupChatroom::GroupChatroom(const GroupId* groupId_, IDialogsManager* dialogsManager)
        : groupId{groupId_}, dialogsManager{dialogsManager} {}

GroupChatroom::~GroupChatroom() { qDebug() << __func__; }

const ContactId& GroupChatroom::getContactId() { return *groupId; }

bool GroupChatroom::hasNewMessage() const { return false; }

void GroupChatroom::resetEventFlags() {
    //    group->setEventFlag(false);
    //    group->setMentionedFlag(false);
}

bool GroupChatroom::friendExists(const FriendId& pk) {
    return FriendList::findFriend(pk) != nullptr;
}

void GroupChatroom::inviteFriend(const FriendId& pk) {
    const Friend* frnd = FriendList::findFriend(pk);
    const auto friendId = frnd->getId();

    const auto canInvite = Status::isOnline(frnd->getStatus());

    if (canInvite) {
        //        Core::getInstance()->groupInviteFriend(friendId.toString(), groupId->getId());
    }
}
