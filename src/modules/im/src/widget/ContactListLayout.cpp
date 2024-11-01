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

#include "ContactListLayout.h"
#include <cassert>
#include "ContactListWidget.h"
#include "friendwidget.h"
#include "genericchatitemlayout.h"
#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/model/status.h"

ContactListLayout::ContactListLayout(QWidget* parent) : QVBoxLayout(parent) {
    itemLayout = new GenericChatItemLayout(parent);
    setSpacing(0);
    setMargin(0);

    itemLayout->getLayout()->setSpacing(0);
    itemLayout->getLayout()->setMargin(0);
    addLayout(itemLayout->getLayout());
}

ContactListLayout::~ContactListLayout() {}

void ContactListLayout::removeFriendWidget(FriendWidget* widget) {
    itemLayout->removeSortedWidget(widget);
}

int ContactListLayout::indexOfFriendWidget(GenericChatItemWidget* widget, bool online) const {
    return itemLayout->indexOfSortedWidget(widget);
}

int ContactListLayout::friendOnlineCount() const { return itemLayout->getLayout()->count(); }

int ContactListLayout::friendTotalCount() const { return friendOnlineCount(); }

void ContactListLayout::search(const QString& searchString) { itemLayout->search(searchString); }

QLayout* ContactListLayout::getLayoutOnline() const { return itemLayout->getLayout(); }

void ContactListLayout::addWidget(GenericChatItemWidget* w) {
    w->setVisible(true);
    itemLayout->addSortedWidget(w);
}
