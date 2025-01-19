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

#ifndef CONTACTWIDGET_H
#define CONTACTWIDGET_H

#include <QPushButton>
#include <QWidget>
#include <memory>

#include "src/widget/form/aboutgroupform.h"

#include "ContactListWidget.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/model/status.h"
#include "src/widget/ContactListWidget.h"
#include "src/widget/MainLayout.h"
#include "src/widget/form/aboutfriendform.h"

namespace Ui {
class ContactWidget;
}
namespace module::im {
class FriendId;
class Core;
class GroupInviteForm;
class AddFriendForm;

/**
 * 通讯录界面
 * @brief ContactWidget::ContactWidget
 */
class ContactWidget : public MainLayout {
    Q_OBJECT

public:
    explicit ContactWidget(QWidget* parent = nullptr);
    ~ContactWidget();

    [[__nodiscard__]] ContentLayout* getContentLayout() const override {
        // MUST to be initialized.
        assert(contentLayout);
        return contentLayout.get();
    }

    void reloadTheme();
    void retranslateUi();

    void showFriendDetails(const Friend* f);
    void removeFriendDetails(const Friend* f);

    void showGroupDetails(const Group* g);
    void removeGroupDetails(const Group* g);
    void removeAllDetails();

public slots:
    void searchContacts();

    /**
     * do开头接受界面事件，调用core执行操作
     * on开头接受core事件，操作界面
     */
    void do_openAddForm();
    void do_addContactToGroup(const ContactId& id, const ContactId& gId);

    void onCoreChanged(Core* core);
    void onCoreStarted();
    void onFriendAdded(const FriendInfo& frnd);
    void onFriendRemoved(const Friend* f);
    void onFriendNickChanged(const FriendId& friendPk, const QString& nick);

    void onFriendStatusChanged(const FriendId& friendPk, Status status);
    void onFriendStatusMessageChanged(const FriendId& friendPk, const QString& message);

    void onFriendAvatarChanged(const FriendId& friendPk, const QByteArray& avatar);
    void onFriendAliasChanged(const FriendId& friendPk, const QString& alias);

    void onFriendRequest(const FriendId& friendId, const QString& message);

    // 朋友请求
    void do_friendRequest(const FriendId& friendAddress,
                          const QString& nick,
                          const QString& message);
    // 朋友请求接受
    void do_friendRequestAccept(const FriendId& friendPk);

    // 朋友请求拒绝
    void do_friendRequestReject(const FriendId& friendPk);

    void onGroupJoined(const GroupId& groupId, const QString& name);
    void onGroupInfoReceived(const GroupId& groupId, const GroupInfo& info);
    void onGroupInviteReceived(const GroupInvite& inviteInfo);

    void onGroupInviteAccepted(const GroupInvite& inviteInfo);

    void onGroupPeerListChanged(QString groupnumber);

    void onGroupPeerSizeChanged(QString groupnumber, const uint size);

    void onGroupPeerNameChanged(QString groupnumber, const FriendId& peerPk,
                                const QString& newName);

    void onGroupSubjectChanged(const GroupId&, const QString& subject);
    void onGroupPeerStatusChanged(const QString& groupnumber, const GroupOccupant& go);
    void onGroupClicked();

private:
    AddFriendForm* makeAddForm();
    void deinit();
    void connectToCore(Core* core);
    void friendRequestsUpdate();

    Ui::ContactWidget* ui;
    Core* core;
    ContactListWidget* contactListWidget;

    std::unique_ptr<AboutFriendForm> friendAbout;
    std::unique_ptr<AboutGroupForm> groupAbout;

    std::unique_ptr<QWidget> contentWidget;
    std::unique_ptr<ContentLayout> contentLayout;

    AddFriendForm* addForm;

    GroupInviteForm* groupInviteForm;
    uint32_t unreadGroupInvites;
    QPushButton* friendRequestsButton;
    QPushButton* groupInvitesButton;
};
}  // namespace module::im
#endif  // CONTACTWIDGET_H
