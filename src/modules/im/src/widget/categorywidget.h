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

#ifndef CATEGORYWIDGET_H
#define CATEGORYWIDGET_H

#include "genericchatitemwidget.h"
#include "src/core/core.h"
#include "src/model/status.h"

class FriendListLayout;
class FriendListWidget;
class FriendWidget;
class QVBoxLayout;
class QHBoxLayout;

class CategoryWidget : public GenericChatItemWidget {
    Q_OBJECT
public:
    explicit CategoryWidget(bool compact, QWidget* parent = nullptr);

    bool isExpanded() const;
    void setExpanded(bool isExpanded, bool save = true);
    void setName(const QString& name, bool save = true);

    void addFriendWidget(FriendWidget* w, Status::Status s);
    void removeFriendWidget(FriendWidget* w, Status::Status s);
    void updateStatus();

    bool hasChatrooms() const;
    bool cycleContacts(bool forward);
    bool cycleContacts(FriendWidget* activeChatroomWidget, bool forward);
    void search(const QString& searchString, bool updateAll = false, bool hideOnline = false,
                bool hideOffline = false);

public slots:
    void onCompactChanged(bool compact);
    void moveFriendWidgets(FriendListWidget* friendList);

protected:
    virtual void leaveEvent(QEvent* event) final override;
    virtual void mouseReleaseEvent(QMouseEvent* event) final override;

    void editName();
    void setContainerAttribute(Qt::WidgetAttribute attribute, bool enabled);
    QLayout* friendOnlineLayout() const;
    QLayout* friendOfflineLayout() const;
    void emitChatroomWidget(QLayout* layout, int index);

private:
    virtual void onSetName() {}
    virtual void onExpand() {}
    virtual void onAddFriendWidget(FriendWidget*) {}

    QWidget* listWidget;
    FriendListLayout* listLayout;
    QVBoxLayout* fullLayout;
    QVBoxLayout* mainLayout = nullptr;
    QHBoxLayout* topLayout = nullptr;
    QLabel* statusLabel;
    QWidget* container;
    QFrame* lineFrame;
    bool expanded = false;
};

#endif  // CATEGORYWIDGET_H
