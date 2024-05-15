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

#ifndef GROUP_CHATROOM_H
#define GROUP_CHATROOM_H

#include "chatroom.h"

#include <QObject>

class IDialogsManager;
class Group;
class ToxPk;

class GroupChatroom : public QObject, public Chatroom
{
    Q_OBJECT
public:
    GroupChatroom(const Group* group, IDialogsManager* dialogsManager);

   const Contact* getContact() override;

   const Group* getGroup();

    bool hasNewMessage() const;
    void resetEventFlags();

    bool friendExists(const ToxPk& pk);
    void inviteFriend(const ToxPk& pk);




private:
   const Group* group{nullptr};
    IDialogsManager* dialogsManager{nullptr};
};

#endif /* GROUP_CHATROOM_H */
