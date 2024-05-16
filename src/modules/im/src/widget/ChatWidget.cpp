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
#include "base/OkSettings.h"
#include "base/SvgUtils.h"
#include "base/utils.h"
#include "circlewidget.h"
#include "contentdialogmanager.h"
#include "contentlayout.h"
#include "friendlistwidget.h"
#include "lib/settings/translator.h"
#include "src/core/corefile.h"
#include "src/friendlist.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/modules/im/src/grouplist.h"
#include "src/persistence/profile.h"
#include "src/nexus.h"
#include "src/widget/form/addfriendform.h"
#include "src/widget/form/groupinviteform.h"
#include "MessageSessionListWidget.h"
#include "style.h"
#include "ui_ChatWidget.h"
#include "widget.h"
#include <QMenu>
#include <QPainter>
#include <QSvgRenderer>

ChatWidget::ChatWidget(QWidget *parent)
    : MainLayout(parent),  //
      ui(new Ui::ChatWidget), //
      unreadGroupInvites{0}, core{nullptr}, coreFile{nullptr} {
  ui->setupUi(this);
  layout()->setMargin(0);
  layout()->setSpacing(0);

  //  ui->mainSplitter->addWidget(contactListWidget);

  contentWidget = std::make_unique<QWidget>(this);
  contentWidget->setObjectName("contentWidget");
  contentLayout = std::make_unique<ContentLayout>(contentWidget.get());
  ui->mainSplitter->addWidget(contentWidget.get());
  ui->mainSplitter->setSizes(QList<int>() << 200 << 500);

  contactListWidget = std::make_unique<MessageSessionListWidget>(this, false);

  ui->scrollAreaWidgetContents->setGeometry(0, 0, 200, 500);
  auto layout = ui->scrollAreaWidgetContents->layout();
  layout->setAlignment(Qt::AlignTop | Qt::AlignVCenter);
  layout->addWidget((QWidget*)contactListWidget.get());

  const Settings& s = Settings::getInstance();
  setStyleSheet(Style::getStylesheet("window/chat.css"));
  reloadTheme();
  setupStatus();
  setupSearch();
  init();

//  QString locale = Settings::getInstance().getTranslation();
//  settings::Translator::translate(OK_IM_MODULE, locale);
  //  circleWidget= contactListWidget->createCircleWidget();
  //  connectCircleWidget();

  //  connect(contactListWidget, &FriendListWidget::searchCircle, this,
  //          &FriendListWidget::searchCircle);
  //  connect(contactListWidget, &FriendListWidget::connectCircleWidget, this,
  //          &FriendListWidget::connectCircleWidget);
}

ChatWidget::~ChatWidget() {
    deinit();
}

//void ChatWidget::searchCircle(CircleWidget &circleWidget) {
//  //  FilterCriteria filter = getFilterCriteria();
//  //  QString text = ui->searchContactText->text();
//  //  circleWidget.search(text, true, filterOnline(filter),
//  //  filterOffline(filter));
//}

//void ChatWidget::connectCircleWidget() {
////  connect(circleWidget, &CircleWidget::searchCircle, this,
////          &ChatWidget::searchCircle);
//  //  connect( circleWidget, &CircleWidget::newContentDialog, this,
//  //          &ChatWidget::registerContentDialog);
//}

void ChatWidget::init() {

    auto widget = Widget::getInstance();
    connect(widget, &Widget::toSendMessage, [&](const QString& to, bool isGroup){
        contactListWidget->toSendMessage(ToxPk(to), isGroup);
    });


    connect(Nexus::getProfile(), &Profile::coreChanged,
            this, &ChatWidget::onCoreChanged);
}

void ChatWidget::deinit() {

    disconnect(Nexus::getProfile(), &Profile::coreChanged,
               this, &ChatWidget::onCoreChanged);
}

void ChatWidget::connectToCore(Core *core) {
    qDebug() << __func__<<"core"<<core;
  connect(core, &Core::usernameSet, this, &ChatWidget::onUsernameSet);
  connect(core, &Core::statusSet, this, &ChatWidget::onStatusSet);
  connect(core, &Core::statusMessageSet, this, &ChatWidget::onStatusMessageSet);

  connect(core, &Core::friendMessageSessionReceived,
          this, &ChatWidget::onFriendMessageSessionReceived);
  connect(core, &Core::friendMessageReceived, this,
          &ChatWidget::onFriendMessageReceived);

  connect(core, &Core::friendUsernameChanged,
          this, &ChatWidget::onFriendUsernameChanged);

  connect(core, &Core::friendAvatarChanged, this,
          &ChatWidget::onFriendAvatarChanged);

  connect(core, &Core::friendStatusChanged, this,
          &ChatWidget::onFriendStatusChanged);
  connect(core, &Core::friendStatusMessageChanged, this,
          &ChatWidget::onFriendStatusMessageChanged);
  connect(core, &Core::friendRequestReceived, this,
          &ChatWidget::onFriendRequestReceived);

  connect(core, &Core::friendTypingChanged, this,
          &ChatWidget::onFriendTypingChanged);
  connect(core, &Core::receiptRecieved, this, &ChatWidget::onReceiptReceived);

  connect(core, &Core::groupAdded, this, &ChatWidget::onGroupJoined);

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

  auto username = core->getUsername();
  qDebug() << "username" << username;
  ui->nameLabel->setText(username);
}


void ChatWidget::onFriendMessageSessionReceived(const ToxPk &friendPk, const QString &sid)
{
     qDebug() << __func__ << "friend:" << friendPk.toString() << "sid:" <<sid;
     contactListWidget->createMessageSession(friendPk, sid, ChatType::Chat);
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
    contactListWidget->setFriendName(friendPk, username);
}

void ChatWidget::onFriendMessageReceived(
    const ToxPk &friendnumber,                //
    const FriendMessage &message, //
    bool isAction)                            //
{
  qDebug() << __func__ <<"content:"<< message.content << "from" << message.from.toString();
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

void ChatWidget::onFriendStatusChanged(const ToxPk &friendPk,
                                       Status::Status status) {
  qDebug() << __func__ << friendPk.toString() << "status:" << (int)status;
  //  const auto &friendPk = FriendList::id2Key(friendPk);
//    contactListWidget->setFriendStatus(friendPk, status);
    Friend *f = FriendList::findFriend(friendPk);
    if (!f) {
      qWarning() << "Unable to find friend" << friendPk.toString();
      return;
    }
    f->setStatus(status);

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

  //  widget->updateStatusLight();
  //  if (widget->isActive()) {
  //    setWindowTitle(widget->getTitle());
  //  }
  //
//    ContentDialogManager::getInstance()->updateFriendStatus(friendPk);
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
  if (!addFriendForm) {
    addFriendForm = std::make_unique<AddFriendForm>();

    connect(addFriendForm.get(), &AddFriendForm::friendRequested, this,
            &ChatWidget::friendRequestsUpdate);
    connect(addFriendForm.get(), &AddFriendForm::friendRequestsSeen, this,
            &ChatWidget::friendRequestsUpdate);

    //    connect(addFriendForm.get(), &AddFriendForm::friendRequestAccepted,
    //    this,
    //            &ChatWidget::friendRequestAccepted);
    //    connect(addFriendForm.get(), &AddFriendForm::friendRequestRejected,
    //    this,
    //            &ChatWidget::friendRequestRejected);
  }
  addFriendForm->show(nullptr);
  return addFriendForm.get();
}

void ChatWidget::showEvent(QShowEvent *e) {}

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

void ChatWidget::onUsernameSet(const QString &username) {

  ui->nameLabel->setText(username);
  ui->nameLabel->setToolTip(
      Qt::convertFromPlainText(username, Qt::WhiteSpaceNormal));
  // for overlength names
  //  sharedMessageProcessorParams.onUserNameSet(username);
}

void ChatWidget::onStatusSet(Status::Status status) {
  int icon_size = 15;
  ui->statusButton->setProperty("status", static_cast<int>(status));
  ui->statusButton->setIcon(SvgUtils::prepareIcon(
      getIconPath(status), icon_size, icon_size));



  updateIcons();
}

void ChatWidget::updateIcons() {

  QIcon ico;
  bool eventIcon = true;

  const QString assetSuffix =
      Status::getAssetSuffix(static_cast<Status::Status>(
          ui->statusButton->property("status").toInt())) +
      (eventIcon ? "_event" : "");

  QString color = Settings::getInstance().getLightTrayIcon() ? "light" : "dark";
  QString path = ":/img/taskbar/" + color + "/taskbar_" + assetSuffix + ".svg";
  QSvgRenderer renderer(path);

  // Prepare a QImage with desired characteritisc
  QImage image = QImage(250, 250, QImage::Format_ARGB32);
  image.fill(Qt::transparent);
  QPainter painter(&image);
  renderer.render(&painter);
  ico = QIcon(QPixmap::fromImage(image));
  setWindowIcon(ico);
}

void ChatWidget::onStatusMessageSet(const QString &statusMessage) {
    ui->statusLabel->setText(statusMessage);
}



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

void ChatWidget::onGroupJoined(const GroupId &groupId, const QString &name) {
  qDebug() << __func__ << groupId.toString() << name;
//  auto group = contactListWidget->addGroup(groupId, name);
//  qDebug() << "Created group:" << group << "=>" << groupId.toString();
}

void ChatWidget::onGroupInviteReceived(const GroupInvite &inviteInfo) {

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
      Widget::getInstance()->newMessageAlert(window(), isActiveWindow(), true,
                                             true);

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

void ChatWidget::onGroupMessageReceived(const GroupMessage& msg) {

  qDebug() <<__func__<< msg.toString();

  //  Group *g = GroupList::findGroup(groupId);
  //  if (!g) {
  //    qWarning() << "Can not find the group named:" << groupnumber;
  //    return;
  //  }
  //
  //  ToxPk author = core->getGroupPeerPk(groupnumber, nick);
  //  groupMessageDispatchers[groupId]->onMessageReceived(author, isAction,
  //  content,\
  nick, from, time);
//    contactListWidget->setRecvGroupMessage(msg);
}

void ChatWidget::onGroupPeerListChanged(QString groupnumber) {
//  const GroupId &groupId = GroupList::id2Key(groupnumber);
//  Group *g = GroupList::findGroup(groupId);
//  assert(g);
//  g->regeneratePeerList();
}

void ChatWidget::onGroupPeerSizeChanged(QString groupnumber, const uint size) {
  const GroupId &groupId = GroupId(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find the group named:" << groupnumber;
    return;
  }

  g->setPeerCount(size);
}

void ChatWidget::onGroupPeerNameChanged(QString groupnumber,
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

void ChatWidget::onGroupPeerStatusChanged(const QString& groupnumber,
                                          const GroupOccupant &go) {

  const GroupId &groupId = GroupId(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find group named:" << groupId.username;
    return;
  }

  g->addPeer(go);
//  g->regeneratePeerList();
}

void ChatWidget::onGroupTitleChanged(QString groupnumber, const QString &author,
                                     const QString &title) {
    qDebug()<<__func__ << "group" << groupnumber << title;
  const GroupId &groupId = GroupId(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find group" << groupnumber;
    return;
  }

//  contactListWidget->setGroupTitle(groupId, author, title);

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

    connect(groupInvitesButton, &QPushButton::released, this,
            &ChatWidget::onGroupClicked);
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
  if (!groupInviteForm) {
    groupInviteForm = new GroupInviteForm;

    connect(groupInviteForm, &GroupInviteForm::groupCreate, core,
            &Core::createGroup);
  }
  groupInviteForm->show(contentLayout.get());
  //    setWindowTitle(fromDialogType(DialogType::GroupDialog));
  //    setActiveToolMenuButton(ActiveToolMenuButton::GroupButton);
}
void ChatWidget::reloadTheme() {
  QString statusPanelStyle = Style::getStylesheet("window/statusPanel.css");
  //  ui->tooliconsZone->setStyleSheet(
  //      Style::getStylesheet("tooliconsZone/tooliconsZone.css"));
  //  ui->statusPanel->setStyleSheet(statusPanelStyle);
  ui->statusHead->setStyleSheet(statusPanelStyle);
  ui->friendList->setStyleSheet(
      Style::getStylesheet("friendList/friendList.css"));
  ui->statusButton->setStyleSheet(
      Style::getStylesheet("statusButton/statusButton.css"));
  contactListWidget->reDraw();

  //  profilePicture->setStyleSheet(Style::getStylesheet("window/profile.css"));

  if (contentLayout != nullptr) {
    contentLayout->reloadTheme();
  }

  //  for (Friend *f : FriendList::getAllFriends()) {
  //    contactListWidget->getFriend(f->getPublicKey())->reloadTheme();
  //  }

  contactListWidget->reloadTheme();
}
void ChatWidget::setupSearch() {

  filterMenu = new QMenu(this);
  filterGroup = new QActionGroup(this);
  filterDisplayGroup = new QActionGroup(this);

  filterDisplayName = new QAction(this);
  filterDisplayName->setCheckable(true);
  filterDisplayName->setChecked(true);
  filterDisplayGroup->addAction(filterDisplayName);
  filterMenu->addAction(filterDisplayName);
  filterDisplayActivity = new QAction(this);
  filterDisplayActivity->setCheckable(true);
  filterDisplayGroup->addAction(filterDisplayActivity);
  filterMenu->addAction(filterDisplayActivity);

  Settings::getInstance().getFriendSortingMode() == MessageSessionListWidget::SortingMode::Name
      ? filterDisplayName->setChecked(true)
      : filterDisplayActivity->setChecked(true);
  filterMenu->addSeparator();

  filterAllAction = new QAction(this);
  filterAllAction->setCheckable(true);
  filterAllAction->setChecked(true);
  filterGroup->addAction(filterAllAction);
  filterMenu->addAction(filterAllAction);
  filterOnlineAction = new QAction(this);
  filterOnlineAction->setCheckable(true);
  filterGroup->addAction(filterOnlineAction);
  filterMenu->addAction(filterOnlineAction);
  filterOfflineAction = new QAction(this);
  filterOfflineAction->setCheckable(true);
  filterGroup->addAction(filterOfflineAction);
  filterMenu->addAction(filterOfflineAction);
  filterFriendsAction = new QAction(this);
  filterFriendsAction->setCheckable(true);
  filterGroup->addAction(filterFriendsAction);
  filterMenu->addAction(filterFriendsAction);
  filterGroupsAction = new QAction(this);
  filterGroupsAction->setCheckable(true);
  filterGroup->addAction(filterGroupsAction);
  filterMenu->addAction(filterGroupsAction);

  filterDisplayName->setText(tr("By Name"));
  filterDisplayActivity->setText(tr("By Activity"));
  filterAllAction->setText(tr("All"));
  filterOnlineAction->setText(tr("Online"));
  filterOfflineAction->setText(tr("Offline"));
  filterFriendsAction->setText(tr("Friends"));
  filterGroupsAction->setText(tr("Groups"));

  ui->searchContactText->setPlaceholderText(tr("Search Contacts"));
    connect(ui->searchContactText, &QLineEdit::textChanged, this,
            &ChatWidget::searchContacts);

  ui->searchContactFilterBox->setMenu(filterMenu);
  updateFilterText();


    connect(filterGroup, &QActionGroup::triggered,
            this, &ChatWidget::searchContacts);
    connect(filterDisplayGroup, &QActionGroup::triggered,
            this,&ChatWidget::changeDisplayMode);


}

void ChatWidget::changeDisplayMode() {
  filterDisplayGroup->setEnabled(false);

  //  if (filterDisplayGroup->checkedAction() == filterDisplayActivity) {
  //    contactListWidget->setMode(FriendListWidget::SortingMode::Activity);
  //  } else if (filterDisplayGroup->checkedAction() == filterDisplayName) {
  //    contactListWidget->setMode(FriendListWidget::SortingMode::Name);
  //  }

  searchContacts();
  filterDisplayGroup->setEnabled(true);

  updateFilterText();
}

void ChatWidget::searchContacts() {
    QString searchString = ui->searchContactText->text();

    qDebug() <<__func__ << searchString;

    FilterCriteria filter = getFilterCriteria();

    contactListWidget->searchChatrooms(searchString,
                                       filterOnline(filter),
                                       filterOffline(filter),
                                       filterGroups(filter));
    contactListWidget->reDraw();
    updateFilterText();
}

void ChatWidget::updateFilterText() {
  QString action = filterDisplayGroup->checkedAction()->text();
  QString text = filterGroup->checkedAction()->text();
  text = action + QStringLiteral(" | ") + text;
  ui->searchContactFilterBox->setText(text);
}

ChatWidget::FilterCriteria ChatWidget::getFilterCriteria() const {
  QAction *checked = filterGroup->checkedAction();

  if (checked == filterOnlineAction)
    return FilterCriteria::Online;
  else if (checked == filterOfflineAction)
    return FilterCriteria::Offline;
  else if (checked == filterFriendsAction)
    return FilterCriteria::Friends;
  else if (checked == filterGroupsAction)
    return FilterCriteria::Groups;

  return FilterCriteria::All;
}


bool ChatWidget::filterGroups(FilterCriteria index) {
  switch (index) {
  case FilterCriteria::Offline:
  case FilterCriteria::Friends:
    return true;
  default:
    return false;
  }
}

bool ChatWidget::filterOffline(FilterCriteria index) {
  switch (index) {
  case FilterCriteria::Online:
  case FilterCriteria::Groups:
    return true;
  default:
    return false;
  }
}

bool ChatWidget::filterOnline(FilterCriteria index) {
  switch (index) {
  case FilterCriteria::Offline:
  case FilterCriteria::Groups:
    return true;
  default:
    return false;
  }
}


bool ChatWidget::groupsVisible() const {
  FilterCriteria filter = getFilterCriteria();
  return !filterGroups(filter);
}

void ChatWidget::retranslateUi() {
  ui->retranslateUi(this);



}
void ChatWidget::setupStatus() {
 int icon_size = 15;

  // Preparing icons and set their size
  statusOnline = new QAction(this);
  statusOnline->setIcon(SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Online),
                                              icon_size, icon_size));
  connect(statusOnline, &QAction::triggered, this, &ChatWidget::setStatusOnline);

  statusAway = new QAction(this);
  statusAway->setIcon(SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Away),
                                            icon_size, icon_size));
  connect(statusAway, &QAction::triggered, this, &ChatWidget::setStatusAway);

  statusBusy = new QAction(this);
  statusBusy->setIcon(SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Busy),
                                            icon_size, icon_size));
  connect(statusBusy, &QAction::triggered, this, &ChatWidget::setStatusBusy);

  QMenu *statusButtonMenu = new QMenu(ui->statusButton);
  statusButtonMenu->addAction(statusOnline);
  statusButtonMenu->addAction(statusAway);
  statusButtonMenu->addAction(statusBusy);
  ui->statusButton->setMenu(statusButtonMenu);



  //  ui->searchContactText->setPlaceholderText(tr("Search Contacts"));
  statusOnline->setText(tr("Online", "Button to set your status to 'Online'"));
  statusAway->setText(tr("Away", "Button to set your status to 'Away'"));
  statusBusy->setText(tr("Busy", "Button to set your status to 'Busy'"));
//  actionLogout->setText(tr("Logout", "Tray action menu to logout user"));
//  actionQuit->setText(tr("Exit", "Tray action menu to exit tox"));
//  actionShow->setText(tr("Show", "Tray action menu to show qTox window"));

}



void ChatWidget::setStatusOnline() {
  //  if (!ui->statusButton->isEnabled()) {
  //    return;
  //  }

  core->setStatus(Status::Status::Online);
}

void ChatWidget::setStatusAway() {
  //  if (!ui->statusButton->isEnabled()) {
  //    return;
  //  }

  core->setStatus(Status::Status::Away);
}

void ChatWidget::setStatusBusy() {
  //  if (!ui->statusButton->isEnabled()) {
  //    return;
  //  }

  core->setStatus(Status::Status::Busy);
}
