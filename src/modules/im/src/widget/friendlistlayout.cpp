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

#include "friendlistlayout.h"
#include <cassert>
#include "friendlistwidget.h"
#include "friendwidget.h"
#include "genericchatitemlayout.h"
#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/model/status.h"

FriendListLayout::FriendListLayout(QWidget* parent) : QVBoxLayout(parent) {
    friendOnlineLayout = new GenericChatItemLayout(parent);
    init();
}

FriendListLayout::~FriendListLayout() {}

void FriendListLayout::init() {
    setSpacing(0);
    setMargin(0);

    friendOnlineLayout->getLayout()->setSpacing(0);
    friendOnlineLayout->getLayout()->setMargin(0);
    addLayout(friendOnlineLayout->getLayout());
}

void FriendListLayout::addFriendWidget(FriendWidget* w, Status::Status s) {
    friendOnlineLayout->removeSortedWidget(w);
    friendOnlineLayout->addSortedWidget(w);
}

void FriendListLayout::removeFriendWidget(FriendWidget* widget) {
    friendOnlineLayout->removeSortedWidget(widget);
}

int FriendListLayout::indexOfFriendWidget(GenericChatItemWidget* widget, bool online) const {
    return friendOnlineLayout->indexOfSortedWidget(widget);
}

void FriendListLayout::moveFriendWidgets(FriendListWidget* listWidget) {
    while (!friendOnlineLayout->getLayout()->isEmpty()) {
        QWidget* getWidget = friendOnlineLayout->getLayout()->takeAt(0)->widget();

        FriendWidget* friendWidget = qobject_cast<FriendWidget*>(getWidget);
        const Friend* f = friendWidget->getFriend();
        listWidget->moveWidget(friendWidget, f->getStatus(), true);
    }
}

int FriendListLayout::friendOnlineCount() const { return friendOnlineLayout->getLayout()->count(); }

int FriendListLayout::friendTotalCount() const { return friendOnlineCount(); }

bool FriendListLayout::hasChatrooms() const { return !friendOnlineLayout->getLayout()->isEmpty(); }

void FriendListLayout::search(const QString& searchString) {
    friendOnlineLayout->search(searchString);
}

QLayout* FriendListLayout::getLayoutOnline() const { return friendOnlineLayout->getLayout(); }

void FriendListLayout::addWidget(GenericChatItemWidget* w) {
    w->setVisible(true);
    friendOnlineLayout->addSortedWidget(w);
}

QLayout* FriendListLayout::getFriendLayout(Status::Status s) const {
    return friendOnlineLayout->getLayout();
}
