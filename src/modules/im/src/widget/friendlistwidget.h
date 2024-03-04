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

#ifndef FRIENDLISTWIDGET_H
#define FRIENDLISTWIDGET_H

#include "genericchatitemlayout.h"
#include "src/core/core.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include <QWidget>

class QVBoxLayout;
class QGridLayout;
class QPixmap;
class Widget;
class FriendWidget;
class GroupWidget;
class CircleWidget;
class FriendListLayout;
class GenericChatroomWidget;
class CategoryWidget;
class Friend;

class FriendListWidget : public QWidget
{
    Q_OBJECT
public:
    using SortingMode = Settings::FriendListSortingMode;
    explicit FriendListWidget(Widget* parent, bool groupsOnTop = true);
    ~FriendListWidget();
    void setMode(SortingMode mode);
    SortingMode getMode() const;

    void addGroupWidget(GroupWidget* widget);
    void addFriendWidget(FriendWidget* w, Status::Status s, int circleIndex);
    void removeGroupWidget(GroupWidget* w);
    void removeFriendWidget(FriendWidget* w);
    void addCircleWidget(int id);
    void addCircleWidget(FriendWidget* widget = nullptr);
    void removeCircleWidget(CircleWidget* widget);
    void searchChatrooms(const QString& searchString, bool hideOnline = false,
                         bool hideOffline = false, bool hideGroups = false);

    void cycleContacts(GenericChatroomWidget* activeChatroomWidget, bool forward);

    void updateActivityTime(const QDateTime& date);
    void reDraw();

signals:
    void onCompactChanged(bool compact);
    void connectCircleWidget(CircleWidget& circleWidget);
    void searchCircle(CircleWidget& circleWidget);

public slots:
    void renameGroupWidget(GroupWidget* groupWidget, const QString& newName);
    void renameCircleWidget(CircleWidget* circleWidget, const QString& newName);
    void onFriendWidgetRenamed(FriendWidget* friendWidget);
    void onGroupchatPositionChanged(bool top);
    void moveWidget(FriendWidget* w, Status::Status s, bool add = false);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void dayTimeout();

private:
    CircleWidget* createCircleWidget(int id = -1);
    QLayout* nextLayout(QLayout* layout, bool forward) const;
    void moveFriends(QLayout* layout);
    CategoryWidget* getTimeCategoryWidget(const Friend* frd) const;
    void sortByMode(SortingMode mode);

    SortingMode mode;
    bool groupsOnTop;
    FriendListLayout* listLayout;
    GenericChatItemLayout* circleLayout = nullptr;
    GenericChatItemLayout groupLayout;
    QVBoxLayout* activityLayout = nullptr;
    QTimer* dayTimer;
};

#endif // FRIENDLISTWIDGET_H
