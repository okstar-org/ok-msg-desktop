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

#ifndef FRIENDLISTLAYOUT_H
#define FRIENDLISTLAYOUT_H

#include "genericchatitemlayout.h"
#include "src/model/status.h"
#include "src/core/core.h"
#include <QBoxLayout>

class FriendWidget;
class FriendListWidget;

class FriendListLayout : public QVBoxLayout
{
    Q_OBJECT
public:
    explicit FriendListLayout();
    explicit FriendListLayout(QWidget* parent);

    void addFriendWidget(FriendWidget* widget, Status::Status s);
    void removeFriendWidget(FriendWidget* widget, Status::Status s);
    int indexOfFriendWidget(GenericChatItemWidget* widget, bool online) const;
    void moveFriendWidgets(FriendListWidget* listWidget);
    int friendOnlineCount() const;
    int friendTotalCount() const;

    bool hasChatrooms() const;
    void searchChatrooms(const QString& searchString, bool hideOnline = false,
                         bool hideOffline = false);

    QLayout* getLayoutOnline() const;
    QLayout* getLayoutOffline() const;

private:
    void init();
    QLayout* getFriendLayout(Status::Status s) const;

    GenericChatItemLayout friendOnlineLayout;
    GenericChatItemLayout friendOfflineLayout;
};

#endif // FRIENDLISTLAYOUT_H
