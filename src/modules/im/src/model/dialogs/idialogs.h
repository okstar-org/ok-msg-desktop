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

#ifndef I_DIALOGS_H
#define I_DIALOGS_H

class ContactId;
class GroupId;
class FriendId;

class IDialogs {
public:
    virtual ~IDialogs() = default;
    virtual bool hasContact(const ContactId& contactId) const = 0;
    virtual bool isContactActive(const ContactId& contactId) const = 0;

    virtual void removeFriend(const FriendId& friendPk) = 0;
    virtual void removeGroup(const GroupId& groupId) = 0;

    virtual int chatroomCount() const = 0;
};

#endif  // I_DIALOGS_H
