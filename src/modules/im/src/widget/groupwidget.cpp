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
#include <QApplication>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDrag>
#include <QMenu>
#include <QMimeData>
#include <QPalette>
#include "contentdialogmanager.h"
#include "contentlayout.h"
#include "form/groupchatform.h"
#include "groupwidget.h"
#include "gui.h"
#include "lib/settings/translator.h"
#include "maskablepixmapwidget.h"
#include "src/lib/settings/style.h"
#include "src/model/sessionchatlog.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include "src/widget/friendwidget.h"
#include "src/widget/widget.h"
#include "tool/croppinglabel.h"

GroupWidget::GroupWidget(ContentLayout* layout, QString groupnumber, const GroupId& groupId,
                         const QString& groupName, bool compact)
        : GenericChatroomWidget(ChatType::GroupChat, groupId)
        , contentLayout{layout}
        , quitGroup{nullptr}
        , destroyGrpAct{nullptr}
        , about{nullptr} {
    setAcceptDrops(true);

    connect(this, &GroupWidget::removeGroup, this, &GroupWidget::do_removeGroup);
    connect(this, &GroupWidget::destroyGroup, this, &GroupWidget::do_destroyGroup);

    auto core = Core::getInstance();
    auto& settings = Settings::getInstance();

    group = GroupList::addGroup(groupId, groupName, true, core->getUsername());

    auto dialogManager = ContentDialogManager::getInstance();

    nameLabel->setText(group->getDisplayedName());

    updateUserCount(group->getPeersCount());
    connect(group, &Group::subjectChanged, this, &GroupWidget::updateTitle);
    connect(group, &Group::peerCountChanged, this, &GroupWidget::updateUserCount);
    connect(group, &Group::descChanged, this, &GroupWidget::updateDesc);
    connect(group, &Group::privilegesChanged, this, &GroupWidget::do_privilegesChanged);

    //  connect(nameLabel, &CroppingLabel::editFinished, group, &Group::setName);
    connect(group, &Group::displayedNameChanged, [&](auto& newName) { setName(newName); });

    connect(this, &GroupWidget::chatroomWidgetClicked, [this]() {
        this->do_widgetClicked(this);
        emit groupWidgetClicked(this);
    });

    settings::Translator::registerHandler([this] { retranslateUi(); }, this);
    retranslateUi();

    init();

    emit Widget::getInstance() -> groupAdded(group);

    //    groupAlertConnections.insert(groupId, notifyReceivedConnection);
}

GroupWidget::~GroupWidget() {
    qDebug() << __func__;
    emit Widget::getInstance() -> groupRemoved(group);
    settings::Translator::unregister(this);
}

void GroupWidget::init() {
    menu = new QMenu(this);

    //  QAction *openChatWindow = nullptr;
    //  if (chatroom->possibleToOpenInNewWindow()) {
    //    openChatWindow = menu.addAction(tr("Open chat in new window"));
    //  }

    //  QAction *removeChatWindow = nullptr;
    //  if (chatroom->canBeRemovedFromWindow()) {
    //    removeChatWindow = menu.addAction(tr("Remove chat from this window"));
    //  }

    //    menu.addSeparator();

    //  QAction *setSubject = menu.addAction(tr("Set title..."));
    quitGroup = menu->addAction(tr("Quit group", "Menu to quit a groupchat"));
}

ContentDialog* GroupWidget::addGroupDialog(Group* group) {
    auto& settings = Settings::getInstance();

    auto& groupId = group->getPersistentId();

    ContentDialog* dialog = ContentDialogManager::getInstance()->getGroupDialog(GroupId(groupId));
    //  bool separated = settings.getSeparateWindow();
    //  if (!dialog) {
    //    dialog = createContentDialog();
    //  }

    //  GroupWidget *widget = groupWidgets[groupId];
    //  bool isCurrentWindow = activeChatroomWidget == widget;
    //  if (!groupDialog && !separated && isCurrentWindow) {
    //    onAddClicked();
    //  }

    //  auto chatForm = groupChatForms[groupId].data();
    //  auto chatroom = groupChatrooms[groupId];
    //  ContentDialogManager::getInstance()->addGroupToDialog(
    //      groupId, dialog, chatroom.get(), chatform.get());
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

ContentDialog* GroupWidget::createContentDialog() const {
    qDebug() << __func__;

    ContentDialog* contentDialog = new ContentDialog();
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


// #ifdef Q_OS_MAC
//   Nexus &n = Nexus::getInstance();
//   connect(&contentDialog, &ContentDialog::destroyed, &n,
//           &Nexus::updateWindowsClosed);
//   connect(&contentDialog, &ContentDialog::windowStateChanged, &n,
//           &Nexus::onWindowStateChanged);
//   connect(contentDialog.windowHandle(), &QWindow::windowTitleChanged, &n,
//           &Nexus::updateWindows);
//   n.updateWindows();
// #endif


    ContentDialogManager::getInstance()->addContentDialog(*contentDialog);
    return contentDialog;
}

void GroupWidget::updateTitle(const QString& author, const QString& newName) { Q_UNUSED(author); }

void GroupWidget::contextMenuEvent(QContextMenuEvent* event) {
    if (!active) {
        setBackgroundRole(QPalette::Highlight);
    }

    // Disable leave event.
    installEventFilter(this);
    QAction* selectedItem = menu->exec(event->globalPos());
    removeEventFilter(this);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }

    if (!selectedItem) {
        return;
    }

    if (selectedItem == quitGroup) {
        const bool retYes =
                GUI::askQuestion(tr("Confirmation"),
                                 tr("Are you sure to quit %1 chat group?").arg(getName()),
                                 /* defaultAns = */ false,
                                 /* warning = */ true,
                                 /* yesno = */ true);
        if (!retYes) {
            return;
        }
        emit removeGroup(group->getPersistentId());
    } else if (selectedItem == destroyGrpAct) {
        emit destroyGroup(group->getPersistentId());
    }
    /*else if (selectedItem == openChatWindow) {
      emit newWindowOpened(this);
    } else if (selectedItem == removeChatWindow) {
      chatroom->removeGroupFromDialogs();
    } else if (selectedItem == setSubject) {
      editName();
    }*/
}

void GroupWidget::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::LeftButton) {
        dragStartPos = ev->pos();
    }

    GenericChatroomWidget::mousePressEvent(ev);
}

void GroupWidget::mouseMoveEvent(QMouseEvent* ev) {
    if (!(ev->buttons() & Qt::LeftButton)) {
        return;
    }

    if ((dragStartPos - ev->pos()).manhattanLength() > QApplication::startDragDistance()) {
        QMimeData* mdata = new QMimeData;
        const Group* group = getGroup();
        mdata->setText(group->getName());
        mdata->setData("groupId", group->getPersistentId().getByteArray());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void GroupWidget::do_widgetClicked(GenericChatroomWidget* w) { showDetails(); }

void GroupWidget::updateDesc(const QString&) {}

void GroupWidget::do_removeGroup(const GroupId& groupId) {
    qDebug() << __func__ << groupId.toString();
    auto core = Core::getInstance();
    core->leaveGroup(groupId.toString());
}

void GroupWidget::do_destroyGroup(const GroupId& groupId) {}

void GroupWidget::do_privilegesChanged(const Group::Role& role, const Group::Affiliation& aff,
                                       const QList<int> codes) {
    if (aff == Group::Affiliation::Owner) {
        if (!destroyGrpAct) {
            destroyGrpAct = menu->addAction(tr("Destroy group", "Destroy the groupchat"));
        }
    } else {
        menu->removeAction(destroyGrpAct);
        destroyGrpAct = nullptr;
    }
}

void GroupWidget::showDetails() {
    if (!about) {
        about = new AboutGroupForm(group->getPersistentId(), this);
        contentLayout->addWidget(about);
    }
    contentLayout->setCurrentWidget(about);
}

void GroupWidget::updateUserCount(int numPeers) {
    //    qDebug()<<"updateUserCount group:" << getGroup()->getId() << numPeers;
    //    statusMessageLabel->setText(tr("%n user(s) in chat", "Number of users in
    //    chat", numPeers));
    //    const auto group = chatroom->getGroup();
    //    if(about){
    //      about->set
    //    }
}

void GroupWidget::setAsActiveChatroom() { setActive(true); }

void GroupWidget::setAsInactiveChatroom() { setActive(false); }

void GroupWidget::onActiveSet(bool active) {
    //  const auto uri = active ? ":img/group_dark.svg" : ":img/group.svg";
    //  avatar->setPixmap(Style::scaleSvgImage(uri, avatar->width(), avatar->height()));
}

void GroupWidget::updateStatusLight(Status::Status status, bool event) {
    //  const Group *g = chatroom->getGroup();
    //  if(statusPic){
    //    statusPic->setPixmap(QPixmap(Status::getIconPath(status, event)));
    //    statusPic->setMargin(event ? 1 : 3);
    //  }
}

QString GroupWidget::getStatusString() const {
    //  if (chatroom->hasNewMessage()) {
    //    return tr("New Message");
    //  } else {
    //    return tr("Online");
    //  }
    return {};
}

void GroupWidget::editName() { nameLabel->editBegin(); }

void GroupWidget::resetEventFlags() {}

void GroupWidget::dragEnterEvent(QDragEnterEvent* ev) {
    //  if (!ev->mimeData()->hasFormat("toxPk")) {
    //    return;
    //  }
    //  const ToxPk pk{ev->mimeData()->data("toxPk")};
    //  if (chatroom->friendExists(pk)) {
    //    ev->acceptProposedAction();
    //  }

    //  if (!active) {
    //    setBackgroundRole(QPalette::Highlight);
    //  }
}

void GroupWidget::dragLeaveEvent(QDragLeaveEvent*) {
    if (!active) {
        setBackgroundRole(QPalette::Window);
    }
}

void GroupWidget::dropEvent(QDropEvent* ev) {
    //  if (!ev->mimeData()->hasFormat("toxPk")) {
    //    return;
    //  }
    //  const ToxPk pk{ev->mimeData()->data("toxPk")};
    //  if (!chatroom->friendExists(pk)) {
    //    return;
    //  }

    //  chatroom->inviteFriend(pk);

    //  if (!active) {
    //    setBackgroundRole(QPalette::Window);
    //  }
}

void GroupWidget::retranslateUi() { updateUserCount(group->getPeersCount()); }

void GroupWidget::reloadTheme() {
    if (about) {
        // TODO about->reloadTheme();
    }
}
