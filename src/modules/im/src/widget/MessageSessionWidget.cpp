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

#include <src/nexus.h>

/**
 * @class MessageSessionWidget
 *
 * Widget, which displays brief information about friend.
 * For example, used on friend list.
 * When you click should open the chat with friend. Widget has a context menu.
 */
MessageSessionWidget::MessageSessionWidget(
                           ContentLayout *layout,
                           const ToxPk &toxPk,
                           ChatType chatType)
    : GenericChatroomWidget(chatType, toxPk),
      contentLayout(layout),
      sendWorker{nullptr},
      isDefaultAvatar{true} {

  qDebug() <<__func__ <<"friend:"<<toxPk.toString();

  avatar->setPixmap(QPixmap(":/img/contact.svg"));
  statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Offline)));
  statusPic.setMargin(3);

  auto profile = Nexus::getProfile();
  auto core = Core::getInstance();
  auto &settings = Settings::getInstance();
  auto history = profile->getHistory();
  auto dialogManager = ContentDialogManager::getInstance();


  if(chatType==ChatType::Chat){
      auto f = FriendList::addFriend(toxPk, true);


      connect(f, &Friend::displayedNameChanged, this,
              [this](const QString &newName) {
                setName(newName);
              });

      connect(f, &Friend::statusChanged, this,
              [this](Status::Status status, bool event) {
                setStatus(status, event);
              });

      sendWorker = std::move( SendWorker::forFriend(*f));
  }else if(chatType == ChatType::GroupChat) {
      auto g = GroupList::addGroup(GroupId(toxPk), "", true, toxPk.resource);
      sendWorker = std::move(SendWorker::forGroup(*g));
  }


  contentWidget = std::make_unique<ContentWidget>(this);
  contentWidget->hide();
  contentWidget->setChatForm(sendWorker->getChatForm());

  //  const auto compact = settings.getCompactLayout();

  const auto activityTime = settings.getFriendActivity(toxPk);
  const auto chatTime = sendWorker->getChatForm()->getLatestTime();
  if (chatTime > activityTime && chatTime.isValid()) {
    settings.setFriendActivity(toxPk, chatTime);
  }

//  chatRoom = std::make_unique<FriendChatroom>(m_friend, dialogManager);
//  auto frnd = chatRoom->getFriend();

  nameLabel->setText(toxPk.username);

  // update alias when edited
//  connect(nameLabel, &CroppingLabel::editFinished, //
//          m_friend, &Friend::setAlias);

//  connect(m_friend, &Friend::displayedNameChanged, //
//          nameLabel, &CroppingLabel::setText);

//  connect(m_friend, &Friend::displayedNameChanged, this,
//          [this](const QString &newName) {
//            Q_UNUSED(newName);
//            emit widgetRenamed(this);
//          });

//  connect(sendWorker->getChatroom(),
//          &Chatroom::activeChanged,
//          this,
//          &MessageSessionWidget::setActive);
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

}

MessageSessionWidget::~MessageSessionWidget()
{
    qDebug()<<__func__;

}

void MessageSessionWidget::do_widgetClicked() {
//    qDebug() << __func__ << "contactId:" << contactId.toString();
    contentWidget->showTo(contentLayout);
}

void MessageSessionWidget::showEvent(QShowEvent *)
{
    auto core = Nexus::getCore();
    auto status= core->getFriendStatus(contactId.toString());
    setStatus(status, false);
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


  auto chatRoom = sendWorker->getChatroom();


  QMenu menu;
//  menu.addSeparator();
  auto deleteAct = menu.addAction(tr("Delete the session"));
  //  inviteMenu->setEnabled(chatRoom->canBeInvited());


  connect(deleteAct, &QAction::triggered,
          this, &MessageSessionWidget::removeChat);
//  inviteMenu->addSeparator();

//  for (const auto &group : chatRoom->getGroups()) {
//    const auto groupAction =
//        inviteMenu->addAction(tr("Invite to group '%1'").arg(group.name));
//    connect(groupAction, &QAction::triggered,
//            [=, this]() { chatRoom->inviteFriend(group.group); });
//  }
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

//  const auto setAlias = menu.addAction(tr("Set alias..."));
//  connect(setAlias, &QAction::triggered, nameLabel, &CroppingLabel::editBegin);

  //  自动接收文件
  //  menu.addSeparator();
  //  auto autoAccept = menu.addAction(
  //      tr("Auto accept files from this friend", "context menu entry"));
  //  autoAccept->setCheckable(true);
  //  autoAccept->setChecked(!chatRoom->autoAcceptEnabled());
  //  connect(autoAccept, &QAction::triggered, this,
  //  &MessageSessionWidget::changeAutoAccept);

//  menu.addSeparator();

//  auto fnd = chatRoom->getFriend();
//  if (chatRoom->friendCanBeRemoved()) {
//    const auto friendPk = fnd->getPublicKey();
//    const auto removeAction = menu.addAction(
//        tr("Remove friend", "Menu to remove the friend from our friendlist"));
//    connect(
//        removeAction, &QAction::triggered, this,
//        [=, this]() { emit removeFriend(friendPk); }, Qt::QueuedConnection);
//  }

//  menu.addSeparator();

//  if (!fnd->isFriend()) {
//    const auto friendPk = fnd->getPublicKey();
//    const auto addAction = menu.addAction("添加好友");
//    connect(
//        addAction, &QAction::triggered, this,
//        [=, this]() { emit addFriend(friendPk); }, Qt::QueuedConnection);
//  }

//    menu.addSeparator();
    const auto aboutWindow = menu.addAction(tr("Show details"));
    connect(aboutWindow, &QAction::triggered, this,
    &MessageSessionWidget::showDetails);

  const auto pos = event->globalPos();
  menu.exec(pos);

  removeEventFilter(this);

  if (!active) {
    setBackgroundRole(QPalette::Window);
  }
}

void MessageSessionWidget::removeChat() {
    emit deleteWidget(this);
}

//namespace {

//std::tuple<CircleWidget *, FriendListWidget *>
//getCircleAndFriendList(const Friend *frnd, MessageSessionWidget *fw) {
//  const auto pk = frnd->getPublicKey();
//  const auto circleId = Settings::getInstance().getFriendCircleID(pk);
//  auto circleWidget = CircleWidget::getFromID(circleId);
//  auto w = circleWidget ? static_cast<QWidget *>(circleWidget)
//                        : static_cast<QWidget *>(fw);
//  auto friendList = qobject_cast<FriendListWidget *>(w->parentWidget());
//  return std::make_tuple(circleWidget, friendList);
//}

//} // namespace

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
//  const auto frnd = chatRoom->getFriend();
//  CircleWidget *circleWidget;
//  FriendListWidget *friendList;
//  std::tie(circleWidget, friendList) = getCircleAndFriendList(frnd, this);

//  if (friendList != nullptr) {
//    friendList->moveWidget(this, frnd->getStatus(), true);
//  } else {
//    const auto pk = frnd->getPublicKey();
//    auto &s = Settings::getInstance();
//    s.setFriendCircleID(pk, -1);
//  }

//  if (circleWidget != nullptr) {
//    circleWidget->updateStatus();
//    emit searchCircle(*circleWidget);
//  }
}

void MessageSessionWidget::moveToCircle(int newCircleId) {
//  const auto frnd = getFriend();
//  const auto pk = frnd->getPublicKey();
//  const auto oldCircleId = Settings::getInstance().getFriendCircleID(pk);
//  auto &s = Settings::getInstance();
//  auto oldCircleWidget = CircleWidget::getFromID(oldCircleId);
//  auto newCircleWidget = CircleWidget::getFromID(newCircleId);

//  if (newCircleWidget) {
//    newCircleWidget->addFriendWidget(this, frnd->getStatus());
//    newCircleWidget->setExpanded(true);
//    emit searchCircle(*newCircleWidget);
//    s.savePersonal();
//  } else {
//    s.setFriendCircleID(pk, newCircleId);
//  }

//  if (oldCircleWidget) {
//    oldCircleWidget->updateStatus();
//    emit searchCircle(*oldCircleWidget);
//  }
}

void MessageSessionWidget::changeAutoAccept(bool enable) {
//    auto chatRoom = sendWorker->getChatroom();
//    if (enable) {
//    const auto oldDir = chatRoom->getAutoAcceptDir();
//    const auto newDir = QFileDialog::getExistingDirectory(
//        Q_NULLPTR, tr("Choose an auto accept directory", "popup title"),
//        oldDir);
//    chatRoom->setAutoAcceptDir(newDir);
//  } else {
//    chatRoom->disableAutoAccept();
//  }
}

void MessageSessionWidget::showDetails() {
//  const auto frnd = chatRoom->getFriend();
//  const auto iabout = new AboutFriend(frnd, &Settings::getInstance());
//  std::unique_ptr<IAboutFriend> about = std::unique_ptr<IAboutFriend>(iabout);
//  const auto aboutUser = new AboutFriendForm(std::move(about), this);
//  connect(aboutUser, &AboutFriendForm::histroyRemoved, this,
//          &MessageSessionWidget::friendHistoryRemoved);
//  aboutUser->show();

auto w = Widget::getInstance();
if(w){
  emit w->toShowDetails(getContactId());
}
}

void MessageSessionWidget::setAsActiveChatroom() { setActive(true); }

void MessageSessionWidget::setAsInactiveChatroom() { setActive(false); }

void MessageSessionWidget::setAvatar(const QPixmap &pixmap) {
  if(pixmap.isNull()){
    return;
  }
  isDefaultAvatar = false;
  avatar->setPixmap(pixmap);

  if(!isGroup()){
       auto frnd = FriendList::findFriend(ToxPk(contactId));
       frnd->setAvatar(pixmap);
  }
}

void MessageSessionWidget::onSetActive(bool active) {

  if (isDefaultAvatar) {
    const auto uri = active ?
                            QStringLiteral(":img/contact_dark.svg")//
                            : QStringLiteral(":img/contact.svg");
    avatar->setPixmap(QPixmap{uri});
  }
}

void MessageSessionWidget::updateStatusLight(Status::Status status, bool event) {

//  const auto frnd = chatRoom->getFriend();
//  const bool event = frnd->getEventFlag();

//  if (event) {
//    const Settings &s = Settings::getInstance();
//    const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
//    CircleWidget *circleWidget = CircleWidget::getFromID(circleId);
//    if (circleWidget) {
//      circleWidget->setExpanded(true);
//    }
//    emit updateFriendActivity(*frnd);
//  }

//  statusPic.setMargin(event ? 1 : 3);
  statusPic.setPixmap(QPixmap(Status::getIconPath(status, event)));

}

QString MessageSessionWidget::getStatusString() const {
  auto contact = sendWorker->getChatroom()->getContact();
  auto frnd = FriendList::findFriend(ToxPk(contact->getId()));

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

//Friend *MessageSessionWidget::getFriend() const {
//    auto contact = sendWorker->getChatroom()->getContact();
//    auto cid = contact->getId();

//    auto frnd = FriendList::findFriend(ToxPk(cid));
//    return frnd;
//}

//const ContactId& MessageSessionWidget::getContactId() const { return contactId; }

void MessageSessionWidget::search(const QString &searchString, bool hide) {
//  const auto frnd = chatRoom->getFriend();
//  searchName(searchString, hide);
//  const Settings &s = Settings::getInstance();
//  const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
//  CircleWidget *circleWidget = CircleWidget::getFromID(circleId);
//  if (circleWidget) {
//    circleWidget->search(searchString);
//  }
}

void MessageSessionWidget::resetEventFlags() {
//    getFriend()->setEventFlag(false);
}

void MessageSessionWidget::onAvatarSet(const ToxPk &friendPk, const QPixmap& pic) {
//  const auto frnd =  getFriend();;
//  if (friendPk != frnd->getPublicKey()) {
//    return;
//  }
  qDebug() <<__func__<< "onAvatarSet:" << friendPk.toString()
           << "pic:" << pic.size();
  isDefaultAvatar = false;

  if (!pic.isNull()) {
    setAvatar(pic);
  }
}

void MessageSessionWidget::onAvatarRemoved(const ToxPk &friendPk) {


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
//    const Friend *frnd = getFriend();
//    mdata->setText(frnd->getDisplayedName());
//    mdata->setData("toxPk", frnd->getPublicKey().getByteArray());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mdata);
    drag->setPixmap(avatar->getPixmap());
    drag->exec(Qt::CopyAction | Qt::MoveAction);
  }
}

void MessageSessionWidget::setRecvMessage(const FriendMessage &message,
                                          bool isAction) {
  FriendMessageDispatcher* fmd= (FriendMessageDispatcher*)sendWorker->dispacher();
  fmd->onMessageReceived(isAction,message);
}

void MessageSessionWidget::setStatus(Status::Status status, bool event) {
  updateStatusLight(status, event);
}

void MessageSessionWidget::setStatusMsg(const QString &msg) {
//  m_friend->setStatusMessage(msg);
  GenericChatroomWidget::setStatusMsg(msg);
}

void MessageSessionWidget::setTyping(bool typing) {
    auto chatForm =(ChatForm*) sendWorker->getChatForm();
    chatForm->setFriendTyping(typing);
}

void MessageSessionWidget::setName(const QString &name)
{
    auto chatForm =(ChatForm*) sendWorker->getChatForm();
    chatForm->setName(name);
    GenericChatroomWidget::setName(name);
}
