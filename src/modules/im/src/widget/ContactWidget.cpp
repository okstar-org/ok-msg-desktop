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

#include "ContactWidget.h"
#include "ui_ContactWidget.h"

#include "Bus.h"
#include "ContactListWidget.h"
#include "application.h"
#include "lib/storage/settings/translator.h"
#include "src/lib/session/profile.h"
#include "src/lib/storage/settings/style.h"
#include "src/model/aboutfriend.h"
#include "src/nexus.h"
#include "src/widget/form/aboutfriendform.h"
#include "widget.h"

#include "src/model/friend.h"
#include "src/model/friendlist.h"
#include "src/model/group.h"
#include "src/model/grouplist.h"
#include "src/widget/form/addfriendform.h"
#include "src/widget/form/groupinviteform.h"
#include "src/widget/friendwidget.h"
#include "src/widget/groupwidget.h"

#include <QLabel>
#include <QStyle>
namespace module::im {
ContactWidget::ContactWidget(QWidget* parent)
        : MainLayout(parent), ui(new Ui::ContactWidget), addForm(nullptr) {
    ui->setupUi(this);

    layout()->setMargin(0);
    layout()->setSpacing(0);

    ui->searchContactsContainer->setAutoFillBackground(false);

    // 右侧内容容器
    contentWidget = new QWidget(this);
    contentWidget->setObjectName("ContactContentWidget");
    contentWidget->setContentsMargins(8, 8, 8, 8);
    contentLayout = new ContentLayout(contentWidget);

    // 左侧朋友列表
    contactListWidget = new ContactListWidget(this, false);
    contactListWidget->layout()->setAlignment(Qt::AlignTop | Qt::AlignVCenter);
    ui->friendList->setWidget(contactListWidget);

    // 点击事件 - 打开联系人详情
    connect(contactListWidget, &ContactListWidget::friendClicked, [&](const FriendWidget* w) {
        showFriendDetails(w->getFriend());
    });

    connect(contactListWidget, &ContactListWidget::groupClicked, [&](const GroupWidget* w) {
        showGroupDetails(w->getGroup());
    });

    ui->mainSplitter->addWidget(contentWidget);
    ui->mainSplitter->setSizes(QList<int>() << 240 << 500);
    ui->mainSplitter->setStretchFactor(1, 1);
    ui->mainSplitter->setChildrenCollapsible(false);

    ui->searchContact->setPlaceholderText(tr("Search Contacts"));
    connect(ui->searchContact, &QLineEdit::textChanged, this, &ContactWidget::searchContacts);

    ui->addBtn->setCursor(Qt::PointingHandCursor);
    connect(ui->addBtn, &QPushButton::released, this, &ContactWidget::do_openAddForm);

    connect(ok::Application::Instance()->bus(),
            &ok::Bus::coreChanged,
            this,
            &ContactWidget::onCoreChanged);

    connect(Widget::getInstance(), &Widget::friendRemoved, this, &ContactWidget::onFriendRemoved);
    connect(Widget::getInstance(), &Widget::addMember, this, &ContactWidget::do_addContactToGroup);

       auto a = ok::Application::Instance();
    connect(a->bus(), &ok::Bus::languageChanged,this,
            [&](const QString& locale0) {
                retranslateUi();
            });
}

ContactWidget::~ContactWidget() {
    qDebug() << __func__;
    deinit();
    delete ui;
}

void ContactWidget::reloadTheme() {
    setStyleSheet(lib::settings::Style::getStylesheet("contact/ContactWidget.css"));
    ui->friendList->setStyleSheet(lib::settings::Style::getStylesheet("contact/ContactList.css"));

    contactListWidget->reloadTheme();
    contentLayout->reloadTheme();
}

AddFriendForm* ContactWidget::makeAddForm() {
    if (!addForm) {
        addForm = new AddFriendForm(this);

        //        connect(addForm, &AddFriendForm::friendRequestsSeen, this,
        //                &ContactWidget::friendRequestsUpdate);
        connect(addForm, &AddFriendForm::friendRequested, this, &ContactWidget::do_friendRequest);
        connect(addForm,
                &AddFriendForm::friendRequestAccepted,
                this,
                &ContactWidget::do_friendRequestAccept);
        connect(addForm,
                &AddFriendForm::friendRequestRejected,
                this,
                &ContactWidget::do_friendRequestReject);
    }

    return addForm;
}

void ContactWidget::do_openAddForm() {
    makeAddForm()->showTo(getContentLayout());
}

void ContactWidget::deinit() {
    disconnect(ok::Application::Instance()->bus(),
               &ok::Bus::coreChanged,
               this,
               &ContactWidget::onCoreChanged);
    disconnect(ui->addBtn, &QPushButton::released, this, &ContactWidget::do_openAddForm);
}

void ContactWidget::onCoreChanged(Core* core_) {
    qDebug() << __func__ << &core_;
    core = core_;
    connectToCore(core);
}

void ContactWidget::onCoreStarted() {
    std::list<FriendInfo> fl;
    core->loadFriendList(fl);
    for (auto& friendInfo : fl) {
        contactListWidget->addFriend(friendInfo.getId());
    }
    core->loadGroupList();
}

void ContactWidget::connectToCore(Core* core) {
    qDebug() << __func__ << core;

    connect(core, &Core::started, this, &ContactWidget::onCoreStarted);

    // 好友请求
    connect(core, &Core::friendRequestReceived, this, &ContactWidget::onFriendRequest);
    connect(core, &Core::friendAdded, this, &ContactWidget::onFriendAdded);
    connect(core, &Core::friendNicknameChanged, this, &ContactWidget::onFriendNickChanged);
    connect(core, &Core::friendAvatarChanged, this, &ContactWidget::onFriendAvatarChanged);
    connect(core, &Core::friendAliasChanged, this, &ContactWidget::onFriendAliasChanged);
    connect(core, &Core::friendStatusChanged, this, &ContactWidget::onFriendStatusChanged);
    connect(core,
            &Core::friendStatusMessageChanged,
            this,
            &ContactWidget::onFriendStatusMessageChanged);

    connect(core, &Core::groupAdded, this, &ContactWidget::onGroupJoined);

    connect(core, &Core::groupInfoReceipt, this, &ContactWidget::onGroupInfoReceived);

    connect(core, &Core::groupSubjectChanged, this, &ContactWidget::onGroupSubjectChanged);
    connect(core, &Core::groupInviteReceived, this, &ContactWidget::onGroupInviteReceived);

    connect(core, &Core::groupPeerlistChanged, this, &ContactWidget::onGroupPeerListChanged);
    connect(core, &Core::groupPeerSizeChanged, this, &ContactWidget::onGroupPeerSizeChanged);
    connect(core, &Core::groupPeerNameChanged, this, &ContactWidget::onGroupPeerNameChanged);
    connect(core, &Core::groupPeerStatusChanged, this, &ContactWidget::onGroupPeerStatusChanged);
}

void ContactWidget::onFriendAdded(const FriendInfo& frnd) {
    qDebug() << __func__ << "friend:" << frnd.getId().toString();
    if (!frnd.getId().isValid()) {
        qWarning() << "Invalid friend id:" << frnd.getId();
        return;
    }
    contactListWidget->addFriend(frnd.getId());
}

void ContactWidget::onFriendRemoved(const Friend* f) {
    removeFriendDetails(f);
}

void ContactWidget::onFriendAvatarChanged(const FriendId& friendnumber, const QByteArray& avatar) {
    qDebug() << __func__ << "friend:" << friendnumber.toString() << "size:" << avatar.size();
    contactListWidget->setFriendAvatar(friendnumber, avatar);
}

void ContactWidget::onFriendAliasChanged(const FriendId& friendPk, const QString& alias) {
    qDebug() << __func__ << "friend:" << friendPk.toString() << "alias:" << alias;
    contactListWidget->setFriendAlias(friendPk, alias);
}

void ContactWidget::onFriendNickChanged(const FriendId& friendPk, const QString& nick) {
    qDebug() << __func__ << "friend:" << friendPk.toString() << "nick:" << nick;
    contactListWidget->setFriendName(friendPk, nick);
}

void ContactWidget::onFriendStatusChanged(const FriendId& friendPk, Status status) {
    contactListWidget->setFriendStatus(friendPk, status);
}

void ContactWidget::onFriendStatusMessageChanged(const FriendId& friendPk, const QString& message) {
    contactListWidget->setFriendStatusMsg(friendPk, message);
}

void ContactWidget::onFriendRequest(const FriendId& friendId, const QString& message) {
    qDebug() << __func__ << friendId.toString() << message;
    do_openAddForm();
    Widget::getInstance()->newMessageAlert(window(), isActiveWindow(), true, true);

#if DESKTOP_NOTIFICATIONS
    if (settings.getNotifyHide()) {
        notifier.notifyMessageSimple(DesktopNotify::MessageType::FRIEND_REQUEST);
    } else {
        notifier.notifyMessage(friendPk.toString() + tr(" sent you a friend request."), message);
    }
#endif
    //    }
}

void ContactWidget::do_friendRequest(const FriendId& friendPk,
                                     const QString& nick,
                                     const QString& message) {
    qDebug() << __func__ << friendPk.toString();
    core->requestFriendship(friendPk, nick, message);
}

void ContactWidget::do_friendRequestAccept(const FriendId& friendPk) {
    qDebug() << __func__ << friendPk.toString();
    core->acceptFriendRequest(friendPk);
}

void ContactWidget::do_friendRequestReject(const FriendId& friendPk) {
    qDebug() << __func__ << friendPk.toString();
    core->rejectFriendRequest(friendPk);
}

void ContactWidget::onGroupJoined(const GroupId& groupId, const QString& name) {
    qDebug() << __func__ << groupId.toString() << name;
    auto group = contactListWidget->addGroup(groupId, name);
    qDebug() << "Created group:" << group << "=>" << groupId.toString();
}

void ContactWidget::onGroupInfoReceived(const GroupId& groupId, const GroupInfo& info) {
    qDebug() << __func__ << groupId.toString();
    contactListWidget->setGroupInfo(groupId, info);
}

void ContactWidget::onGroupInviteReceived(const GroupInvite& inviteInfo) {
    auto confType = inviteInfo.getType();
    if (confType == ConferenceType::TEXT || confType == ConferenceType::AV) {
        if (false
            // settings.getAutoGroupInvite(f->getPublicKey())
        ) {
            onGroupInviteAccepted(inviteInfo);
        } else {
            //      if (!groupInviteForm->addGroupInvite(inviteInfo)) {
            //        return;
            //      }

            //      ++unreadGroupInvites;
            //      groupInvitesUpdate();
            //      Widget::getInstance()->newMessageAlert(window(), isActiveWindow(), true,
            //                                             true);

#if DESKTOP_NOTIFICATIONS
            if (settings.getNotifyHide()) {
                notifier.notifyMessageSimple(DesktopNotify::MessageType::GROUP_INVITE);
            } else {
                notifier.notifyMessagePixmap(
                        f->getDisplayedName() + tr(" invites you to join a group."),
                        {},
                        Nexus::getProfile()->loadAvatar(f->getPublicKey()));
            }
#endif
        }
    } else {
        qWarning() << "onGroupInviteReceived: Unknown ConferenceType:" << (int)confType;
        return;
    }
}

void ContactWidget::onGroupInviteAccepted(const GroupInvite& inviteInfo) {
    const QString groupId = core->joinGroupchat(inviteInfo);
    qDebug() << "onGroupInviteAccepted groupId=>" << groupId;

    if (groupId == std::numeric_limits<uint32_t>::max()) {
        qWarning() << "onGroupInviteAccepted: Unable to accept group invite";
        return;
    }
}

void ContactWidget::onGroupPeerListChanged(QString groupnumber) {
    const GroupId& groupId = GroupId(groupnumber);
    Group* g = GroupList::findGroup(groupId);
    assert(g);
    // g->regeneratePeerList();
}

void ContactWidget::onGroupPeerSizeChanged(QString groupnumber, const uint size) {
    const GroupId& groupId = GroupId(groupnumber);
    Group* g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "Can not find the group named:" << groupnumber;
        return;
    }

    g->setPeerCount(size);
}

void ContactWidget::onGroupPeerNameChanged(QString groupnumber,
                                           const FriendId& peerPk,
                                           const QString& newName) {
    const GroupId& groupId = GroupId(groupnumber);
    Group* g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "Can not find the group named:" << groupnumber;
        return;
    }
    //  const QString &setName = FriendList::decideNickname(peerPk, newName);
    //  g->updateUsername(peerPk, newName);
}

void ContactWidget::onGroupPeerStatusChanged(const QString& groupnumber, const GroupOccupant& go) {
    qDebug() << __func__ << "group" << groupnumber << "occupant:" << go.nick;

    Group* g = GroupList::findGroup(GroupId(groupnumber));
    if (!g) {
        qWarning() << "Can not find group named:" << groupnumber;
        return;
    }

    g->addPeer(go);
}

void ContactWidget::onGroupSubjectChanged(const GroupId& groupId, const QString& subject) {
    Group* g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "Can not find group" << groupId;
        return;
    }

    g->setSubject({}, subject);
}

// void ContactWidget::groupInvitesUpdate() {
//   if (unreadGroupInvites == 0) {
//     delete groupInvitesButton;
//     groupInvitesButton = nullptr;
//   } else if (!groupInvitesButton) {
//     groupInvitesButton = new QPushButton(this);
//     groupInvitesButton->setObjectName("green");
//     //    ui->statusLayout->insertWidget(2, groupInvitesButton);

//    connect(groupInvitesButton, &QPushButton::released, this,
//            &ContactWidget::onGroupClicked);
//  }

//  if (groupInvitesButton) {
//    groupInvitesButton->setText(
//        tr("%n New Group Invite(s)", "", unreadGroupInvites));
//  }
//}

// void ContactWidget::groupInvitesClear() {
//   unreadGroupInvites = 0;
//   groupInvitesUpdate();
// }

void ContactWidget::onGroupClicked() {
    //    auto settings = Nexus::getProfile()->getSettings();
    //    hideMainForms(nullptr);
    //  if (!groupInviteForm) {
    //    groupInviteForm = new GroupInviteForm;

    //    connect(groupInviteForm, &GroupInviteForm::groupCreate,
    //            core, &Core::createGroup);
    //  }
    //  groupInviteForm->show(contentLayout.get());
    //    setWindowTitle(fromDialogType(DialogType::GroupDialog));
    //    setActiveToolMenuButton(ActiveToolMenuButton::GroupButton);
}

void ContactWidget::friendRequestsUpdate() {
    //    auto& settings = Nexus::getProfile()->getSettings();

    //    unsigned int unreadFriendRequests = settings.getUnreadFriendRequests();
    //    qDebug() << __func__ << "unreadFriendRequests" << unreadFriendRequests;
    //  if (unreadFriendRequests == 0) {
    //    delete friendRequestsButton;
    //    friendRequestsButton = nullptr;
    //  } else if (!friendRequestsButton) {
    //    friendRequestsButton = new QPushButton(this);
    //    friendRequestsButton->setObjectName("green");
    //    connect(friendRequestsButton, &QPushButton::released, [this]() {
    ////    onAddClicked();
    //      addForm->setMode(AddFriendForm::Mode::FriendRequest);
    //    });
    //    ui->statusLayout->insertWidget(2, friendRequestsButton);
    //  }

    //  if (friendRequestsButton) {
    //    friendRequestsButton->setText(tr("%n New IMFriend Request(s)", "", unreadFriendRequests));
    //  }
}
void ContactWidget::retranslateUi() {
    ui->searchContact->setPlaceholderText(tr("Search Contacts"));
    ui->retranslateUi(this);
}

void ContactWidget::searchContacts() {
    QString text = ui->searchContact->text();
    contactListWidget->search(text);
}

void ContactWidget::showFriendDetails(const Friend* f) {
    qDebug() << __func__ << f->getIdAsString();
    removeAllDetails();

    friendAbout = std::make_unique<AboutFriendForm>(f);
    //    connect(about.get(), &AboutFriendForm::histroyRemoved, this,
    //    &FriendWidget::friendHistoryRemoved);
    contentLayout->addWidget(friendAbout.get());
    contentLayout->setCurrentWidget(friendAbout.get());
}

void ContactWidget::removeFriendDetails(const Friend* f) {
    if (friendAbout && friendAbout->getId() == f->getId()) {
        contentLayout->removeWidget(friendAbout.get());
        friendAbout.reset();
    }
}

void ContactWidget::showGroupDetails(const Group* g) {
    qDebug() << __func__ << g->getIdAsString();

    removeAllDetails();

    groupAbout = std::make_unique<AboutGroupForm>(g);
    contentLayout->addWidget(groupAbout.get());
    contentLayout->setCurrentWidget(groupAbout.get());
}

void ContactWidget::removeGroupDetails(const Group* g) {
    if (groupAbout && groupAbout->getId() == g->getId()) {
        contentLayout->removeWidget(groupAbout.get());
        groupAbout.reset();
    }
}

void ContactWidget::removeAllDetails() {
    if (groupAbout) {
        contentLayout->removeWidget(groupAbout.get());
        groupAbout.reset();
    }

    if (friendAbout) {
        contentLayout->removeWidget(friendAbout.get());
        friendAbout.reset();
    }
}

void ContactWidget::do_addContactToGroup(const ContactId& id, const ContactId& gId) {
    core->inviteToGroup(id, GroupId(gId));
}
}  // namespace module::im
