#include "ContactWidget.h"
#include "Bus.h"
#include "application.h"
#include "friendlistwidget.h"
#include "src/lib/settings/style.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "ui_ContactWidget.h"
#include "widget.h"

#include <src/friendlist.h>
#include <src/grouplist.h>

#include <src/widget/form/addfriendform.h>
#include <src/widget/form/groupinviteform.h>

#include <QLabel>
#include <QStyle>

ContactWidget::ContactWidget(QWidget* parent)
        : MainLayout(parent), ui(new Ui::ContactWidget), addForm{nullptr} {
    ui->setupUi(this);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    // 右侧内容容器
    contentWidget = std::make_unique<QWidget>(this);
    contentWidget->setObjectName("ContactContentWidget");
    contentWidget->setContentsMargins(8, 8, 8, 8);
    contentLayout = std::make_unique<ContentLayout>(contentWidget.get());

    // 左侧朋友列表
    contactListWidget = new FriendListWidget(this, contentLayout.get(), false);
    contactListWidget->setGeometry(0, 0, 400, 400);
    contactListWidget->layout()->setAlignment(Qt::AlignTop | Qt::AlignVCenter);
    connect(contactListWidget,
            &FriendListWidget::deleteFriendWidget,
            this,
            &ContactWidget::do_friendDelete);

    ui->scrollAreaWidgetContents->setGeometry(0, 0, 200, 500);
    ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop | Qt::AlignVCenter);
    ui->scrollAreaWidgetContents->layout()->addWidget((QWidget*)contactListWidget);

    ui->mainSplitter->addWidget(contentWidget.get());
    ui->mainSplitter->setSizes(QList<int>() << 200 << 500);

    // ui->searchText->setPlaceholderText(tr("Search Contacts"));

    init();

    reloadTheme();
}

ContactWidget::~ContactWidget() {
    deinit();
    delete ui;
}

void ContactWidget::reloadTheme() {
    setStyleSheet(Style::getStylesheet("contact/ContactWidget.css"));
    // I don't know why
    QTimer::singleShot(0, this, [this]() {
        style()->unpolish(ui->searchText);
        style()->unpolish(ui->addBtn);
        style()->polish(ui->searchText);
        style()->polish(ui->addBtn);
        ui->searchText->updateGeometry();
    });

    ui->friendList->setStyleSheet(Style::getStylesheet("contact/ContactList.css"));
    contactListWidget->reloadTheme();
    contentLayout->reloadTheme();

    ui->friendList->setAutoFillBackground(false);
    ui->friendList->viewport()->setAutoFillBackground(false);
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

void ContactWidget::do_openAddForm() { makeAddForm()->showTo(getContentLayout()); }

void ContactWidget::init() {
    connect(ok::Application::Instance()->bus(),
            &ok::Bus::coreChanged,
            this,
            &ContactWidget::onCoreChanged);
    connect(ui->addBtn, &QPushButton::released, this, &ContactWidget::do_openAddForm);
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

void ContactWidget::connectToCore(Core* core) {
    qDebug() << __func__ << core;

    connect(core, &Core::started, [core, this]() {
        std::list<FriendInfo> fl;
        core->loadFriendList(fl);
        for (auto& fk : fl) {
            contactListWidget->addFriend(fk);
        }
        core->loadGroupList();
    });

    // 好友请求
    connect(core, &Core::friendRequestReceived, this, &ContactWidget::onFriendRequest);

    connect(core, &Core::friendAdded, this, &ContactWidget::onFriendAdded);

    connect(core, &Core::friendUsernameChanged, this, &ContactWidget::onFriendNickChanged);

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
        return;
    }
    contactListWidget->addFriend(frnd);
}

void ContactWidget::onFriendAvatarChanged(const FriendId& friendnumber, const QByteArray& avatar) {
    qDebug() << __func__ << "friend:" << friendnumber.toString() << avatar.size();
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

void ContactWidget::onFriendStatusChanged(const FriendId& friendPk, Status::Status status) {
    qDebug() << __func__ << friendPk.toString() << "status:" << (int)status;
    //  const auto &friendPk = FriendList::id2Key(friendPk);
    contactListWidget->setFriendStatus(friendPk, status);
    //  IMFriend *f = FriendList::findFriend(friendPk);
    //  if (!f) {
    //    qWarning() << "Unable to find friend" << friendPk;
    //    return;
    //  }
    //
    //  bool isActualChange = f->getStatus() != status;
    //
    //  FriendWidget *widget = // friendWidgets[f->getPublicKey()];
    //  contactListWidget->getFriend(f->getPublicKey());
    //  if (isActualChange) {
    //    if (!Status::isOnline(f->getStatus())) {
    //      contactListWidget->moveWidget(widget, Status::Status::Online);
    //    } else if (status == Status::Status::Offline) {
    //      contactListWidget->moveWidget(widget, Status::Status::Offline);
    //    }
    //  }
    //
    //  f->setStatus(status);

    //  if (widget->isActive()) {
    //    setWindowTitle(widget->getSubject());
    //  }
    //
    //    ContentDialogManager::getInstance()->updateFriendStatus(friendPk);
}

void ContactWidget::onFriendStatusMessageChanged(const FriendId& friendPk, const QString& message) {
    contactListWidget->setFriendStatusMsg(friendPk, message);

    //  IMFriend *f = FriendList::findFriend(friendPk);
    //  if (!f) {
    //    return;
    //  }
    //
    //  QString str = message;
    //  str.replace('\n', ' ').remove('\r').remove(QChar('\0'));
    //  f->setStatusMessage(str);
    //
    //  chatForms[friendPk]->setStatusMessage(str);
    //
    //  auto frd = contactListWidget->getFriend(friendPk);
    //  if(frd){
    //    frd->setStatusMsg(str);
    //  }
}

void ContactWidget::onFriendRequest(const FriendId& friendPk, const QString& message) {
    qDebug() << __func__ << friendPk.toString() << message;
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

void ContactWidget::do_friendDelete(const FriendId& friendPk) {
    qDebug() << __func__ << friendPk.toString();
    core->removeFriend(friendPk.toString());
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
    auto& settings = Settings::getInstance();

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
    auto& settings = Settings::getInstance();

    unsigned int unreadFriendRequests = settings.getUnreadFriendRequests();
    qDebug() << __func__ << "unreadFriendRequests" << unreadFriendRequests;
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
