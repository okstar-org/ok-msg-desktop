#include "ContactWidget.h"
#include "friendlistwidget.h"
#include "ui_ContactWidget.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"

#include <src/friendlist.h>
#include <src/grouplist.h>

#include <src/widget/form/groupinviteform.h>

#include <QLabel>


ContactWidget::ContactWidget(QWidget *parent) :
    MainLayout(parent),
    ui(new Ui::ContactWidget)
{
    ui->setupUi(this);
    layout()->setMargin(0);
    layout()->setSpacing(0);



  contentWidget = std::make_unique<QWidget>(this);
  contentWidget->setObjectName("contentWidget");
  contentLayout = std::make_unique< ContentLayout>(contentWidget.get());

  contactListWidget = std::make_unique<FriendListWidget>(this, false);
  contactListWidget->setGeometry(0, 0 , 400, 400);
  contactListWidget->layout()->setAlignment(Qt::AlignTop | Qt::AlignVCenter);

  auto split = ui->splitter;
  split->addWidget(contactListWidget.get());
  split->addWidget(contentWidget.get());
  split->setSizes(QList<int>() << 200 << 500);


    init();
}

ContactWidget::~ContactWidget()
{
    delete ui;
    deinit();
}


void ContactWidget::init() {
    connect(Nexus::getProfile(), &Profile::coreChanged,
            this, &ContactWidget::onCoreChanged);
}

void ContactWidget::deinit() {

    disconnect(Nexus::getProfile(), &Profile::coreChanged,
               this, &ContactWidget::onCoreChanged);
}


void ContactWidget::onCoreChanged(Core &core_) {
  core = &core_;
  connectToCore(core);

  auto fl = core->loadFriendList();
  for (auto fk : fl) {
    contactListWidget->addFriend(fk, true);
  }

  core->loadGroupList();

}

void ContactWidget::connectToCore(Core *core) {
  qDebug() << __func__<<"core"<<core;

  connect(core, &Core::friendAdded,
          this, &ContactWidget::onFriendAdded);

  connect(core, &Core::friendUsernameChanged,
          this, &ContactWidget::onFriendUsernameChanged);

  connect(core, &Core::friendAvatarChanged, this,
          &ContactWidget::onFriendAvatarChanged);

  connect(core, &Core::friendStatusChanged, this,
          &ContactWidget::onFriendStatusChanged);
  connect(core, &Core::friendStatusMessageChanged, this,
          &ContactWidget::onFriendStatusMessageChanged);
  connect(core, &Core::friendRequestReceived, this,
          &ContactWidget::onFriendRequestReceived);



  connect(core, &Core::groupAdded, this, &ContactWidget::onGroupJoined);

  connect(core, &Core::groupInfoReceipt, this, &ContactWidget::onGroupInfoReceived);

  connect(core, &Core::groupTitleChanged, this,
          &ContactWidget::onGroupTitleChanged);

  connect(core, &Core::groupInviteReceived, this,
          &ContactWidget::onGroupInviteReceived);

  connect(core, &Core::groupPeerlistChanged, this,
          &ContactWidget::onGroupPeerListChanged);
  connect(core, &Core::groupPeerSizeChanged, this,
          &ContactWidget::onGroupPeerSizeChanged);
  connect(core, &Core::groupPeerNameChanged, this,
          &ContactWidget::onGroupPeerNameChanged);
  connect(core, &Core::groupPeerStatusChanged, this,
          &ContactWidget::onGroupPeerStatusChanged);


}

void ContactWidget::onFriendAdded(const ToxPk &friendPk, bool isFriend) {
  qDebug() << __func__ << "friend:" << friendPk.toString();
  contactListWidget->addFriend(friendPk, isFriend);
}


void ContactWidget::onFriendAvatarChanged(const ToxPk &friendnumber,
                                       const QByteArray &avatar) {
  qDebug() << __func__ << "friend:" << friendnumber.toString() << avatar.size();
  contactListWidget->setFriendAvatar(friendnumber, avatar);
}

void ContactWidget::onFriendUsernameChanged(const ToxPk &friendPk,
                                         const QString &username) {
  qDebug() << __func__ << "friend:" << friendPk.toString()
           << "name:" << username;
//  contactListWidget->setFriendName(friendPk, username);
}



void ContactWidget::onFriendStatusChanged(const ToxPk &friendPk,
                                       Status::Status status) {
  qDebug() << __func__ << friendPk.toString() << "status:" << (int)status;
  //  const auto &friendPk = FriendList::id2Key(friendPk);
    contactListWidget->setFriendStatus(friendPk, status);
  //  Friend *f = FriendList::findFriend(friendPk);
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
  //  widget->updateStatusLight();
  //  if (widget->isActive()) {
  //    setWindowTitle(widget->getTitle());
  //  }
  //
//    ContentDialogManager::getInstance()->updateFriendStatus(friendPk);
}

void ContactWidget::onFriendStatusMessageChanged(const ToxPk &friendPk,
                                              const QString &message) {

  contactListWidget->setFriendStatusMsg(friendPk, message);

  //  Friend *f = FriendList::findFriend(friendPk);
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

void ContactWidget::onFriendRequestReceived(const ToxPk &friendPk,
                                         const QString &message) {
  qDebug() << __func__ << "friendId:" << friendPk.toString();

  //  if (addFriendForm->addFriendRequest(friendPk.toString(), message)) {
  //    friendRequestsUpdate();
  //    newMessageAlert(window(), isActiveWindow(), true, true);
  //
  // #if DESKTOP_NOTIFICATIONS
  //    if (settings.getNotifyHide()) {
  //      notifier.notifyMessageSimple(DesktopNotify::MessageType::FRIEND_REQUEST);
  //    } else {
  //      notifier.notifyMessage(
  //          friendPk.toString() + tr(" sent you a friend request."), message);
  //    }
  // #endif
  //  }
}

void ContactWidget::onGroupJoined(const GroupId &groupId, const QString &name) {
  qDebug() << __func__ << groupId.toString() << name;
  auto group = contactListWidget->addGroup(groupId, name);
  qDebug() << "Created group:" << group << "=>" << groupId.toString();
}

void ContactWidget::onGroupInfoReceived(const GroupId &groupId, const GroupInfo &info){
  qDebug() << __func__ << groupId.toString();
  contactListWidget->setGroupInfo(groupId, info);
}

void ContactWidget::onGroupInviteReceived(const GroupInvite &inviteInfo) {

  const uint8_t confType = inviteInfo.getType();
  if (confType == TOX_CONFERENCE_TYPE_TEXT ||
      confType == TOX_CONFERENCE_TYPE_AV) {
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
            f->getDisplayedName() + tr(" invites you to join a group."), {},
            Nexus::getProfile()->loadAvatar(f->getPublicKey()));
      }
#endif
    }
  } else {
    qWarning() << "onGroupInviteReceived: Unknown groupchat type:" << confType;
    return;
  }
}

void ContactWidget::onGroupInviteAccepted(const GroupInvite &inviteInfo) {
  const QString groupId = core->joinGroupchat(inviteInfo);
  qDebug() << "onGroupInviteAccepted groupId=>" << groupId;

  if (groupId == std::numeric_limits<uint32_t>::max()) {
    qWarning() << "onGroupInviteAccepted: Unable to accept group invite";
    return;
  }
}



void ContactWidget::onGroupPeerListChanged(QString groupnumber) {
  const GroupId &groupId = GroupId(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  assert(g);
//g->regeneratePeerList();
}

void ContactWidget::onGroupPeerSizeChanged(QString groupnumber, const uint size) {
  const GroupId &groupId = GroupId(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find the group named:" << groupnumber;
    return;
  }

  g->setPeerCount(size);
}

void ContactWidget::onGroupPeerNameChanged(QString groupnumber,
                                        const ToxPk &peerPk,
                                        const QString &newName) {
  const GroupId &groupId = GroupId(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find the group named:" << groupnumber;
    return;
  }
//  const QString &setName = FriendList::decideNickname(peerPk, newName);
//  g->updateUsername(peerPk, newName);
}

void ContactWidget::onGroupPeerStatusChanged(const QString &groupnumber,
                                             const GroupOccupant &go) {

  const GroupId &groupId = GroupId(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find group named:" << groupnumber;
    return;
  }

  g->addPeer(go);

}

void ContactWidget::onGroupTitleChanged(QString groupnumber, const QString &author,
                                     const QString &title) {
    qDebug()<<__func__ << "group" << groupnumber << title;
  const GroupId &groupId = GroupId(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find group" << groupnumber;
    return;
  }

  contactListWidget->setGroupTitle(groupId, author, title);

//  FilterCriteria filter = getFilterCriteria();
//  widget->searchName(ui->searchContactText->text(), filterGroups(filter));
}

//void ContactWidget::groupInvitesUpdate() {
//  if (unreadGroupInvites == 0) {
//    delete groupInvitesButton;
//    groupInvitesButton = nullptr;
//  } else if (!groupInvitesButton) {
//    groupInvitesButton = new QPushButton(this);
//    groupInvitesButton->setObjectName("green");
//    //    ui->statusLayout->insertWidget(2, groupInvitesButton);

//    connect(groupInvitesButton, &QPushButton::released, this,
//            &ContactWidget::onGroupClicked);
//  }

//  if (groupInvitesButton) {
//    groupInvitesButton->setText(
//        tr("%n New Group Invite(s)", "", unreadGroupInvites));
//  }
//}

//void ContactWidget::groupInvitesClear() {
//  unreadGroupInvites = 0;
//  groupInvitesUpdate();
//}

void ContactWidget::onGroupClicked() {
  auto &settings = Settings::getInstance();

  //    hideMainForms(nullptr);
  if (!groupInviteForm) {
    groupInviteForm = new GroupInviteForm;

    connect(groupInviteForm, &GroupInviteForm::groupCreate, core,
            &Core::createGroup);
  }
  groupInviteForm->show(contentLayout.get());
  //    setWindowTitle(fromDialogType(DialogType::GroupDialog));
  //    setActiveToolMenuButton(ActiveToolMenuButton::GroupButton);
}
