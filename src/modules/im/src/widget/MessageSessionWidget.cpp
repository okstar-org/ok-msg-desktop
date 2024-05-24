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

#include <src/chatlog/chatlog.h>

/**
 * @class MessageSessionWidget
 *
 * Widget, which displays brief information about friend.
 * For example, used on friend list.
 * When you click should open the chat with friend. Widget has a context menu.
 */
MessageSessionWidget::MessageSessionWidget(
                           ContentLayout *layout,
                           const ContactId &cId,
                           ChatType chatType)
    : GenericChatroomWidget(chatType, cId),
      contentLayout(layout),
      sendWorker{nullptr},
      contactId(cId)
        {

  qDebug() <<__func__ <<"contactId:"<<cId.toString();

  auto profile = Nexus::getProfile();
  auto core = Core::getInstance();
  auto &settings = Settings::getInstance();
  auto history = profile->getHistory();
  auto dialogManager = ContentDialogManager::getInstance();
  auto widget =  Widget::getInstance();


  if(chatType==ChatType::Chat){
      friendId = ToxPk(contactId);

      sendWorker = std::move( SendWorker::forFriend(friendId));
      connect(sendWorker->dispacher(),&IMessageDispatcher::messageSent,
              this, &MessageSessionWidget::onMessageSent);




  }else if(chatType == ChatType::GroupChat) {
      auto nick = core->getNick();
      groupId = GroupId(contactId);
      groupId.nick = nick;

      sendWorker = std::move(SendWorker::forGroup(groupId));

      connect(sendWorker->dispacher(),&IMessageDispatcher::messageSent,
              this, &MessageSessionWidget::onMessageSent);

//      connect(g, &Group::displayedNameChanged, this,
//              [this](const QString &newName) {
//                setName(newName);
//              });
  }


  auto chatForm = sendWorker->getChatForm();

  contentWidget = std::make_unique<ContentWidget>(this);
  contentWidget->hide();
  contentWidget->setObjectName(QString("ContentWidget:%1").arg(contactId.toString()));
  contentWidget->setChatForm(chatForm);

  connect(chatForm->getChatLog(), &ChatLog::readAll, this, [&](){
    //已经阅读完消息，清除信号灯
      if(contentWidget->isVisible()){
          clearStatusLight();
      }
  });

  //  const auto compact = settings.getCompactLayout();

//  const auto activityTime = settings.getFriendActivity(toxPk);
//  const auto chatTime = sendWorker->getChatForm()->getLatestTime();
//  if (chatTime > activityTime && chatTime.isValid()) {
//    settings.setFriendActivity(toxPk, chatTime);
//  }

//  chatRoom = std::make_unique<FriendChatroom>(m_friend, dialogManager);
//  auto frnd = chatRoom->getFriend();

//  nameLabel->setText(getContact()->getDisplayedName());;

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

//    connect(getContact(), &Contact::avatarChanged,
//            [&](auto& pic) {
//              setAvatar(pic);
//            });



}

MessageSessionWidget::~MessageSessionWidget()
{
    qDebug()<<__func__;

}

void MessageSessionWidget::do_widgetClicked() {
//    qDebug() << __func__ << "contactId:" << contactId.toString();
    contentWidget->showTo(contentLayout);
}

void MessageSessionWidget::showEvent(QShowEvent *e)
{

    GenericChatroomWidget::showEvent(e);

    if(isGroup()){
        //获取名称
        auto group = GroupList::findGroup(GroupId{contactId.toString()});
        if(group){
            setName(group->getName());
        }
    }else{
         auto f = FriendList::findFriend(contactId);
        if(f){
            setContact(*f);
        }

        auto msgs = sendWorker->getLastTextMessage();
        for(auto m : msgs){
            updateLastMessage(m);
            break;
        }
    }
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
    emit deleteSession(contactId.toString());
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

void MessageSessionWidget::onMessageSent(DispatchedMessageId id, const Message &message)
{
    updateLastMessage(message);
}

void MessageSessionWidget::setFriend(const Friend *f)
{
    qDebug() << __func__ << f;
    if(!f){
        return;
    }

    auto chatForm = (ChatForm*) sendWorker->getChatForm();
    chatForm->setContact(f);
    setContact(*f);

    connect(f, &Friend::displayedNameChanged, this, [&](const QString& name){
        setName(name);
    });

    connect(f, &Friend::statusChanged, this, [this](Status::Status status, bool event) {
        setStatus(status, event);
    });

    connect(f, &Friend::avatarChanged, this, [this](const QPixmap& avatar) {
        setAvatar(avatar);
    });
}

void MessageSessionWidget::removeFriend()
{
    auto chatForm = (ChatForm*) sendWorker->getChatForm();
    chatForm->removeContact();
    removeContact();
}



void MessageSessionWidget::setAsActiveChatroom() { setActive(true); }

void MessageSessionWidget::setAsInactiveChatroom() { setActive(false); }


void MessageSessionWidget::onActiveSet(bool active) {

}

QString MessageSessionWidget::getStatusString() const {
    qDebug() <<__func__;
//  auto contactId = sendWorker->getChatroom()->getContactId();
//  auto frnd = FriendList::findFriend(ToxPk(contact));

//  const int status = static_cast<int>(frnd->getStatus());
//  const bool event = frnd->getEventFlag();

//  static const QVector<QString> names = {
//      tr("Online"),
//      tr("Away"),
//      tr("Busy"),
//      tr("Offline"),
//  };

//  return event ? tr("New message") : names.value(status);

    return {};
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

  if(!pic.isNull()){

      setAvatar(pic);

    }
//  auto c = getContact();
//  if(c){
//      c->setAvatar(pic);
//  }



}

void MessageSessionWidget::onAvatarRemoved(const ToxPk &friendPk) {
    qDebug() << __func__ << friendPk.toString();
  // 清空联系人头像
//  auto c = getContact();
//  c->clearAvatar();


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

void MessageSessionWidget::setRecvMessage(const FriendMessage &msg, bool isAction) {

  FriendMessage m = msg;
  m.from = ContactId(m.from).toString();

  auto frd = FriendList::findFriend(contactId);
  if(frd)
  {
        m.displayName = frd->getDisplayedName();
  }

  auto md= (FriendMessageDispatcher*)sendWorker->dispacher();
  md->onMessageReceived(m);

  updateLastMessage(m);

  auto vis = contentWidget->isVisible();
  if(!vis){
      //更新状态信号灯
      updateStatusLight(Status::Status::Online, true);
      //聊天界面不显示，消息提示。
      Widget::getInstance()->newGroupMessageAlert(GroupId(contactId), ToxPk(msg.from), msg.content, true);
  }
}

void MessageSessionWidget::setMessageReceipt(const ReceiptNum &receipt)
{
    auto md= (FriendMessageDispatcher*)sendWorker->dispacher();
    md->onReceiptReceived(receipt);
}

void MessageSessionWidget::setRecvGroupMessage(const GroupMessage &msg)
{
    GroupMessage m = msg;
    m.from = ContactId(m.from).toString();

    auto frd = FriendList::findFriend(contactId);
    if(frd)
    {
          m.displayName = frd->getDisplayedName();
    }

    auto md= (GroupMessageDispatcher*)sendWorker->dispacher();
    md->onMessageReceived(m);

    updateLastMessage(m);

    auto vis = contentWidget->isVisible();
    if(!vis){
        //更新状态信号灯
        updateStatusLight(Status::Status::Online, true);
        //聊天界面不显示，消息提示。
        Widget::getInstance()->newGroupMessageAlert(GroupId(contactId), ToxPk(msg.from), msg.content, true);
    }
}

void MessageSessionWidget::setFileReceived(const ToxFile &file)
{
    qDebug() << __func__ <<file.fileName;
//    auto cl = sendWorker->getChatLog();
//    cl->onMessageReceived(ToxPk(contactId), file);

    auto md= (FriendMessageDispatcher*)sendWorker->dispacher();
    md->onFileReceived(file);

}

void MessageSessionWidget::clearHistory()
{
    sendWorker->clearHistory();
}

void MessageSessionWidget::setStatus(Status::Status status, bool event) {
    updateStatusLight(status, event);
    auto f = FriendList::findFriend(contactId);
    if(!f){
        qWarning() <<"friend is no existing.";
        return;
    }
    f->setStatus(status);
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
    qDebug() << __func__ <<"friend:" << contactId.toString()<<"name:" << name;
    GenericChatroomWidget::setName(name);
}
