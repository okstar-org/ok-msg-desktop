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

#ifndef _CONTENT_DIALOG_MANAGER_H_
#define _CONTENT_DIALOG_MANAGER_H_

#include "contentdialog.h"
#include "src/core/contactid.h"
#include "src/core/groupid.h"
#include "src/core/toxpk.h"
#include "src/model/dialogs/idialogsmanager.h"

#include <QObject>

/**
 * @breaf Manage all content dialogs
 */
class ContentDialogManager : public QObject, public IDialogsManager
{
    Q_OBJECT
public:
    ContentDialog* current();
    bool contactWidgetExists(const ContactId& groupId);
    void focusContact(const ContactId& contactId);
    void updateFriendStatus(const ToxPk& friendPk);
    void updateGroupStatus(const GroupId& friendPk);
    bool isContactActive(const ContactId& contactId);
    ContentDialog* getFriendDialog(const ToxPk& friendPk) const;
    ContentDialog* getGroupDialog(const GroupId& friendPk) const;

    IDialogs* getFriendDialogs(const ToxPk& friendPk) const;
    IDialogs* getGroupDialogs(const GroupId& groupId) const;

    void addFriendToDialog(const ToxPk& friendPx,
                                    ContentDialog* dialog,
                                    FriendChatroom* chatroom,
                                    GenericChatForm* form);

    ContentDialog* addGroupToDialog(const GroupId& groupId,
                                    ContentDialog* dialog,
                                    GroupChatroom* chatroom,
                                    GenericChatForm* form);

    void addContentDialog(ContentDialog& dialog);

    static ContentDialogManager* getInstance();

private slots:
    void onDialogClose();
    void onDialogActivate();

private:
    ContentDialog* focusDialog(const ContactId& id,
                               const QHash<const ContactId&, ContentDialog*>& list);

    ContentDialog* currentDialog = nullptr;

    QHash<const ContactId&, ContentDialog*> contactDialogs;

    static ContentDialogManager* instance;
};

#endif // _CONTENT_DIALOG_MANAGER_H_
