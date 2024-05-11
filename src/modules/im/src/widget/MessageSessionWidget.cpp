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

#include "MessageSessionWidget.h"

#include "circlewidget.h"
#include "friendlistwidget.h"
#include "groupwidget.h"
#include "maskablepixmapwidget.h"

#include "contentdialogmanager.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/aboutfriend.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/form/aboutfriendform.h"
#include "src/widget/form/chatform.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/widget.h"

#include <QApplication>
#include <QBitmap>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDrag>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMimeData>

#include "form/chatform.h"
#include "src/model/chathistory.h"
#include "src/persistence/profile.h"
#include <cassert>

/**
 * @class MessageSessionWidget
 *
 * Widget, which displays brief information about friend.
 * For example, used on friend list.
 * When you click should open the chat with friend. Widget has a context menu.
 */
MessageSessionWidget::MessageSessionWidget(ContentLayout *layout,
                           const ToxPk &friendPk,
                           bool isFriend,
                           bool compact)
    : GenericChatroomWidget(compact), contentLayout(layout),
      isDefaultAvatar{true} {

  qDebug() <<__func__ <<"friend:"<<friendPk.toString();

  avatar->setPixmap(QPixmap(":/img/contact.svg"));
  statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Offline)));
  statusPic.setMargin(3);

  auto profile = Nexus::getProfile();
  auto core = Core::getInstance();
  auto &settings = Settings::getInstance();
  auto history = profile->getHistory();
  auto dialogManager = ContentDialogManager::getInstance();

  m_friend = FriendList::addFriend(friendPk, isFriend);

  auto messageProcessor = MessageProcessor(sharedMessageProcessorParams);
  messageDispatcher = std::make_unique<FriendMessageDispatcher>(
      *m_friend, messageProcessor, *core);

  // Note: We do not have to connect the message dispatcher signals since
  // ChatHistory hooks them up in a very specific order
  chatHistory = std::make_unique<ChatHistory>(*m_friend, history, *core,
                                              Settings::getInstance(),
                                              *messageDispatcher.get());


  chatLog = std::make_unique<SessionChatLog>(*core);
  connect(messageDispatcher.get(), &IMessageDispatcher::messageReceived,
          chatLog.get(), &SessionChatLog::onMessageReceived);

  chatForm = std::make_unique<ChatForm>(m_friend,
                                        *chatHistory,
                                        *messageDispatcher);

  contentWidget = new ContentWidget(this);
  contentWidget->hide();
  contentWidget->setChatForm(chatForm.get());
  //  const auto compact = settings.getCompactLayout();

  const auto activityTime = settings.getFriendActivity(friendPk);
  const auto chatTime = chatForm->getLatestTime();
  if (chatTime > activityTime && chatTime.isValid()) {

    settings.setFriendActivity(friendPk, chatTime);
  }

  chatRoom = std::make_unique<FriendChatroom>(m_friend, dialogManager);
  auto frnd = chatRoom->getFriend();
  nameLabel->setText(frnd->getDisplayedName());

  // update alias when edited
  connect(nameLabel, &CroppingLabel::editFinished, //
          frnd, &Friend::setAlias);
  // update on changes of the displayed name
  connect(frnd, &Friend::displayedNameChanged, //
          nameLabel, &CroppingLabel::setText);

  connect(frnd, &Friend::displayedNameChanged, this,
          [this](const QString &newName) {
            Q_UNUSED(newName);
            emit widgetRenamed(this);
          });
  connect(chatRoom.get(), &FriendChatroom::activeChanged, this,
          &MessageSessionWidget::setActive);
  //  statusMessageLabel->setTextFormat(Qt::PlainText);

  //  connect(this, &MessageSessionWidget::middleMouseClicked, dialog,
  //          [this]() { dialog->removeFriend(friendPk); });
  //  connect(MessageSessionWidget, &MessageSessionWidget::copyFriendIdToClipboard, this,
  //          &Widget::copyFriendIdToClipboard);
  //  connect(MessageSessionWidget, &MessageSessionWidget::newWindowOpened, this,
  //          &Widget::openNewDialog);

  // Signal transmission from the created `MessageSessionWidget` (which shown in
  // ContentDialog) to the `widget` (which shown in main widget)
  // FIXME: emit should be removed
  //  connect(
  //      this, &MessageSessionWidget::contextMenuCalled, this,
  //      [this](QContextMenuEvent *event) { emit contextMenuCalled(event); });
  //
  connect(this, &MessageSessionWidget::chatroomWidgetClicked,
          [=, this](GenericChatroomWidget *w) {
            Q_UNUSED(w);
            do_widgetClicked();
            emit widgetClicked(this);
          });
  //  connect(MessageSessionWidget, &MessageSessionWidget::newWindowOpened,
  //          [this](GenericChatroomWidget *w) {
  //            Q_UNUSED(w);
  //            emit MessageSessionWidget->newWindowOpened(MessageSessionWidget);
  //          });

  // Try to get the avatar from the cache
  QPixmap avatar = Nexus::getProfile()->loadAvatar(friendPk);
  if (!avatar.isNull()) {
    chatForm->onAvatarChanged(friendPk, avatar);
    setAvatar(avatar);
  }
}

void MessageSessionWidget::do_widgetClicked( ) {
//  qDebug() << __func__ << "show friend:" << m_friend->getId();


  //  GroupId id;

  //  GenericChatForm *form = reinterpret_cast<GenericChatForm
  //  *>(chatRoom.get());

  //  ContentDialogManager::getInstance()->focusContact(id);
  //  bool chatFormIsSet =
  //  ContentDialogManager::getInstance()->contactWidgetExists(id); if
  //  ((chatFormIsSet || form->isVisible()) && !newWindow) {
  //    return;
  //  }

  bool newWindow = false;
  auto &settings = Settings::getInstance();
  if (settings.getSeparateWindow() || newWindow) {
    ContentDialog *dialog = nullptr;

    //    if (!settings.getDontGroupWindows()) {
    //      dialog = ContentDialogManager::getInstance()->current();
    //    }

    if (m_friend) {
      qDebug() << "show friend:" << m_friend->getId();
      dialog = addFriendDialog(m_friend);
    } else {
      //      Group *group = widget->getGroup();
      //      addGroupDialog(group, dialog);
    }
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
  } else {
    //    hideMainForms(widget);
    if (m_friend) {
      contentWidget->showTo(contentLayout);
    } else {
      //      groupChatForms[group->getPersistentId()]->show(contentLayout);
    }
    //    widget->setAsActiveChatroom();
    //    setWindowTitle(widget->getTitle());
  }
}

ContentDialog *MessageSessionWidget::addFriendDialog(const Friend *frnd) {
  QString friendId = frnd->getId();
  qDebug() << __func__ << friendId;

  const ToxPk &friendPk = frnd->getPublicKey();
  qDebug() << "friendPk" << friendPk.toString();

  ContentDialog *dialog =
      ContentDialogManager::getInstance()->getFriendDialog(friendPk);
  qDebug() << "Find contentDialog:" << dialog;
  if (!dialog) {
    dialog = createContentDialog();
  }

  auto &settings = Settings::getInstance();
  bool isSeparate = settings.getSeparateWindow();
  //  MessageSessionWidget *widget = MessageSessionWidgets[friendPk];
  //  bool isCurrent = activeChatroomWidget == widget;
  //  if (!contentDialog && !isSeparate && isCurrent) {
  //    onAddClicked();
  //  }

  ContentDialogManager::getInstance()->addFriendToDialog(
      friendPk, dialog, chatRoom.get(), chatForm.get());

  //  friendWidget->setStatusMsg(widget->getStatusMsg());

  // #if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
  //   auto widgetRemoveFriend = QOverload<const ToxPk
  //   &>::of(&Widget::removeFriend);
  // #else
  //   auto widgetRemoveFriend =
  //       static_cast<void (Widget::*)(const ToxPk &)>(&Widget::removeFriend);
  // #endif
  //   connect(friendWidget, &MessageSessionWidget::removeFriend, this,
  //   widgetRemoveFriend); connect(friendWidget, &MessageSessionWidget::addFriend,
  //   this, &Widget::addFriend0);

  // FIXME: emit should be removed
  //  emit friendWidget->chatroomWidgetClicked(friendWidget);

  Profile *profile = Nexus::getProfile();
  connect(profile, &Profile::friendAvatarSet, this, &MessageSessionWidget::onAvatarSet);
  connect(profile, &Profile::friendAvatarRemoved, this,
          &MessageSessionWidget::onAvatarRemoved);

  QPixmap avatar = Nexus::getProfile()->loadAvatar(frnd->getPublicKey());
  if (!avatar.isNull()) {
    setAvatar(avatar);
  }

  return dialog;
}

/**
 * @brief MessageSessionWidget::contextMenuEvent
 * @param event Describe a context menu event
 *
 * Default context menu event handler.
 * Redirect all event information to the signal.
 */
void MessageSessionWidget::contextMenuEvent(QContextMenuEvent *event) {
  onContextMenuCalled(event);
  emit contextMenuCalled(event);
}

/**
 * @brief MessageSessionWidget::onContextMenuCalled
 * @param event Redirected from native contextMenuEvent
 *
 * Context menu handler. Always should be called to MessageSessionWidget from FriendList
 */
void MessageSessionWidget::onContextMenuCalled(QContextMenuEvent *event) {
  if (!active) {
    setBackgroundRole(QPalette::Highlight);
  }

  installEventFilter(this); // Disable leave event.

  QMenu menu;

  if (chatRoom->possibleToOpenInNewWindow()) {
    const auto openChatWindow = menu.addAction(tr("Open chat in new window"));
    connect(openChatWindow, &QAction::triggered,
            [this]() { emit newWindowOpened(this); });
  }

  if (chatRoom->canBeRemovedFromWindow()) {
    const auto removeChatWindow =
        menu.addAction(tr("Remove chat from this window"));
    connect(removeChatWindow, &QAction::triggered, this,
            &MessageSessionWidget::removeChatWindow);
  }

  menu.addSeparator();
  QMenu *inviteMenu = menu.addMenu(
      tr("Invite to group", "Menu to invite a friend to a groupchat"));
  //  inviteMenu->setEnabled(chatRoom->canBeInvited());

  const auto newGroupAction = inviteMenu->addAction(tr("To new group"));
  connect(newGroupAction, &QAction::triggered, chatRoom.get(),
          &FriendChatroom::inviteToNewGroup);
  inviteMenu->addSeparator();

  for (const auto &group : chatRoom->getGroups()) {
    const auto groupAction =
        inviteMenu->addAction(tr("Invite to group '%1'").arg(group.name));
    connect(groupAction, &QAction::triggered,
            [=, this]() { chatRoom->inviteFriend(group.group); });
  }
  //
  //  const auto circleId = chatRoom->getCircleId();
  //  auto circleMenu = menu.addMenu(
  //      tr("Move to circle...", "Menu to move a friend into a different
  //      circle"));
  //
  //  const auto newCircleAction = circleMenu->addAction(tr("To new circle"));
  //  connect(newCircleAction, &QAction::triggered, this,
  //          &MessageSessionWidget::moveToNewCircle);
  //
  //  if (circleId != -1) {
  //    const auto circleName = chatRoom->getCircleName();
  //    const auto removeCircleAction =
  //        circleMenu->addAction(tr("Remove from circle
  //        '%1'").arg(circleName));
  //    connect(removeCircleAction, &QAction::triggered, this,
  //            &MessageSessionWidget::removeFromCircle);
  //  }
  //  circleMenu->addSeparator();
  //
  //  for (const auto &circle : chatRoom->getOtherCircles()) {
  //    QAction *action =
  //        new QAction(tr("Move  to circle \"%1\"").arg(circle.name),
  //        circleMenu);
  //    connect(action, &QAction::triggered,
  //            [this]() { moveToCircle(circle.circleId); });
  //    circleMenu->addAction(action);
  //  }

  const auto setAlias = menu.addAction(tr("Set alias..."));
  connect(setAlias, &QAction::triggered, nameLabel, &CroppingLabel::editBegin);

  //  自动接收文件
  //  menu.addSeparator();
  //  auto autoAccept = menu.addAction(
  //      tr("Auto accept files from this friend", "context menu entry"));
  //  autoAccept->setCheckable(true);
  //  autoAccept->setChecked(!chatRoom->autoAcceptEnabled());
  //  connect(autoAccept, &QAction::triggered, this,
  //  &MessageSessionWidget::changeAutoAccept);

  auto fnd = chatRoom->getFriend();

  menu.addSeparator();
  if (chatRoom->friendCanBeRemoved()) {
    const auto friendPk = fnd->getPublicKey();
    const auto removeAction = menu.addAction(
        tr("Remove friend", "Menu to remove the friend from our friendlist"));
    connect(
        removeAction, &QAction::triggered, this,
        [=, this]() { emit removeFriend(friendPk); }, Qt::QueuedConnection);
  }

  menu.addSeparator();

  if (!fnd->isFriend()) {
    const auto friendPk = fnd->getPublicKey();
    const auto addAction = menu.addAction("添加好友");
    connect(
        addAction, &QAction::triggered, this,
        [=, this]() { emit addFriend(friendPk); }, Qt::QueuedConnection);
  }

  //  menu.addSeparator();
  //  const auto aboutWindow = menu.addAction(tr("Show details"));
  //  connect(aboutWindow, &QAction::triggered, this,
  //  &MessageSessionWidget::showDetails);

  const auto pos = event->globalPos();
  menu.exec(pos);

  removeEventFilter(this);

  if (!active) {
    setBackgroundRole(QPalette::Window);
  }
}

void MessageSessionWidget::removeChatWindow() { chatRoom->removeFriendFromDialogs(); }

namespace {

std::tuple<CircleWidget *, FriendListWidget *>
getCircleAndFriendList(const Friend *frnd, MessageSessionWidget *fw) {
  const auto pk = frnd->getPublicKey();
  const auto circleId = Settings::getInstance().getFriendCircleID(pk);
  auto circleWidget = CircleWidget::getFromID(circleId);
  auto w = circleWidget ? static_cast<QWidget *>(circleWidget)
                        : static_cast<QWidget *>(fw);
  auto friendList = qobject_cast<FriendListWidget *>(w->parentWidget());
  return std::make_tuple(circleWidget, friendList);
}

} // namespace

void MessageSessionWidget::moveToNewCircle() {
//  const auto frnd = chatRoom->getFriend();
//  CircleWidget *circleWidget;
//  FriendListWidget *friendList;
//  std::tie(circleWidget, friendList) = getCircleAndFriendList(frnd, this);

//  if (circleWidget != nullptr) {
//    circleWidget->updateStatus();
//  }

//  if (friendList != nullptr) {
//    friendList->addCircleWidget(this);
//  } else {
//    const auto pk = frnd->getPublicKey();
//    auto &s = Settings::getInstance();
//    auto circleId = s.addCircle();
//    s.setFriendCircleID(pk, circleId);
//  }
}

void MessageSessionWidget::removeFromCircle() {
  const auto frnd = chatRoom->getFriend();
  CircleWidget *circleWidget;
  FriendListWidget *friendList;
  std::tie(circleWidget, friendList) = getCircleAndFriendList(frnd, this);

  if (friendList != nullptr) {
//    friendList->moveWidget(this, frnd->getStatus(), true);
  } else {
    const auto pk = frnd->getPublicKey();
    auto &s = Settings::getInstance();
    s.setFriendCircleID(pk, -1);
  }

  if (circleWidget != nullptr) {
    circleWidget->updateStatus();
    emit searchCircle(*circleWidget);
  }
}

void MessageSessionWidget::moveToCircle(int newCircleId) {
  const auto frnd = chatRoom->getFriend();
  const auto pk = frnd->getPublicKey();
  const auto oldCircleId = Settings::getInstance().getFriendCircleID(pk);
  auto &s = Settings::getInstance();
  auto oldCircleWidget = CircleWidget::getFromID(oldCircleId);
  auto newCircleWidget = CircleWidget::getFromID(newCircleId);

  if (newCircleWidget) {
//    newCircleWidget->addFriendWidget(this, frnd->getStatus());
//    newCircleWidget->setExpanded(true);
    emit searchCircle(*newCircleWidget);
    s.savePersonal();
  } else {
    s.setFriendCircleID(pk, newCircleId);
  }

  if (oldCircleWidget) {
    oldCircleWidget->updateStatus();
    emit searchCircle(*oldCircleWidget);
  }
}

void MessageSessionWidget::changeAutoAccept(bool enable) {
  if (enable) {
    const auto oldDir = chatRoom->getAutoAcceptDir();
    const auto newDir = QFileDialog::getExistingDirectory(
        Q_NULLPTR, tr("Choose an auto accept directory", "popup title"),
        oldDir);
    chatRoom->setAutoAcceptDir(newDir);
  } else {
    chatRoom->disableAutoAccept();
  }
}
void MessageSessionWidget::showDetails() {
  const auto frnd = chatRoom->getFriend();
  const auto iabout = new AboutFriend(frnd, &Settings::getInstance());
  std::unique_ptr<IAboutFriend> about = std::unique_ptr<IAboutFriend>(iabout);
  const auto aboutUser = new AboutFriendForm(std::move(about), this);
  connect(aboutUser, &AboutFriendForm::histroyRemoved, this,
          &MessageSessionWidget::friendHistoryRemoved);
  aboutUser->show();
}

void MessageSessionWidget::setAsActiveChatroom() { setActive(true); }

void MessageSessionWidget::setAsInactiveChatroom() { setActive(false); }

void MessageSessionWidget::setAvatar(const QPixmap &pixmap) {
  if(pixmap.isNull()){
    return;
  }
  isDefaultAvatar = false;
  avatar->setPixmap(pixmap);
}

void MessageSessionWidget::onSetActive(bool active) {

  if (isDefaultAvatar) {
    const auto uri = active ?
                            QStringLiteral(":img/contact_dark.svg")//
                            : QStringLiteral(":img/contact.svg");
    avatar->setPixmap(QPixmap{uri});
  }
}

void MessageSessionWidget::updateStatusLight() {
  const auto frnd = chatRoom->getFriend();
  const bool event = frnd->getEventFlag();

  if (event) {
    const Settings &s = Settings::getInstance();
    const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
    CircleWidget *circleWidget = CircleWidget::getFromID(circleId);
    if (circleWidget) {
      circleWidget->setExpanded(true);
    }

    emit updateFriendActivity(*frnd);
  }

  statusPic.setMargin(event ? 1 : 3);
  statusPic.setPixmap(QPixmap(Status::getIconPath(frnd->getStatus(), event)));

}

QString MessageSessionWidget::getStatusString() const {
  const auto frnd = chatRoom->getFriend();
  const int status = static_cast<int>(frnd->getStatus());
  const bool event = frnd->getEventFlag();

  static const QVector<QString> names = {
      tr("Online"),
      tr("Away"),
      tr("Busy"),
      tr("Offline"),
  };

  return event ? tr("New message") : names.value(status);
}

const Friend *MessageSessionWidget::getFriend() const { return chatRoom->getFriend(); }

const Contact *MessageSessionWidget::getContact() const { return getFriend(); }

void MessageSessionWidget::search(const QString &searchString, bool hide) {
  const auto frnd = chatRoom->getFriend();
  searchName(searchString, hide);
  const Settings &s = Settings::getInstance();
  const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
  CircleWidget *circleWidget = CircleWidget::getFromID(circleId);
  if (circleWidget) {
    circleWidget->search(searchString);
  }
}

void MessageSessionWidget::resetEventFlags() { chatRoom->resetEventFlags(); }

void MessageSessionWidget::onAvatarSet(const ToxPk &friendPk, const std::string pic) {
  const auto frnd = chatRoom->getFriend();
  if (friendPk != frnd->getPublicKey()) {
    return;
  }
  qDebug() << "MessageSessionWidget::onAvatarSet:" << friendPk.toString()
           << "pic:" << pic.size();
  isDefaultAvatar = false;
  QPixmap pixmap;
  auto f = pixmap.loadFromData(QByteArray::fromStdString(pic));
  if (f) {
    setAvatar(pixmap);
  }
}

void MessageSessionWidget::onAvatarRemoved(const ToxPk &friendPk) {
  const auto frnd = chatRoom->getFriend();
  if (friendPk != frnd->getPublicKey()) {
    return;
  }

  isDefaultAvatar = true;

  const QString path =
      QString(":/img/contact%1.svg").arg(isActive() ? "_dark" : "");
  avatar->setPixmap(QPixmap(path));
}

void MessageSessionWidget::mousePressEvent(QMouseEvent *ev) {
  if (ev->button() == Qt::LeftButton) {
    dragStartPos = ev->pos();
  }

  GenericChatroomWidget::mousePressEvent(ev);
}

void MessageSessionWidget::mouseMoveEvent(QMouseEvent *ev) {
  if (!(ev->buttons() & Qt::LeftButton)) {
    return;
  }

  const int distance = (dragStartPos - ev->pos()).manhattanLength();
  if (distance > QApplication::startDragDistance()) {
    QMimeData *mdata = new QMimeData;
    const Friend *frnd = getFriend();
    mdata->setText(frnd->getDisplayedName());
    mdata->setData("toxPk", frnd->getPublicKey().getByteArray());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mdata);
    drag->setPixmap(avatar->getPixmap());
    drag->exec(Qt::CopyAction | Qt::MoveAction);
  }
}

ContentDialog *MessageSessionWidget::createContentDialog() const {
  qDebug() << __func__;

  ContentDialog *contentDialog = new ContentDialog();
  //  connect(contentDialog, &ContentDialog::friendDialogShown, this,
  //          &Widget::onFriendDialogShown);
  //  connect(contentDialog, &ContentDialog::groupDialogShown, this,
  //          &Widget::onGroupDialogShown);

  auto core = Core::getInstance();

  connect(core, &Core::usernameSet, contentDialog, &ContentDialog::setUsername);

  //  connect(&settings, &Settings::groupchatPositionChanged, &contentDialog,
  //          &ContentDialog::reorderLayouts);
  //  connect(&contentDialog, &ContentDialog::addFriendDialog, this,
  //          &Widget::addFriendDialog);
  //  connect(&contentDialog, &ContentDialog::addGroupDialog, this,
  //          &Widget::addGroupDialog);
  //  connect(&contentDialog, &ContentDialog::connectFriendWidget, this,
  //          &Widget::connectFriendWidget);

#ifdef Q_OS_MAC
  Nexus &n = Nexus::getInstance();
  connect(&contentDialog, &ContentDialog::destroyed, &n,
          &Nexus::updateWindowsClosed);
  connect(&contentDialog, &ContentDialog::windowStateChanged, &n,
          &Nexus::onWindowStateChanged);
  connect(contentDialog.windowHandle(), &QWindow::windowTitleChanged, &n,
          &Nexus::updateWindows);
  n.updateWindows();
#endif

  ContentDialogManager::getInstance()->addContentDialog(*contentDialog);
  return contentDialog;
}

void MessageSessionWidget::setRecvMessage(const FriendMessage &message,
                                  bool isAction) {
  messageDispatcher->onMessageReceived(isAction,message);
}

void MessageSessionWidget::setStatus(Status::Status status) {
  m_friend->setStatus(status);
  updateStatusLight();
}

void MessageSessionWidget::setStatusMsg(const QString &msg) {
  m_friend->setStatusMessage(msg);
  GenericChatroomWidget::setStatusMsg(msg);
}

void MessageSessionWidget::setTyping(bool typing) {
    chatForm->setFriendTyping(typing);

}

void MessageSessionWidget::setName(const QString &name)
{
    chatForm->setName(name);
    m_friend->setName(name);
    GenericChatroomWidget::setName(name);
}
