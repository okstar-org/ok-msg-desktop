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

#include "groupwidget.h"
#include "contentlayout.h"
#include "contentdialogmanager.h"
#include "form/groupchatform.h"
#include "lib/settings/translator.h"
#include "maskablepixmapwidget.h"
#include "src/model/sessionchatlog.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include "src/widget/friendwidget.h"
#include "src/widget/style.h"
#include "src/widget/widget.h"
#include "tool/croppinglabel.h"
#include <QApplication>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDrag>
#include <QMenu>
#include <QMimeData>
#include <QPalette>

GroupWidget::GroupWidget(ContentLayout *layout, QString groupnumber,
                         const GroupId &groupId, const QString &groupName,
                         bool compact)
    : GenericChatroomWidget(compact), contentLayout{layout} {

  settings::Translator::registerHandler(
      std::bind(&GroupWidget::retranslateUi, this), this);

  avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(),
                                         avatar->height()));
  statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Online)));
  statusPic.setMargin(3);

  connect(this, &GroupWidget::chatroomWidgetClicked, [this]() {
    this->do_widgetClicked(this);
    emit groupWidgetClicked(this);
  });

  auto core = Core::getInstance();
  auto &settings = Settings::getInstance();

  group = GroupList::addGroup(
      groupnumber, groupId, groupName, true, ""
      //                                        core->getUsername()
  );

  auto dialogManager = ContentDialogManager::getInstance();
  chatroom = std::make_unique<GroupChatroom>(group, dialogManager);

  //    Group* g = chatroom->getGroup();
  nameLabel->setText(group->getName());

  updateUserCount(group->getPeersCount());
  setAcceptDrops(true);

  connect(group, &Group::titleChanged, this, &GroupWidget::updateTitle);
  connect(group, &Group::numPeersChanged, this, &GroupWidget::updateUserCount);
  connect(nameLabel, &CroppingLabel::editFinished, group, &Group::setName);

  auto messageProcessor = MessageProcessor(sharedMessageProcessorParams);
  messageDispatcher = std::make_unique<GroupMessageDispatcher>(
      *group, std::move(messageProcessor), *core, *core,
      Settings::getInstance());

  chatLog = std::make_unique<SessionChatLog>(*core);

  chatform = std::make_unique<GroupChatForm>(group, *chatLog,
                                             *messageDispatcher, settings);

  contentWidget = new ContentWidget(this);
  contentWidget->hide();
  contentWidget->setGroupChatForm(chatform.get());

  connect(messageDispatcher.get(), &IMessageDispatcher::messageReceived,
          chatLog.get(), &SessionChatLog::onMessageReceived);
  connect(messageDispatcher.get(), &IMessageDispatcher::messageSent,
          chatLog.get(), &SessionChatLog::onMessageSent);
  connect(messageDispatcher.get(), &IMessageDispatcher::messageComplete,
          chatLog.get(), &SessionChatLog::onMessageComplete);

  auto notifyReceivedCallback = [this, groupId](const ToxPk &author, const Message &message) {
      auto isTargeted =
          std::any_of(message.metadata.begin(), message.metadata.end(),
                      [](MessageMetadata metadata) {
                        return metadata.type == MessageMetadataType::selfMention;
                      });
      auto &settings = Settings::getInstance();
      Widget::getInstance()->newGroupMessageAlert(groupId, author, message.content,
                           isTargeted || settings.getGroupAlwaysNotify());
    };

    auto notifyReceivedConnection =
        connect(messageDispatcher.get(), &IMessageDispatcher::messageReceived,
                notifyReceivedCallback);
//    groupAlertConnections.insert(groupId, notifyReceivedConnection);
}

GroupWidget::~GroupWidget() { settings::Translator::unregister(this); }

ContentDialog *GroupWidget::addGroupDialog(Group *group) {

  auto &settings = Settings::getInstance();

  const GroupId &groupId = group->getPersistentId();

  ContentDialog *dialog =
      ContentDialogManager::getInstance()->getGroupDialog(groupId);
  bool separated = settings.getSeparateWindow();
  if (!dialog) {
    dialog = createContentDialog();
  }

  //  GroupWidget *widget = groupWidgets[groupId];
  //  bool isCurrentWindow = activeChatroomWidget == widget;
  //  if (!groupDialog && !separated && isCurrentWindow) {
  //    onAddClicked();
  //  }

  //  auto chatForm = groupChatForms[groupId].data();
  //  auto chatroom = groupChatrooms[groupId];
  ContentDialogManager::getInstance()->addGroupToDialog(
      groupId, dialog, chatroom.get(), chatform.get());
  //
  // #if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
  //  auto removeGroup = QOverload<const GroupId &>::of(&Widget::removeGroup);
  //  auto destroyGroup = QOverload<const GroupId &>::of(&Widget::destroyGroup);
  // #else
  //  auto removeGroup =
  //      static_cast<void (Widget::*)(const GroupId &)>(&Widget::destroyGroup);
  //  auto destroyGroup =
  //      static_cast<void (Widget::*)(const GroupId &)>(&Widget::destroyGroup);
  // #endif
  //  connect(groupWidget, &GroupWidget::removeGroup, this, removeGroup);
  //  connect(groupWidget, &GroupWidget::destroyGroup, this, destroyGroup);
  //  connect(groupWidget, &GroupWidget::chatroomWidgetClicked, chatForm,
  //          &GroupChatForm::focusInput);
  //  connect(groupWidget, &GroupWidget::middleMouseClicked, dialog,
  //          [this]() { dialog->removeGroup(groupId); });
  //  connect(groupWidget, &GroupWidget::chatroomWidgetClicked, chatForm,
  //          &ChatForm::focusInput);
  //  connect(groupWidget, &GroupWidget::newWindowOpened, this,
  //          &Widget::openNewDialog);
  //
  //  // Signal transmission from the created `groupWidget` (which shown in
  //  // ContentDialog) to the `widget` (which shown in main widget)
  //  // FIXME: emit should be removed
  //  connect(groupWidget, &GroupWidget::chatroomWidgetClicked,
  //          [this](GenericChatroomWidget *w) {
  //            Q_UNUSED(w);
  //            emit widget->chatroomWidgetClicked(widget);
  //          });
  //
  //  connect(groupWidget, &GroupWidget::newWindowOpened,
  //          [this](GenericChatroomWidget *w) {
  //            Q_UNUSED(w);
  //            emit widget->newWindowOpened(widget);
  //          });
  //
  //  // FIXME: emit should be removed
  //  emit widget->chatroomWidgetClicked(widget);

  return dialog;
}

ContentDialog *GroupWidget::createContentDialog() const {
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

void GroupWidget::updateTitle(const QString &author, const QString &newName) {
  Q_UNUSED(author);
  nameLabel->setText(newName);
}

void GroupWidget::contextMenuEvent(QContextMenuEvent *event) {
  if (!active) {
    setBackgroundRole(QPalette::Highlight);
  }

  installEventFilter(this); // Disable leave event.

  QMenu menu(this);

  QAction *openChatWindow = nullptr;
  if (chatroom->possibleToOpenInNewWindow()) {
    openChatWindow = menu.addAction(tr("Open chat in new window"));
  }

  QAction *removeChatWindow = nullptr;
  if (chatroom->canBeRemovedFromWindow()) {
    removeChatWindow = menu.addAction(tr("Remove chat from this window"));
  }

  menu.addSeparator();

  QAction *setTitle = menu.addAction(tr("Set title..."));
  QAction *quitGroup =
      menu.addAction(tr("Quit group", "Menu to quit a groupchat"));
  QAction *destroyGrpAct =
      menu.addAction(tr("Destroy group", "Destroy the groupchat"));

  QAction *selectedItem = menu.exec(event->globalPos());

  removeEventFilter(this);

  if (!active) {
    setBackgroundRole(QPalette::Window);
  }

  if (!selectedItem) {
    return;
  }

  if (selectedItem == quitGroup) {
    emit removeGroup(group->getPersistentId());
  } else if (selectedItem == destroyGrpAct) {
    emit destroyGroup(group->getPersistentId());
  } else if (selectedItem == openChatWindow) {
    emit newWindowOpened(this);
  } else if (selectedItem == removeChatWindow) {
    chatroom->removeGroupFromDialogs();
  } else if (selectedItem == setTitle) {
    editName();
  }
}

void GroupWidget::mousePressEvent(QMouseEvent *ev) {
  if (ev->button() == Qt::LeftButton) {
    dragStartPos = ev->pos();
  }

  GenericChatroomWidget::mousePressEvent(ev);
}

void GroupWidget::mouseMoveEvent(QMouseEvent *ev) {
  if (!(ev->buttons() & Qt::LeftButton)) {
    return;
  }

  if ((dragStartPos - ev->pos()).manhattanLength() >
      QApplication::startDragDistance()) {
    QMimeData *mdata = new QMimeData;
    const Group *group = getGroup();
    mdata->setText(group->getName());
    mdata->setData("groupId", group->getPersistentId().getByteArray());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mdata);
    drag->setPixmap(avatar->getPixmap());
    drag->exec(Qt::CopyAction | Qt::MoveAction);
  }
}

void GroupWidget::do_widgetClicked(GenericChatroomWidget *w) {
  //  qDebug() << __func__ << "show group:" << group->getId();
//  contentWidget->showTo(contentLayout);
    showDetails();
}

void GroupWidget::showDetails(){
    const auto group = chatroom->getGroup();
    if(!about){
        qDebug() << "create about for:" << group->getId();
        about = std::make_unique<AboutGroupForm>(group->getPersistentId(), this);
    }

    contentLayout->addWidget(about.get());
    contentLayout->setCurrentWidget(about.get());
}

void GroupWidget::updateUserCount(int numPeers) {
  //    qDebug()<<"updateUserCount group:" << getGroup()->getId() << numPeers;
  //    statusMessageLabel->setText(tr("%n user(s) in chat", "Number of users in
  //    chat", numPeers));
}

void GroupWidget::setAvatar(const QPixmap &pixmap) {
  avatar->setPixmap(pixmap);
}

void GroupWidget::setAsActiveChatroom() { setActive(true); }

void GroupWidget::setAsInactiveChatroom() { setActive(false); }

void GroupWidget::onSetActive(bool active) {
  const auto uri = active ? ":img/group_dark.svg" : ":img/group.svg";
  avatar->setPixmap(
      Style::scaleSvgImage(uri, avatar->width(), avatar->height()));
}

void GroupWidget::updateStatusLight() {
  Group *g = chatroom->getGroup();
  const bool event = g->getEventFlag();
  statusPic.setPixmap(
      QPixmap(Status::getIconPath(Status::Status::Online, event)));
  statusPic.setMargin(event ? 1 : 3);
}

QString GroupWidget::getStatusString() const {
  if (chatroom->hasNewMessage()) {
    return tr("New Message");
  } else {
    return tr("Online");
  }
}

void GroupWidget::editName() { nameLabel->editBegin(); }

// TODO: Remove
Group *GroupWidget::getGroup() const { return chatroom->getGroup(); }

const Contact *GroupWidget::getContact() const { return getGroup(); }

void GroupWidget::resetEventFlags() { chatroom->resetEventFlags(); }

void GroupWidget::dragEnterEvent(QDragEnterEvent *ev) {
  if (!ev->mimeData()->hasFormat("toxPk")) {
    return;
  }
  const ToxPk pk{ev->mimeData()->data("toxPk")};
  if (chatroom->friendExists(pk)) {
    ev->acceptProposedAction();
  }

  if (!active) {
    setBackgroundRole(QPalette::Highlight);
  }
}

void GroupWidget::dragLeaveEvent(QDragLeaveEvent *) {
  if (!active) {
    setBackgroundRole(QPalette::Window);
  }
}

void GroupWidget::dropEvent(QDropEvent *ev) {
  if (!ev->mimeData()->hasFormat("toxPk")) {
    return;
  }
  const ToxPk pk{ev->mimeData()->data("toxPk")};
  if (!chatroom->friendExists(pk)) {
    return;
  }

  chatroom->inviteFriend(pk);

  if (!active) {
    setBackgroundRole(QPalette::Window);
  }
}

void GroupWidget::setName(const QString &name) { nameLabel->setText(name); }

void GroupWidget::retranslateUi() {
  const Group *group = chatroom->getGroup();
  updateUserCount(group->getPeersCount());
}

void GroupWidget::setRecvMessage(const GroupMessage& msg) {

  auto core = Core::getInstance();
  ToxPk author = core->getGroupPeerPk(msg.groupId.getUsername(), msg.from);
  messageDispatcher->onMessageReceived(author, msg.isAction, msg.id, msg.content, msg.nick,msg.from,
                                       msg.timestamp);
}

void GroupWidget::reloadTheme() { chatform->reloadTheme(); }
