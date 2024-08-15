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

#include "friendwidget.h"
#include "gui.h"

#include "circlewidget.h"
#include "contentdialogmanager.h"
#include "contentlayout.h"
#include "friendlistwidget.h"
#include "groupwidget.h"
#include "maskablepixmapwidget.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/lib/settings/style.h"
#include "src/model/aboutfriend.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/form/aboutfriendform.h"
#include "src/widget/form/chatform.h"
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
#include <QPainter>
#include <QStyleOption>

#include "form/chatform.h"
#include "src/model/chathistory.h"
#include "src/persistence/profile.h"
#include "src/widget/form/GroupCreateForm.h"

/**
 * @class FriendWidget
 *
 * Widget, which displays brief information about friend.
 * For example, used on friend list.
 * When you click should open the chat with friend. Widget has a context menu.
 */
FriendWidget::FriendWidget(ContentLayout* layout, const FriendInfo& f, QWidget* parent)
        : GenericChatroomWidget(ChatType::Chat, f.id, parent)
        , contentLayout(layout)
        , menu{nullptr}
        , about{nullptr} {
    qDebug() << __func__ << "friend:" << f.toString();

    auto profile = Nexus::getProfile();

    auto core = Core::getInstance();
    auto& settings = Settings::getInstance();
    auto history = profile->getHistory();
    auto dialogManager = ContentDialogManager::getInstance();

    m_friend = core->getFriendList().findFriend(f.getId());
    nameLabel->setText(m_friend->getDisplayedName());

    // update alias when edited
    connect(nameLabel, &CroppingLabel::editFinished,  //
            m_friend, &Friend::setAlias);

    // update on changes of the displayed name
    connect(m_friend, &Friend::displayedNameChanged,  //
            nameLabel, &CroppingLabel::setText);

    connect(m_friend, &Friend::displayedNameChanged, this, [this](const QString& newName) {
        Q_UNUSED(newName);
        emit friendWidgetRenamed(this);
    });

    //  connect(getContact(), &Contact::avatarChanged,
    //          [&](auto& pic) {
    //            setAvatar(pic);
    //          });

    //  connect(chatRoom.get(), &Chatroom::activeChanged, this,
    //          &FriendWidget::setActive);
    //  statusMessageLabel->setTextFormat(Qt::PlainText);

    //  connect(friendWidget, &FriendWidget::copyFriendIdToClipboard, this,
    //          &Widget::copyFriendIdToClipboard);

    // Signal transmission from the created `friendWidget` (which shown in
    // ContentDialog) to the `widget` (which shown in main widget)

    //
    connect(this, &FriendWidget::chatroomWidgetClicked, [=, this](GenericChatroomWidget* w) {
        Q_UNUSED(w);
        do_widgetClicked(this);
        emit friendWidgetClicked(this);
    });
    //  connect(friendWidget, &FriendWidget::newWindowOpened,
    //          [this](GenericChatroomWidget *w) {
    //            Q_UNUSED(w);
    //            emit friendWidget->newWindowOpened(friendWidget);
    //          });

    statusPic->hide();

    // Try to get the avatar from the cache
    //  QPixmap avatar = Nexus::getProfile()->loadAvatar(friendPk);
    //  if (!avatar.isNull()) {
    //    sendWorker->onAvatarChanged(friendPk, avatar);
    //    setAvatar(avatar);
    //  }

    menu = new QMenu(this);
    inviteToGrp = menu->addAction(tr("Invite to group"));
    newGroupAction = menu->addAction(tr("To new group"));

    connect(newGroupAction, &QAction::triggered, this, &FriendWidget::inviteToNewGroup);

    //  inviteMenu->addSeparator();

    menu->addSeparator();
    removeAct = menu->addAction(tr("Remove friend"));

    init();
}

FriendWidget::~FriendWidget() {
    qDebug() << __func__;
    removeDetails();
    deinit();
}

void FriendWidget::init() {}

void FriendWidget::deinit() {}

void FriendWidget::do_widgetClicked(GenericChatroomWidget* w) {
    qDebug() << __func__ << "show friend:" << m_friend->getId().toString();
    showDetails();
}

ContentDialog* FriendWidget::addFriendDialog(const Friend* frnd) {
    QString friendId = frnd->getId().toString();
    qDebug() << __func__ << friendId;

    const FriendId& friendPk = frnd->getPublicKey();
    qDebug() << "friendPk" << friendPk.toString();

    ContentDialog* dialog = ContentDialogManager::getInstance()->getFriendDialog(friendPk);
    qDebug() << "Find contentDialog:" << dialog;
    if (!dialog) {
        dialog = createContentDialog();
    }

    auto& settings = Settings::getInstance();

    //  FriendWidget *widget = friendWidgets[friendPk];
    //  bool isCurrent = activeChatroomWidget == widget;
    //  if (!contentDialog && !isSeparate && isCurrent) {
    //    onAddClicked();
    //  }

    //  ContentDialogManager::getInstance()->addFriendToDialog(
    //      friendPk, dialog, chatRoom.get(), chatForm.get());

    //  friendWidget->setStatusMsg(widget->getStatusMsg());

    // #if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    //   auto widgetRemoveFriend = QOverload<const ToxPk
    //   &>::of(&Widget::removeFriend);
    // #else
    //   auto widgetRemoveFriend =
    //       static_cast<void (Widget::*)(const ToxPk &)>(&Widget::removeFriend);
    // #endif
    //   connect(friendWidget, &FriendWidget::removeFriend, this,
    //   widgetRemoveFriend); connect(friendWidget, &FriendWidget::addFriend,
    //   this, &Widget::addFriend0);

    // FIXME: emit should be removed
    //  emit friendWidget->chatroomWidgetClicked(friendWidget);

    //  Profile *profile = Nexus::getProfile();
    //  connect(profile, &Profile::friendAvatarSet, this, &FriendWidget::onAvatarSet);
    //  connect(profile, &Profile::friendAvatarRemoved, this,
    //          &FriendWidget::onAvatarRemoved);

    QPixmap avatar = Nexus::getProfile()->loadAvatar(frnd->getPublicKey());
    if (!avatar.isNull()) {
        setAvatar(avatar);
    }

    return dialog;
}

/**
 * @brief FriendWidget::contextMenuEvent
 * @param event Describe a context menu event
 *
 * Default context menu event handler.
 * Redirect all event information to the signal.
 */
void FriendWidget::contextMenuEvent(QContextMenuEvent* event) {
    onContextMenuCalled(event);
    //  emit contextMenuCalled(event);
}

/**
 * @brief FriendWidget::onContextMenuCalled
 * @param event Redirected from native contextMenuEvent
 *
 * Context menu handler. Always should be called to FriendWidget from FriendList
 */
void FriendWidget::onContextMenuCalled(QContextMenuEvent* event) {
    if (!active) {
        setBackgroundRole(QPalette::Highlight);
    }

    installEventFilter(this);  // Disable leave event.

    //  const auto newGroupAction = inviteMenu->addAction(tr("To new group"));
    //  connect(newGroupAction, &QAction::triggered, chatRoom.get(),
    //          &FriendChatroom::inviteToNewGroup);
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
    //          &FriendWidget::moveToNewCircle);
    //
    //  if (circleId != -1) {
    //    const auto circleName = chatRoom->getCircleName();
    //    const auto removeCircleAction =
    //        circleMenu->addAction(tr("Remove from circle
    //        '%1'").arg(circleName));
    //    connect(removeCircleAction, &QAction::triggered, this,
    //            &FriendWidget::removeFromCircle);
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
    //  &FriendWidget::changeAutoAccept);

    //  menu.addSeparator();
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

    //  menu.addSeparator();
    //  const auto aboutWindow = menu.addAction(tr("Show details"));
    //  connect(aboutWindow, &QAction::triggered, this,
    //  &FriendWidget::showDetails);

    const auto pos = event->globalPos();
    auto selected = menu->exec(pos);
    qDebug() << "selected" << selected;

    removeEventFilter(this);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }

    if (selected == removeAct) {
        const bool retYes = GUI::askQuestion(tr("Confirmation"),
                                             tr("Are you sure to remove %1 ?").arg(getName()),
                                             false,
                                             true,
                                             true);
        if (!retYes) {
            return;
        }

        emit removeFriend(FriendId(contactId));
    } else if (selected == inviteToGrp) {
    }
}

void FriendWidget::changeAutoAccept(bool enable) {
    //  if (enable) {
    //    const auto oldDir = chatRoom->getAutoAcceptDir();
    //    const auto newDir = QFileDialog::getExistingDirectory(
    //        Q_NULLPTR, tr("Choose an auto accept directory", "popup title"),
    //        oldDir);
    //    chatRoom->setAutoAcceptDir(newDir);
    //  } else {
    //    chatRoom->disableAutoAccept();
    //  }
}

void FriendWidget::inviteToNewGroup() {
    auto groupCreate = new GroupCreateForm();
    connect(groupCreate, &GroupCreateForm::confirmed, [&, groupCreate](const QString name) {
        auto core = Core::getInstance();
        auto groupId = core->createGroup(name);
        qDebug() << "Create group successful:" << groupId;
        if (groupId.isValid()) {
            core->inviteToGroup(m_friend->getId(), groupId);
            groupCreate->close();
        }
    });
    groupCreate->show();
}

void FriendWidget::showDetails() {
    qDebug() << __func__;
    if (!about) {
        qDebug() << "create about for:" << m_friend->getId().toString();
        const auto af = new AboutFriend(m_friend, &Settings::getInstance());
        std::unique_ptr<IAboutFriend> iabout = std::unique_ptr<IAboutFriend>(af);
        about = new AboutFriendForm(std::move(iabout), this);
        connect(about, &AboutFriendForm::histroyRemoved, this, &FriendWidget::friendHistoryRemoved);
        contentLayout->addWidget(about);
    }

    contentLayout->setCurrentWidget(about);
}

void FriendWidget::removeDetails() {
    //    if(about){
    //        contentLayout->removeWidget(about);
    //        about = nullptr;
    //    }
}

void FriendWidget::setAsActiveChatroom() { setActive(true); }

void FriendWidget::setAsInactiveChatroom() { setActive(false); }

void FriendWidget::onActiveSet(bool active) { setBackgroundRole(QPalette::Window); }

QString FriendWidget::getStatusString() const {
    const int status = static_cast<int>(m_friend->getStatus());
    const bool event = m_friend->getEventFlag();

    static const QVector<QString> names = {
            tr("Online"),
            tr("Away"),
            tr("Busy"),
            tr("Offline"),
    };

    return event ? tr("New message") : names.value(status);
}

const Friend* FriendWidget::getFriend() const { return m_friend; }

const Contact* FriendWidget::getContact() const { return getFriend(); }

void FriendWidget::search(const QString& searchString, bool hide) {
    //  const auto frnd = chatRoom->getFriend();
    //  searchName(searchString, hide);
    //  const Settings &s = Settings::getInstance();
    //  const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
    //  CircleWidget *circleWidget = CircleWidget::getFromID(circleId);
    //  if (circleWidget) {
    //    circleWidget->search(searchString);
    //  }
}

void FriendWidget::resetEventFlags() {
    // resetEventFlags();
}

void FriendWidget::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::LeftButton) {
        dragStartPos = ev->pos();
    }

    GenericChatroomWidget::mousePressEvent(ev);
}

void FriendWidget::mouseMoveEvent(QMouseEvent* ev) {
    if (!(ev->buttons() & Qt::LeftButton)) {
        return;
    }

    const int distance = (dragStartPos - ev->pos()).manhattanLength();
    if (distance > QApplication::startDragDistance()) {
        QMimeData* mdata = new QMimeData;
        const Friend* frnd = getFriend();
        mdata->setText(frnd->getDisplayedName());
        mdata->setData("toxPk", frnd->getPublicKey().getByteArray());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void FriendWidget::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    if (active) {
        opt.state |= QStyle::State_Selected;
    }
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

ContentDialog* FriendWidget::createContentDialog() const {
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

void FriendWidget::setStatus(Status::Status status, bool event) {
    updateStatusLight(status, event);
}

void FriendWidget::setStatusMsg(const QString& msg) {
    m_friend->setStatusMessage(msg);
    GenericChatroomWidget::setStatusMsg(msg);
}
void FriendWidget::setTyping(bool typing) {}

void FriendWidget::setName(const QString& name) {
    GenericChatroomWidget::setName(name);
    if (about) {
        about->setName(name);
    }
}
