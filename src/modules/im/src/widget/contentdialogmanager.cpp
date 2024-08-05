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

#include "contentdialogmanager.h"

#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/widget/friendwidget.h"
#include "src/widget/groupwidget.h"

#include <tuple>

namespace {
void removeDialog(ContentDialog* dialog, QHash<const ContactId&, ContentDialog*>& dialogs) {
    for (auto it = dialogs.begin(); it != dialogs.end();) {
        if (*it == dialog) {
            it = dialogs.erase(it);
        } else {
            ++it;
        }
    }
}
}  // namespace

ContentDialogManager* ContentDialogManager::instance;

ContentDialog* ContentDialogManager::current() { return currentDialog; }

bool ContentDialogManager::contactWidgetExists(const ContactId& contactId) {
    const auto dialog = contactDialogs.value(contactId, nullptr);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->hasContact(contactId);
}

void ContentDialogManager::addFriendToDialog(const FriendId& friendPk,
                                             ContentDialog* dialog,
                                             FriendChatroom* chatroom,
                                             GenericChatForm* form) {
    dialog->addFriend(chatroom, form);

    ContentDialog* lastDialog = getFriendDialog(friendPk);
    if (lastDialog) {
        lastDialog->removeFriend(friendPk);
    }

    contactDialogs[friendPk] = dialog;
}

ContentDialog* ContentDialogManager::addGroupToDialog(const GroupId& groupId,
                                                      ContentDialog* dialog,
                                                      GroupChatroom* chatroom,
                                                      GenericChatForm* form) {
    dialog->addGroup(chatroom, form);

    ContentDialog* lastDialog = getGroupDialog(groupId);
    if (lastDialog) {
        lastDialog->removeGroup(groupId);
    }

    contactDialogs[groupId] = dialog;
    return lastDialog;
}

void ContentDialogManager::focusContact(const ContactId& contactId) {
    auto dialog = focusDialog(contactId, contactDialogs);
    if (dialog != nullptr) {
        dialog->focusContact(contactId);
    }
}

/**
 * @brief Focus the dialog if it exists.
 * @param id User Id.
 * @param list List with dialogs
 * @return ContentDialog if found, nullptr otherwise
 */
ContentDialog* ContentDialogManager::focusDialog(
        const ContactId& id, const QHash<const ContactId&, ContentDialog*>& list) {
    auto iter = list.find(id);
    if (iter == list.end()) {
        return nullptr;
    }

    ContentDialog* dialog = *iter;
    if (dialog->windowState() & Qt::WindowMinimized) {
        dialog->showNormal();
    }

    dialog->raise();
    dialog->activateWindow();
    return dialog;
}

void ContentDialogManager::updateFriendStatus(const FriendId& friendPk) {
    auto dialog = contactDialogs.value(friendPk);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateContactStatusLight(friendPk);
    if (dialog->isContactActive(friendPk)) {
        dialog->updateTitleAndStatusIcon();
    }

    Friend* f = FriendList::findFriend(friendPk);
    dialog->updateFriendStatus(friendPk, f->getStatus());
}

void ContentDialogManager::updateGroupStatus(const GroupId& groupId) {
    auto dialog = contactDialogs.value(groupId);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateContactStatusLight(groupId);
    if (dialog->isContactActive(groupId)) {
        dialog->updateTitleAndStatusIcon();
    }
}

bool ContentDialogManager::isContactActive(const ContactId& contactId) {
    const auto dialog = contactDialogs.value(contactId);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->isContactActive(contactId);
}

ContentDialog* ContentDialogManager::getFriendDialog(const FriendId& friendPk) const {
    return contactDialogs.value(friendPk);
}

ContentDialog* ContentDialogManager::getGroupDialog(const GroupId& groupId) const {
    return contactDialogs.value(groupId);
}

ContentDialogManager* ContentDialogManager::getInstance() {
    if (instance == nullptr) {
        instance = new ContentDialogManager();
    }

    return instance;
}

void ContentDialogManager::addContentDialog(ContentDialog& dialog) {
    currentDialog = &dialog;
    connect(&dialog, &ContentDialog::willClose, this, &ContentDialogManager::onDialogClose);
    connect(&dialog, &ContentDialog::activated, this, &ContentDialogManager::onDialogActivate);
}

void ContentDialogManager::onDialogActivate() {
    ContentDialog* dialog = qobject_cast<ContentDialog*>(sender());
    currentDialog = dialog;
}

void ContentDialogManager::onDialogClose() {
    ContentDialog* dialog = qobject_cast<ContentDialog*>(sender());
    if (currentDialog == dialog) {
        currentDialog = nullptr;
    }

    removeDialog(dialog, contactDialogs);
}

IDialogs* ContentDialogManager::getFriendDialogs(const FriendId& friendPk) const {
    return getFriendDialog(friendPk);
}

IDialogs* ContentDialogManager::getGroupDialogs(const GroupId& groupId) const {
    return getGroupDialog(groupId);
}
