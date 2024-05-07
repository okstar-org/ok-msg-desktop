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

//
// Created by gaojie on 24-5-7.
//

#include "ChatWidget.h"
#include "circlewidget.h"
#include "contentlayout.h"
#include "friendlistwidget.h"
#include "src/core/corefile.h"
#include "src/friendlist.h"
#include "src/nexus.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/widget/form/groupinviteform.h"
#include "src/widget/form/addfriendform.h"
#include "src/modules/im/src/grouplist.h"
#include "widget.h"
#include "ui_chat.h"

ChatWidget::ChatWidget(QWidget *parent) : QWidget(parent), //
    ui(new Ui::Chat),//
    unreadGroupInvites{0} {
  ui->setupUi(this);
  layout()->setMargin(0);
  layout()->setSpacing(0);

  //  ui->mainSplitter->addWidget(contactListWidget);

  QWidget *contentWidget = new QWidget(this);
  contentWidget->setObjectName("contentWidget");
  contentLayout = new ContentLayout(contentWidget);
  ui->mainSplitter->addWidget(contentWidget);
  ui->mainSplitter->setSizes(QList<int>() << 200<<500);

  contactListWidget = new FriendListWidget(this, false);


  ui->scrollAreaWidgetContents->setGeometry(0, 0, 200, 500);
  auto layout = ui->scrollAreaWidgetContents->layout();
  layout->setAlignment(Qt::AlignTop|Qt::AlignVCenter);
  layout->addWidget(contactListWidget);

  init();

  //  circleWidget= contactListWidget->createCircleWidget();
  //  connectCircleWidget();

  //  connect(contactListWidget, &FriendListWidget::searchCircle, this,
  //          &FriendListWidget::searchCircle);
  //  connect(contactListWidget, &FriendListWidget::connectCircleWidget, this,
  //          &FriendListWidget::connectCircleWidget);
}

ChatWidget::~ChatWidget() {}

void ChatWidget::searchCircle(CircleWidget &circleWidget) {
  //  FilterCriteria filter = getFilterCriteria();
  //  QString text = ui->searchContactText->text();
  //  circleWidget.search(text, true, filterOnline(filter),
  //  filterOffline(filter));
}

void ChatWidget::connectCircleWidget() {
  connect(circleWidget, &CircleWidget::searchCircle, this,
          &ChatWidget::searchCircle);
  //  connect( circleWidget, &CircleWidget::newContentDialog, this,
  //          &ChatWidget::registerContentDialog);
}

void ChatWidget::init() {
  //  core = Nexus::getCore();
  //  coreFile = core->getCoreFile();


}

void ChatWidget::deinit() {
  coreFile->deleteLater();
  core->deleteLater();
}

void ChatWidget::connectToCore(Core *core)
{
    connect(core, &Core::sig_friendAdded, this, &ChatWidget::slot_friendAdded);
    connect(core, &Core::friendUsernameChanged, this,
            &ChatWidget::onFriendUsernameChanged);
    connect(core, &Core::friendAvatarChanged, this,
            &ChatWidget::onFriendAvatarChanged);

    connect(core, &Core::friendStatusChanged, this,
            &ChatWidget::onFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, this,
            &ChatWidget::onFriendStatusMessageChanged);
    connect(core, &Core::friendRequestReceived, this,
            &ChatWidget::onFriendRequestReceived);
    connect(core, &Core::friendMessageReceived, this,
            &ChatWidget::onFriendMessageReceived);
    connect(core, &Core::friendTypingChanged, this,
            &ChatWidget::onFriendTypingChanged);
    connect(core, &Core::receiptRecieved,
            this, &ChatWidget::onReceiptReceived);

    connect(core, &Core::groupJoined, this, &ChatWidget::onGroupJoined);

    connect(core, &Core::groupInviteReceived, this,
            &ChatWidget::onGroupInviteReceived);
    connect(core, &Core::groupMessageReceived, this,
            &ChatWidget::onGroupMessageReceived);
    connect(core, &Core::groupPeerlistChanged, this,
            &ChatWidget::onGroupPeerListChanged);
    connect(core, &Core::groupPeerSizeChanged, this,
            &ChatWidget::onGroupPeerSizeChanged);
    connect(core, &Core::groupPeerNameChanged, this,
            &ChatWidget::onGroupPeerNameChanged);
    connect(core, &Core::groupPeerStatusChanged, this,
            &ChatWidget::onGroupPeerStatusChanged);
    connect(core, &Core::groupTitleChanged, this,
            &ChatWidget::onGroupTitleChanged);

//    connect(core, &Core::groupPeerAudioPlaying, this,
//            &ChatWidget::onGroupPeerAudioPlaying);
//    connect(core, &Core::emptyGroupCreated, this,
//           &ChatWidget::onEmptyGroupCreated);

//    connect(&core, &Core::groupSentFailed, this,
//            &ChatWidget::onGroupSendFailed);

}

void ChatWidget::onCoreChanged(Core &core_) {
  core = &core_;
  connectToCore(core);

  auto fl = core->loadFriendList();
  for (auto fk  : fl) {
    contactListWidget->addFriend(fk, true);
  }

  core->loadGroupList();
  auto username = core->getUsername();
  qDebug() <<"username"<< username;
  ui->nameLabel->setText(username);
}

void ChatWidget::slot_friendAdded(const ToxPk &friendPk, bool isFriend) {
  contactListWidget->addFriend(friendPk, isFriend);
}

void ChatWidget::onFriendAvatarChanged(const ToxPk &friendnumber,
                                       const QByteArray &avatar) {
  qDebug() << __func__ << "friend:" << friendnumber.toString() << avatar.size();
  contactListWidget->setFriendAvatar(friendnumber, avatar);
}

void ChatWidget::onFriendUsernameChanged(const ToxPk &friendPk,
                                         const QString &username) {
  qDebug() << __func__ << "friend:" << friendPk.toString()
           << "name:" << username;

  Friend *f = FriendList::findFriend(friendPk);
  if (!f) {
    qWarning() << "Can not find friend.";
    return;
  }

  QString str = username;
  str.replace('\n', ' ').remove('\r').remove(QChar('\0'));
  f->setName(str);
}

void ChatWidget::onFriendMessageReceived(
    const ToxPk &friendnumber,                //
    const lib::messenger::IMMessage &message, //
    bool isAction)                            //
{
  qDebug() << __func__ << message.body << "from" << message.from;
  contactListWidget->setRecvFriendMessage(friendnumber, message, isAction);
}

void ChatWidget::onReceiptReceived(const ToxPk &friendId, ReceiptNum receipt) {
  qDebug() << __func__ << "friendId:" << friendId.toString();

  Friend *f = FriendList::findFriend(friendId);
  if (!f) {
    return;
  }
  //  TODO
//    contactListWidget->onReceiptReceived(receipt);
}

void ChatWidget::onFriendStatusChanged(const ToxPk &friendId,
                                       Status::Status status) {
  qDebug() << __func__ << friendId.toString() << "status:" << ((int)status);
  //  const auto &friendPk = FriendList::id2Key(friendId);
  //  contactListWidget->setFriendStatus(friendPk, status);
  //  Friend *f = FriendList::findFriend(friendPk);
  //  if (!f) {
  //    qWarning() << "Unable to find friend" << friendId;
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
  //  ContentDialogManager::getInstance()->updateFriendStatus(friendPk);
}

void ChatWidget::onFriendStatusMessageChanged(const ToxPk &friendPk,
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

void ChatWidget::onFriendTypingChanged(const ToxPk &friendId, bool isTyping) {

  contactListWidget->setFriendTyping(friendId, isTyping);
}

void ChatWidget::onFriendRequestReceived(const ToxPk &friendPk,
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

AddFriendForm *ChatWidget::openFriendAddForm() {
  if(!addFriendForm){
    addFriendForm = std::make_unique<AddFriendForm>();

    connect(addFriendForm.get(), &AddFriendForm::friendRequested, this,
            &ChatWidget::friendRequestsUpdate);
    connect(addFriendForm.get(), &AddFriendForm::friendRequestsSeen, this,
            &ChatWidget::friendRequestsUpdate);

//    connect(addFriendForm.get(), &AddFriendForm::friendRequestAccepted, this,
//            &ChatWidget::friendRequestAccepted);
//    connect(addFriendForm.get(), &AddFriendForm::friendRequestRejected, this,
//            &ChatWidget::friendRequestRejected);

  }
  addFriendForm->show(nullptr);
  return addFriendForm.get();
}

void ChatWidget::showEvent(QShowEvent *e)
{
    qDebug() <<"showEvent: "<<e;
    qDebug() <<"friendList size:" << ui->contactPanel->size() << ui->friendList->size() << ui->scrollAreaWidgetContents->size();


}


// void ChatWidget::onFriendDisplayedNameChanged(const ToxPk & friendPk, const
// QString &displayed) {
//    contactListWidget->setFriendName(friendPk, displayed);
// }

// void ChatWidget::onFriendAliasChanged(const ToxPk &friendId, const QString
// &alias) {
//   qDebug() << __func__ <<"friendId" << friendId.toString() << alias;
//   Friend *f = qobject_cast<Friend *>(sender());
//
//   // TODO(sudden6): don't update the contact list here, make it update itself
//   FriendWidget *friendWidget = contactListWidget->getFriend(friendId);
//   Status::Status status = f->getStatus();
//   contactListWidget->moveWidget(friendWidget, status);
//   FilterCriteria criteria = getFilterCriteria();
//   bool filter = status == Status::Status::Offline ? filterOffline(criteria)
//                                                   : filterOnline(criteria);
//   friendWidget->searchName(ui->searchContactText->text(), filter);
//
//   settings.setFriendAlias(friendId, alias);
//   settings.savePersonal();
//}

void ChatWidget::friendRequestsUpdate() {
    auto &settings = Settings::getInstance();

  unsigned int unreadFriendRequests = settings.getUnreadFriendRequests();

  if (unreadFriendRequests == 0) {
    delete friendRequestsButton;
    friendRequestsButton = nullptr;
  } else if (!friendRequestsButton) {
    friendRequestsButton = new QPushButton(this);
    friendRequestsButton->setObjectName("green");
//    ui->statusLayout->insertWidget(2, friendRequestsButton);

//    connect(friendRequestsButton, &QPushButton::released, [this]() {
//      onAddClicked();
//      addFriendForm->setMode(AddFriendForm::Mode::FriendRequest);
//    });
  }

  if (friendRequestsButton) {
    friendRequestsButton->setText(
        tr("%n New Friend Request(s)", "", unreadFriendRequests));
  }
}

void ChatWidget::onGroupJoined(const GroupId &groupId, const QString& name) {
  qDebug() << __func__ <<groupId.toString()<<name;
  auto group = contactListWidget->addGroup(groupId, name);

  qDebug() << "Created group:" << group << "=>" << groupId.toString();
  //  core->joinRoom(groupNuvoid ChatWidget::onGroupJoinedDone(){
  //  qDebug()<<"onGroupJoinedDone";
  //  delayCaller->call(1000,[&](){
  //    for(auto group: GroupList::getAllGroups()){
  //      core->joinRoom(group->getId(), core->getNick());
  //    }
  //  });
}

void ChatWidget::onGroupInviteReceived(const GroupInvite &inviteInfo) {
  //  const QString friendId = inviteInfo.getFriendId();
  //  const ToxPk &friendPk = FriendList::id2Key(friendId);
  //  const Friend *f = FriendList::findFriend(friendPk);
  //  updateFriendActivity(*f);

  const uint8_t confType = inviteInfo.getType();
  if (confType == TOX_CONFERENCE_TYPE_TEXT ||
      confType == TOX_CONFERENCE_TYPE_AV) {
    if (false
        // settings.getAutoGroupInvite(f->getPublicKey())
    ) {
      onGroupInviteAccepted(inviteInfo);
    } else {
      if (!groupInviteForm->addGroupInvite(inviteInfo)) {
        return;
      }

      ++unreadGroupInvites;
      groupInvitesUpdate();
      Widget::getInstance()->newMessageAlert(window(), isActiveWindow(), true, true);

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

void ChatWidget::onGroupInviteAccepted(const GroupInvite &inviteInfo) {
  const QString groupId = core->joinGroupchat(inviteInfo);
  qDebug() << "onGroupInviteAccepted groupId=>" << groupId;

  if (groupId == std::numeric_limits<uint32_t>::max()) {
    qWarning() << "onGroupInviteAccepted: Unable to accept group invite";
    return;
  }
}

void ChatWidget::onGroupMessageReceived(QString groupnumber,
                                    QString nick,
                                    const QString &from,
                                    const QString &content,
                                    const QDateTime &time,
                                    bool isAction) {

  qDebug() << "onGroupMessageReceived group:" << groupnumber
           << "nick:" << nick
           << "content:" << content;

//  Group *g = GroupList::findGroup(groupId);
//  if (!g) {
//    qWarning() << "Can not find the group named:" << groupnumber;
//    return;
//  }
//
//  ToxPk author = core->getGroupPeerPk(groupnumber, nick);
//  groupMessageDispatchers[groupId]->onMessageReceived(author, isAction, content,
//                                                      nick, from, time);



//  contactListWidget->setRecvGroupMessage(groupnumber, nick, from, content, time, isAction);
}

void ChatWidget::onGroupPeerListChanged(QString groupnumber) {
  const GroupId &groupId = GroupList::id2Key(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  assert(g);
  g->regeneratePeerList();
}

void ChatWidget::onGroupPeerSizeChanged(QString groupnumber, const uint size) {
  const GroupId &groupId = GroupList::id2Key(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find the group named:" << groupnumber;
    return;
  }

  g->numPeersChanged(size);
}

void ChatWidget::onGroupPeerNameChanged(QString groupnumber, const ToxPk &peerPk,
                                    const QString &newName) {
  const GroupId &groupId = GroupList::id2Key(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find the group named:" << groupnumber;
    return;
  }
  const QString &setName = FriendList::decideNickname(peerPk, newName);
  g->updateUsername(peerPk, newName);
}

void ChatWidget::onGroupPeerStatusChanged(QString groupnumber, QString peerName,
                                      bool online) {

  const GroupId &groupId = GroupList::id2Key(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find group named:" << groupnumber;
    return;
  }

  g->addPeerName(groupId);
  g->regeneratePeerList();
}

void ChatWidget::onGroupTitleChanged(QString groupnumber, const QString &author,
                                 const QString &title) {
  const GroupId &groupId = GroupList::id2Key(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find group" << groupnumber;
    return;
  }

//  GroupWidget *widget = groupWidgets[groupId];
//  if (widget->isActive()) {
//    GUI::setWindowTitle(title);
//  }
//
//  g->setTitle(author, title);
//  FilterCriteria filter = getFilterCriteria();
//  widget->searchName(ui->searchContactText->text(), filterGroups(filter));
}


void ChatWidget::groupInvitesUpdate() {
  if (unreadGroupInvites == 0) {
    delete groupInvitesButton;
    groupInvitesButton = nullptr;
  } else if (!groupInvitesButton) {
    groupInvitesButton = new QPushButton(this);
    groupInvitesButton->setObjectName("green");
//    ui->statusLayout->insertWidget(2, groupInvitesButton);

    connect(groupInvitesButton, &QPushButton::released,
            this, &ChatWidget::onGroupClicked);
  }

  if (groupInvitesButton) {
    groupInvitesButton->setText(
        tr("%n New Group Invite(s)", "", unreadGroupInvites));
  }
}

void ChatWidget::groupInvitesClear() {
  unreadGroupInvites = 0;
  groupInvitesUpdate();
}

void ChatWidget::onGroupClicked() {
  auto &settings = Settings::getInstance();

//    hideMainForms(nullptr);
    if(!groupInviteForm){
      groupInviteForm = new GroupInviteForm;

      connect(groupInviteForm, &GroupInviteForm::groupCreate,
              core,
              &Core::createGroup);
    }
  groupInviteForm->show(contentLayout);
//    setWindowTitle(fromDialogType(DialogType::GroupDialog));
//    setActiveToolMenuButton(ActiveToolMenuButton::GroupButton);
  }


