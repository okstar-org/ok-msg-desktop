/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

#ifndef FRIENDLISTWIDGET_H
#define FRIENDLISTWIDGET_H

#include <QWidget>
#include "genericchatitemlayout.h"
#include "src/core/core.h"
#include "src/model/friendmessagedispatcher.h"
#include "src/model/message.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"

class QVBoxLayout;
class QGridLayout;
class QPixmap;
class Widget;
class FriendWidget;
class GroupWidget;
class Core;
class FriendListLayout;
class GenericChatroomWidget;
class Friend;
class ContentLayout;
class MainLayout;

class FriendListWidget : public QWidget {
    Q_OBJECT
public:
    using SortingMode = Settings::FriendListSortingMode;
    explicit FriendListWidget(MainLayout* parent, ContentLayout* contentLayout,
                              bool groupsOnTop = true);
    ~FriendListWidget();
    void setMode(SortingMode mode);
    SortingMode getMode() const;
    void reloadTheme();

    FriendWidget* addFriend(const FriendInfo& friendInfo);
    FriendWidget* getFriend(const ContactId& friendPk);
    void removeFriend(const FriendId& cid);

    void setFriendStatus(const ContactId& friendPk, Status::Status status);
    void setFriendStatusMsg(const FriendId& friendPk, const QString& statusMsg);
    void setFriendName(const FriendId& friendPk, const QString& name);
    void setFriendAlias(const FriendId& friendPk, const QString& alias);
    void setFriendAvatar(const FriendId& friendPk, const QByteArray& avatar);
    void setFriendTyping(const FriendId& pk, bool typing);

    GroupWidget* addGroup(const GroupId& groupId, const QString& groupName = "");
    GroupWidget* getGroup(const GroupId& id);
    void removeGroup(const GroupId& groupId);
    void setGroupTitle(const GroupId& groupId, const QString& author, const QString& title);
    void setGroupInfo(const GroupId& groupId, const GroupInfo& info);

    //  void addCircleWidget(int id);
    //  void addCircleWidget(FriendWidget *widget = nullptr);
    //  void removeCircleWidget(CircleWidget *widget);
    void searchChatrooms(const QString& searchString, bool hideOnline = false,
                         bool hideOffline = false, bool hideGroups = false);

    void cycleContacts(GenericChatroomWidget* activeChatroomWidget, bool forward);

    void updateActivityTime(const QDateTime& date);
    void reDraw();

    void setRecvGroupMessage(const GroupMessage& msg);

signals:
    void deleteFriendWidget(const FriendId& friendPk);
    void deleteGroupWidget(const FriendId& friendPk);

public slots:
    void renameGroupWidget(GroupWidget* groupWidget, const QString& newName);

    void slot_friendClicked(GenericChatroomWidget*);
    void moveWidget(FriendWidget* w, Status::Status s, bool add = false);

    void onGroupchatPositionChanged(bool top);

    void slot_groupClicked(GenericChatroomWidget*);
    void do_toShowDetails(const ContactId& cid);
    void do_groupDeleted(const ContactId& cid);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onCoreChanged(Core& core);
    void dayTimeout();

private:
    QLayout* nextLayout(QLayout* layout, bool forward) const;

    void sortByMode(SortingMode mode);
    void connectFriendWidget(FriendWidget& friendWidget);
    void updateFriendActivity(const Friend& frnd);

    void connectToCore(Core* core);
    Core* core;

    SortingMode mode;

    bool groupsOnTop;
    FriendListLayout* listLayout;
    //  GenericChatItemLayout *circleLayout = nullptr;
    QVBoxLayout* activityLayout = nullptr;
    QTimer* dayTimer;

    ContentLayout* m_contentLayout;

    QMap<QString, FriendWidget*> friendWidgets;
    QMap<QString, GroupWidget*> groupWidgets;
    GenericChatItemLayout groupLayout;
};

#endif  // FRIENDLISTWIDGET_H
