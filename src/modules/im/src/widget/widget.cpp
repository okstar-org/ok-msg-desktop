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

#include "widget.h"

#include <cassert>

#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>

#include <QMouseEvent>
#include <QPainter>
#include <QShortcut>
#include <QString>
#include <QSvgRenderer>
#include <QTabWidget>
#include <QWindow>

#ifdef Q_OS_MAC
#include <QMenuBar>
#include <QSignalMapper>
#include <QWindow>
#endif

#include "Bus.h"
#include "ChatWidget.h"
#include "ContactListWidget.h"
#include "application.h"
#include "base/MessageBox.h"
#include "base/Page.h"
#include "base/SvgUtils.h"
#include "base/images.h"

#include "contentdialog.h"
#include "contentlayout.h"
#include "form/groupchatform.h"
#include "friendwidget.h"
#include "groupwidget.h"
#include "gui.h"
#include "lib/audio/audio.h"
#include "lib/storage/settings/OkSettings.h"
#include "lib/storage/settings/translator.h"
#include "splitterrestorer.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/corefile.h"
#include "src/lib/session/profile.h"
#include "src/lib/storage/settings/style.h"
#include "src/lib/ui/widget/tools/MaskablePixmap.h"
#include "src/model/friend.h"
#include "src/model/friendlist.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/model/grouplist.h"
#include "src/model/profile/profileinfo.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/platform/timer.h"
#include "src/widget/ContactWidget.h"
#include "src/widget/contentdialogmanager.h"
#include "src/widget/form/ContactSelectDialog.h"
#include "src/widget/form/addfriendform.h"
#include "src/widget/form/filesform.h"
#include "src/widget/form/groupinviteform.h"
#include "src/widget/form/profileform.h"
#include "src/widget/form/settingswidget.h"
#include "tool/removefrienddialog.h"
#include "ui_mainwindow.h"

namespace module::im {

static Widget* instance = nullptr;

Widget* Widget::getInstance() {
    assert(instance);
    return instance;
};

Widget::Widget(QWidget* parent)  //
        : QFrame(parent)
        , ui(new Ui::IMMainWindow)
        , eventFlag(false)
        , eventIcon(false)
        , delayCaller(std::make_unique<base::DelayedCallTimer>())  //
{
    instance = this;

    ui->setupUi(this);
    layout()->setSpacing(0);

    ui->tabWidget->setObjectName("mainTab");

    chatWidget = new ChatWidget(this);
    ui->tabWidget->addTab(chatWidget, tr("Chat"));

    contactWidget = new ContactWidget(this);
    ui->tabWidget->addTab(contactWidget, tr("Contacts"));

    settingsWidget = new SettingsWidget(this);
    ui->tabWidget->addTab(settingsWidget, tr("Settings"));
    ui->tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);

    installEventFilter(this);

    QIcon themeIcon = QIcon::fromTheme("qtox");
    if (!themeIcon.isNull()) {
        setWindowIcon(themeIcon);
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::onUserAwayCheck);
    connect(timer, &QTimer::timeout, this, &Widget::onEventIconTick);
    timer->start(1000);

    icon_size = 15;

    actionShow = new QAction(this);
    connect(actionShow, &QAction::triggered, this, &Widget::forceShow);

    // Preparing icons and set their size
    statusOnline = new QAction(this);
    statusOnline->setIcon(
            ok::base::SvgUtils::prepareIcon(getIconPath(Status::Online), icon_size, icon_size));
    connect(statusOnline, &QAction::triggered, this, &Widget::setStatusOnline);

    statusAway = new QAction(this);
    statusAway->setIcon(
            ok::base::SvgUtils::prepareIcon(getIconPath(Status::Away), icon_size, icon_size));
    connect(statusAway, &QAction::triggered, this, &Widget::setStatusAway);

    statusBusy = new QAction(this);
    statusBusy->setIcon(
            ok::base::SvgUtils::prepareIcon(getIconPath(Status::Busy), icon_size, icon_size));
    connect(statusBusy, &QAction::triggered, this, &Widget::setStatusBusy);

    actionLogout = new QAction(this);
    actionLogout->setIcon(
            ok::base::SvgUtils::prepareIcon(":/img/others/logout-icon.svg", icon_size, icon_size));

    actionQuit = new QAction(this);
#ifndef Q_OS_OSX
    actionQuit->setMenuRole(QAction::QuitRole);
#endif

    actionQuit->setIcon(ok::base::SvgUtils::prepareIcon(
            lib::settings::Style::getImagePath("rejectCall/rejectCall.svg"), icon_size, icon_size));
    connect(actionQuit, &QAction::triggered, qApp, &QApplication::quit);

    //  layout()->setContentsMargins(0, 0, 0, 0);
    setAttribute(Qt::WA_LayoutOnEntireRect, false);  // style sheet will make content margins
    //  setContentsMargins(0,0,0,0);
    //  setAutoFillBackground(true);

    //  profilePicture =
    //      new MaskablePixmapWidget(this, QSize(40, 40), ":/img/avatar_mask.svg");
    //  profilePicture->setPixmap(QPixmap(":/img/contact_dark.svg"));
    //  profilePicture->setClickable(true);
    //  profilePicture->setObjectName("selfAvatar");
    //  ui->myProfile->insertWidget(0, profilePicture);
    //  if(ui->myProfile){
    //    ui->myProfile->insertSpacing(1, 7);
    //  }

    //  ui->friendList->setWidget(contactListWidget);
    //  ui->friendList->setLayoutDirection(Qt::RightToLeft);
    //  ui->friendList->setContextMenuPolicy(Qt::CustomContextMenu);
    //  ui->statusLabel->setEditable(true);

    // disable proportional scaling
    //  ui->mainSplitter->setStretchFactor(0, 0);
    //  ui->mainSplitter->setStretchFactor(1, 1);
    // Disable some widgets until we're connected to the DHT
    //  ui->statusButton->setEnabled(false);

    auto& settings = lib::settings::OkSettings::getInstance();

    settings::Translator::translate(OK_IM_MODULE, settings.getTranslation());
    connect(ok::Application::Instance()->bus(), &ok::Bus::languageChanged,
            [](QString locale0) { settings::Translator::translate(OK_IM_MODULE, locale0); });

    lib::settings::Style::setThemeColor(settings.getThemeColor());

    onStatusSet(Status::Offline);

    // NOTE: We intentionally do not connect the fileUploadFinished and
    // fileDownloadFinished signals because they are duplicates of
    // fileTransferFinished NOTE: We don't hook up the fileNameChanged signal
    // since it is only emitted before a fileReceiveRequest. We get the initial
    // request with the sanitized name so there is no work for us to do

    // keyboard shortcuts
    new QShortcut(QKeySequence(Qt::CTRL, Qt::Key_Q), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::CTRL, Qt::Key_PageUp), this, SLOT(previousContact()));
    new QShortcut(QKeySequence(Qt::CTRL, Qt::SHIFT, Qt::Key_Tab), this, SLOT(previousContact()));
    new QShortcut(QKeySequence(Qt::CTRL, Qt::Key_Tab), this, SLOT(nextContact()));
    new QShortcut(QKeySequence(Qt::CTRL, Qt::Key_PageDown), this, SLOT(nextContact()));
    new QShortcut(Qt::Key_F11, this, SLOT(toggleFullscreen()));

    //    onSeparateWindowChanged(settings.getSeparateWindow(), false);

    //  ui->addButton->setCheckable(true);
    //  ui->groupButton->setCheckable(true);
    //  ui->transferButton->setCheckable(true);
    //  ui->settingsButton->setCheckable(true);
    //
    //  if (contentLayout) {
    ////    onAddClicked();
    //  }

    // restore window state
    //  restoreGeometry(settings.getWindowGeometry());
    //  restoreState(settings.getWindowState());
    //  SplitterRestorer restorer(ui->mainSplitter);
    //  restorer.restore(settings.getSplitterState(), size());

    //    friendRequestsButton = nullptr;
    //    groupInvitesButton = nullptr;
    //    unreadGroupInvites = 0;
    //
    //    connect(groupInviteForm, &GroupInviteForm::groupInvitesSeen, this,
    //            &Widget::groupInvitesClear);
    //    connect(groupInviteForm, &GroupInviteForm::groupInviteAccepted, this,
    //            &Widget::onGroupInviteAccepted);

    reloadTheme();
    connect(ok::Application::Instance()->bus(), &ok::Bus::themeColorChanged, this,
            [&]() { reloadTheme(); });

    updateIcons();

    retranslateUi();
    settings::Translator::registerHandler(std::bind(&Widget::retranslateUi, this), this);

#ifdef Q_OS_MAC
    // Nexus::getInstance()->updateWindows();
#endif

    init();
}

Widget::~Widget() {
    qDebug() << __func__;
    settings::Translator::unregister(this);
    delete timer;
    delete ui;
}

void Widget::init() {
    connect(this, &Widget::toSendMessage, [&]() { ui->tabWidget->setCurrentIndex(0); });
    connect(this, &Widget::toShowDetails, [&]() { ui->tabWidget->setCurrentIndex(1); });
    // 显示转发消息对话框
    connect(this, &Widget::toForwardMessage, this, &Widget::showForwardMessageDialog);

    // 添加好友到群聊
    connect(this, &Widget::toAddMember, this, &Widget::showAddMemberDialog);

#if UPDATE_CHECK_ENABLED
    updateCheck = std::unique_ptr<UpdateCheck>(new UpdateCheck(settings));
    connect(updateCheck.get(), &UpdateCheck::updateAvailable, this, &Widget::onUpdateAvailable);
    updateCheck->checkForUpdate();
#endif
}

bool Widget::eventFilter(QObject* obj, QEvent* event) {
    QWindowStateChangeEvent* ce = nullptr;
    Qt::WindowStates state = windowState();

    switch (event->type()) {
        case QEvent::Close:
            // It's needed if user enable `Close to tray`
            wasMaximized = state.testFlag(Qt::WindowMaximized);
            break;
        case QEvent::WindowStateChange:
            ce = static_cast<QWindowStateChangeEvent*>(event);
            if (state.testFlag(Qt::WindowMinimized) && obj) {
                wasMaximized = ce->oldState().testFlag(Qt::WindowMaximized);
            }
#ifdef Q_OS_MAC
            emit windowStateChanged(windowState());
#endif
            break;
        default:
            break;
    }

    return false;
}

void Widget::updateIcons() {}

/**
 * @brief Switches to the About settings page.
 */
void Widget::showUpdateDownloadProgress() {
    settingsWidget->showAbout();
}

void Widget::moveEvent(QMoveEvent* event) {
    if (event->type() == QEvent::Move) {
        saveWindowGeometry();
        saveSplitterGeometry();
    }

    QWidget::moveEvent(event);
}

void Widget::closeEvent(QCloseEvent* event) {
    QWidget::closeEvent(event);
}

void Widget::changeEvent(QEvent* event) {}

void Widget::resizeEvent(QResizeEvent* event) {
    saveWindowGeometry();
    QWidget::resizeEvent(event);
}

QString Widget::getUsername() {
    return Nexus::getProfile()->getCore()->getUsername();
}

void Widget::onSelfAvatarLoaded(const QPixmap& pic) {
    if (pic.size().isEmpty()) {
        qWarning() << __func__ << "pic is empty.";
        return;
    }
    //  profilePicture->setPixmap(pic);
}

void Widget::onCoreChanged(Core& coreRef) {
    core = &coreRef;
    connectToCore(coreRef);
}

void Widget::connectToCore(Core& core) {
    connect(&core, &Core::connected, this, &Widget::onConnected);
    connect(&core, &Core::disconnected, this, &Widget::onDisconnected);
    connect(&core, &Core::started, this, &Widget::onStarted);

    connect(&core, &Core::statusSet, this, &Widget::onStatusSet);
    connect(&core, &Core::usernameSet, this, &Widget::setUsername);
    connect(&core, &Core::avatarSet, this, &Widget::setAvatar);
    connect(&core, &Core::failedToAddFriend, this, &Widget::addFriendFailed);

    connect(this, &Widget::statusSet, &core, &Core::setStatus);
    connect(this, &Widget::changeGroupTitle, &core, &Core::setGroupName);
}

void Widget::onConnected() {
    // ui->statusButton->setEnabled(true);
    //  emit core->statusSet(core->getStatus());
}

void Widget::onDisconnected() {
    //  ui->statusButton->setEnabled(false);
    //  emit core->statusSet(Status::Offline);
}

void Widget::onStarted() {}

void Widget::onFailedToStartCore() {
    ok::base::MessageBox::critical(this, "",
                                   tr("The core failed to start, the application will terminate "
                                      "after you close this message."));
    qApp->exit(EXIT_FAILURE);
}

void Widget::onBadProxyCore() {}

void Widget::onStatusSet(Status status) {
    //  ui->statusButton->setProperty("status", static_cast<int>(status));
    //  ui->statusButton->setIcon(
    //      prepareIcon(getIconPath(status), icon_size, icon_size));
    //  updateIcons();
}

void Widget::setWindowTitle(const QString& title) {
    //  if (title.isEmpty()) {
    //      setWindowTitle(QApplication::applicationName());
    //  } else {
    //    QString tmp = title;
    //    /// <[^>]*> Regexp to remove HTML tags, in case someone used them in
    //    title setWindowTitle(QApplication::applicationName() +
    //                                QStringLiteral(" - ") +
    //                                tmp.remove(QRegExp("<[^>]*>")));
    //  }
}

void Widget::forceShow() {
    hide();
    // Workaround to force minimized window to be restored
    show();
    activateWindow();
}

void Widget::onTransferClicked() {
    //  if (settings.getSeparateWindow()) {
    //    if (!filesForm->isShown()) {
    //      filesForm->show(createContentDialog(DialogType::TransferDialog));
    //    }

    //    setActiveToolMenuButton(ActiveToolMenuButton::None);
    //  } else {
    //    hideMainForms(nullptr);
    ////    filesForm->show(contentLayout);
    //    setWindowTitle(fromDialogType(DialogType::TransferDialog));
    //    setActiveToolMenuButton(ActiveToolMenuButton::TransferButton);
    //  }
}

void Widget::confirmExecutableOpen(const QFileInfo& file) {
    static const QStringList dangerousExtensions = {
            "app", "bat", "com",    "cpl",  "dmg",     "exe",  "hta",     "jar",    "js",  "jse",
            "lnk", "msc", "msh",    "msh1", "msh1xml", "msh2", "msh2xml", "mshxml", "msi", "msp",
            "pif", "ps1", "ps1xml", "ps2",  "ps2xml",  "psc1", "psc2",    "py",     "reg", "scf",
            "sh",  "src", "vb",     "vbe",  "vbs",     "ws",   "wsc",     "wsf",    "wsh"};

    if (dangerousExtensions.contains(file.suffix())) {
        bool answer = GUI::askQuestion(tr("Executable file", "popup title"),
                                       tr("You have asked OkMsg to open an executable file. "
                                          "Executable files can potentially damage your computer. "
                                          "Are you sure want to open this file?",
                                          "popup text"),
                                       false, true);
        if (!answer) {
            return;
        }

        // The user wants to run this file, so make it executable and run it
        QFile(file.filePath())
                .setPermissions(file.permissions() | QFile::ExeOwner | QFile::ExeUser |
                                QFile::ExeGroup | QFile::ExeOther);
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(file.filePath()));
}

void Widget::onIconClick(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (isHidden() || isMinimized()) {
            if (wasMaximized) {
                showMaximized();
            } else {
                showNormal();
            }

            activateWindow();
        } else if (!isActiveWindow()) {
            activateWindow();
        } else {
            wasMaximized = isMaximized();
            hide();
        }
    } else if (reason == QSystemTrayIcon::Unknown) {
        if (isHidden()) {
            forceShow();
        }
    }
}

void Widget::setUsername(const QString& username) {}

void Widget::onStatusMessageChanged(const QString& newStatusMessage) {
    core->setStatusMessage(newStatusMessage);
}

void Widget::setAvatar(QByteArray avatar) {
    if (avatar.isEmpty()) {
        qWarning() << __func__ << "avatar is empty!";
        return;
    }

    QPixmap pixmap;
    if (!ok::base::Images::putToPixmap(avatar, pixmap)) {
        qWarning() << "loadFromData failed.";
        return;
    }

    emit avatarSet(pixmap);

    //  profilePicture->setPixmap(pixmap);
    //  profileInfo->setAvatar(pixmap);
}

void Widget::addFriendFailed(const FriendId&, const QString& errorInfo) {
    QString info = QString(tr("Couldn't request friendship"));
    if (!errorInfo.isEmpty()) {
        info = info + QStringLiteral(": ") + errorInfo;
    }

    QMessageBox::critical(nullptr, "Error", info);
}

void Widget::onChatroomWidgetClicked(GenericChatroomWidget* widget) {
    openDialog(widget, /* newWindow = */ false);
}

void Widget::openNewDialog(GenericChatroomWidget* widget) {
    openDialog(widget, /* newWindow = */ true);
}

void Widget::openDialog(GenericChatroomWidget* widget, bool newWindow) {
    widget->resetEventFlags();
    //  widget->updateStatusLight();

    //  GenericChatForm *form;
    //  GroupId id;
    ////  const IMFriend *frnd = widget->getFriend();
    ////  const Group *group = widget->getGroup();
    ////  if (frnd) {
    ////    form = chatForms[frnd->getPublicKey()];
    ////  } else if (group) {
    ////    id = group->getId();
    ////    form = groupChatForms[id].data();
    ////  }
    //  bool chatFormIsSet;
    //  ContentDialogManager::getInstance()->focusContact(id);
    //  chatFormIsSet = ContentDialogManager::getInstance()->contactWidgetExists(id);

    //  if ((chatFormIsSet || form->isVisible()) && !newWindow) {
    //    return;
    //  }

    //  if (settings.getSeparateWindow() || newWindow) {
    //    ContentDialog *dialog = nullptr;

    //    if (!settings.getDontGroupWindows() && !newWindow) {
    //      dialog = ContentDialogManager::getInstance()->current();
    //    }

    //    if (dialog == nullptr) {
    //      dialog = createContentDialog();
    //    }

    //    dialog->show();

    //    if (frnd) {
    //      addFriendDialog(frnd, dialog);
    //    } else {
    //      auto group = widget->getGroup();
    //      addGroupDialog(group, dialog);
    //    }

    //    dialog->raise();
    //    dialog->activateWindow();
    //  } else {
    //    hideMainForms(widget);
    //    if (frnd) {
    ////      chatForms[frnd->getPublicKey()]->show(contentLayout);
    ////    } else {
    ////      groupChatForms[group->getId()]->show(contentLayout);
    //    }
    //    widget->setAsActiveChatroom();
    //    setWindowTitle(widget->getSubject());
    //  }
}

bool Widget::newFriendMessageAlert(const FriendId& friendId, const QString& text, bool sound,
                                   bool file) {
    auto settings = Nexus::getProfile()->getSettings();

    bool hasActive = false;
    QWidget* currentWindow;
    ContentDialog* contentDialog = ContentDialogManager::getInstance()->getFriendDialog(friendId);
    Friend* f = Nexus::getCore()->getFriendList().findFriend(friendId);

    if (contentDialog != nullptr) {
        currentWindow = contentDialog->window();
        hasActive = ContentDialogManager::getInstance()->isContactActive(friendId);
    } else {
        if (settings->getSeparateWindow() && settings->getShowWindow()) {
            if (settings->getDontGroupWindows()) {
                contentDialog = createContentDialog();
            } else {
                contentDialog = ContentDialogManager::getInstance()->current();
                if (!contentDialog) {
                    contentDialog = createContentDialog();
                }
            }

            currentWindow = contentDialog->window();
            hasActive = ContentDialogManager::getInstance()->isContactActive(friendId);
        } else {
            currentWindow = window();
        }
    }

    if (newMessageAlert(currentWindow, hasActive, sound)) {
#if DESKTOP_NOTIFICATIONS
        if (settings.getNotifyHide()) {
            notifier.notifyMessageSimple(file ? DesktopNotify::MessageType::FRIEND_FILE
                                              : DesktopNotify::MessageType::FRIEND);
        } else {
            QString title = f->getDisplayedName();
            if (file) {
                title += " - " + tr("File sent");
            }
            notifier.notifyMessagePixmap(title, text,
                                         Nexus::getProfile()->loadAvatar(f->getPublicKey()));
        }
#endif

        if (contentDialog == nullptr) {
            if (hasActive) {
                //        setWindowTitle(widget->getSubject());
            }
        } else {
            ContentDialogManager::getInstance()->updateFriendStatus(friendId);
        }

        return true;
    }

    return false;
}

bool Widget::newGroupMessageAlert(const GroupId& groupId, const FriendId& authorPk,
                                  const QString& message, bool notify) {
    qDebug() << __func__ << "groupId" << groupId.toString() << "message" << message;
    bool hasActive = false;
    QWidget* currentWindow = currentWindow = window();

    if (!newMessageAlert(currentWindow, hasActive, true, notify)) {
        return false;
    }

//  g->setEventFlag(true);
//  widget->updateStatusLight();
#if DESKTOP_NOTIFICATIONS
    if (settings.getNotifyHide()) {
        notifier.notifyMessageSimple(DesktopNotify::MessageType::GROUP);
    } else {
        IMFriend* f = Nexus::getCore()->getFriendList().findFriend(authorPk);
        QString title = g->getPeerList().value(authorPk) + " (" + g->getDisplayedName() + ")";
        if (!f) {
            notifier.notifyMessage(title, message);
        } else {
            notifier.notifyMessagePixmap(title, message,
                                         Nexus::getProfile()->loadAvatar(f->getPublicKey()));
        }
    }
#endif

    //  if (contentDialog == nullptr) {
    //    if (hasActive) {
    //      setWindowTitle(widget->getSubject());
    //    }
    //  } else {
    //    ContentDialogManager::getInstance()->updateGroupStatus(groupId);
    //  }

    return true;
}

QString Widget::fromDialogType(DialogType type) {
    switch (type) {
        case DialogType::AddDialog:
            return tr("Add friend", "title of the window");
        case DialogType::GroupDialog:
            return tr("Group invites", "title of the window");
        case DialogType::TransferDialog:
            return tr("File transfers", "title of the window");
        case DialogType::SettingDialog:
            return tr("Settings", "title of the window");
        case DialogType::ProfileDialog:
            return tr("My profile", "title of the window");
    }
    assert(false);
    return QString();
}

bool Widget::newMessageAlert(QWidget* currentWindow, bool isActive, bool sound, bool notify) {
    bool inactiveWindow = isMinimized() || !currentWindow->isActiveWindow();

    if (!inactiveWindow && isActive) {
        return false;
    }

    if (notify) {
        auto settings = Nexus::getProfile()->getSettings();
        if (settings->getShowWindow()) {
            currentWindow->show();
        }

        if (settings->getNotify()) {
            if (inactiveWindow) {
#if DESKTOP_NOTIFICATIONS
                if (!settings.getDesktopNotify()) {
                    QApplication::alert(currentWindow);
                }
#else
                QApplication::alert(currentWindow);
#endif
                eventFlag = true;
            }

            bool isBusy = Nexus::getCore()->getStatus() == Status::Busy;
            bool busySound = settings->getBusySound();
            bool notifySound = settings->getNotifySound();

            if (notifySound && sound && (!isBusy || busySound)) {
                Nexus::getInstance()->playNotificationSound(
                        lib::audio::IAudioSink::Sound::NewMessage);
            }
        }
    }

    return true;
}

void Widget::friendRequestedTo(const ToxId& friendAddress, const QString& nick,
                               const QString& message) {
    qDebug() << "friendRequestedTo" << friendAddress.getPublicKey().toString() << nick << message;
    auto frd = Nexus::getCore()->getFriendList().findFriend(friendAddress.getPublicKey());
    if (frd) {
        return;
    }
    emit friendRequested(friendAddress, nick, message);
}

void Widget::onFileReceiveRequested(const ToxFile& file) {
    const FriendId& friendPk = FriendId(file.receiver);
    newFriendMessageAlert(
            friendPk,
            file.fileName + " (" + FileTransferWidget::getHumanReadableSize(file.fileSize) + ")",
            true, true);
}

void Widget::onDialogShown(GenericChatroomWidget* widget) {
    widget->resetEventFlags();
    //  widget->updateStatusLight();

    //  ui->friendList->updateTracking(widget);
    resetIcon();
}

void Widget::onFriendDialogShown(const Friend* f) {
    //  onDialogShown(contactListWidget->getFriend(f->getPublicKey()));
}

void Widget::onGroupDialogShown(const Group* g) {
    const GroupId& groupId = g->getId();
    //  onDialogShown(groupWidgets[groupId]);
}

void Widget::toggleFullscreen() {
    if (windowState().testFlag(Qt::WindowFullScreen)) {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() | Qt::WindowFullScreen);
    }
}

void Widget::onUpdateAvailable() {
    //  ui->settingsButton->setProperty("update-available", true);
    //  ui->settingsButton->style()->unpolish(ui->settingsButton);
    //  ui->settingsButton->style()->polish(ui->settingsButton);
}

ContentDialog* Widget::createContentDialog() const {
    ContentDialog* contentDialog = new ContentDialog();

    registerContentDialog(*contentDialog);
    return contentDialog;
}

void Widget::registerContentDialog(ContentDialog& contentDialog) const {
    ContentDialogManager::getInstance()->addContentDialog(contentDialog);
    connect(&contentDialog, &ContentDialog::friendDialogShown, this, &Widget::onFriendDialogShown);
    connect(&contentDialog, &ContentDialog::groupDialogShown, this, &Widget::onGroupDialogShown);
    connect(core, &Core::usernameSet, &contentDialog, &ContentDialog::setUsername);

    auto settings = Nexus::getProfile()->getSettings();
    connect(settings, &Settings::groupchatPositionChanged, &contentDialog,
            &ContentDialog::reorderLayouts);
}

ContentLayout* Widget::createContentDialog(DialogType type) const {
    class Dialog : public ActivateDialog {
    public:
        explicit Dialog(DialogType type, Settings& settings, Core* core)
                : ActivateDialog(nullptr, Qt::Window), type(type), settings(settings), core{core} {
            restoreGeometry(settings.getDialogSettingsGeometry());

            setWindowIcon(QIcon(":/img/icons/qtox.svg"));
            setStyleSheet(lib::settings::Style::getStylesheet("window/general.css"));
            connect(core, &Core::usernameSet, this, &Dialog::retranslateUi);

            settings::Translator::registerHandler(std::bind(&Dialog::retranslateUi, this), this);
            retranslateUi();
        }

        ~Dialog() {
            settings::Translator::unregister(this);
        }

    public slots:

        void retranslateUi() {
            setWindowTitle(core->getUsername() + QStringLiteral(" - ") +
                           Widget::fromDialogType(type));
        }

    protected:
        void resizeEvent(QResizeEvent* event) override {
            settings.setDialogSettingsGeometry(saveGeometry());
            QDialog::resizeEvent(event);
        }

        void moveEvent(QMoveEvent* event) override {
            settings.setDialogSettingsGeometry(saveGeometry());
            QDialog::moveEvent(event);
        }

    private:
        DialogType type;
        Settings& settings;
        Core* core;
    };

    auto settings = Nexus::getProfile()->getSettings();
    Dialog* dialog = new Dialog(type, *settings, core);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    ContentLayout* contentLayoutDialog = new ContentLayout(dialog);

    dialog->setObjectName("detached");
    dialog->setLayout(contentLayoutDialog);
    dialog->layout()->setMargin(0);
    dialog->layout()->setSpacing(0);
    dialog->setMinimumSize(720, 400);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();

    return contentLayoutDialog;
}

void Widget::copyFriendIdToClipboard(const FriendId& friendId) {
    Friend* f = Nexus::getCore()->getFriendList().findFriend(friendId);
    if (f != nullptr) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(friendId.toString(), QClipboard::Clipboard);
    }
}

void Widget::titleChangedByUser(const QString& title) {
    const auto* group = qobject_cast<Group*>(sender());
    assert(group != nullptr);
    emit changeGroupTitle(group->getIdAsString(), title);
}

void Widget::onGroupPeerAudioPlaying(QString groupnumber, FriendId peerPk) {
    const GroupId& groupId = GroupId(groupnumber);
    Group* g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "Can not find the group named:" << groupnumber;
        return;
    }
}

/**
 * @brief Used to reset the blinking icon.
 */
void Widget::resetIcon() {
    eventIcon = false;
    eventFlag = false;
    updateIcons();
}

void Widget::showForwardMessageDialog(const MsgId& msgId) {
    ContactSelectDialog modalDialog(this);
    connect(&modalDialog, &ContactSelectDialog::contactClicked,
            [this, msgId](const ContactId& cId) { emit forwardMessage(cId, msgId); });
    modalDialog.exec();
}

void Widget::showAddMemberDialog(const ContactId& groupId) {
    ContactSelectDialog modalDialog(this);
    connect(&modalDialog, &ContactSelectDialog::contactClicked,
            [this, groupId](const ContactId& cId) { emit addMember(cId, groupId); });
    modalDialog.exec();
}

bool Widget::event(QEvent* e) {
    switch (e->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick:
            //    focusChatInput();
            break;
        case QEvent::Paint:
            //    ui->friendList->updateVisualTracking();
            break;
        case QEvent::WindowActivate:

            if (eventFlag) {
                resetIcon();
            }

#ifdef Q_OS_MAC
            emit windowStateChanged(windowState());

        case QEvent::WindowStateChange:
            Nexus::getInstance()->updateWindowsStates();
#endif
            break;
        default:
            break;
    }

    return QWidget::event(e);
}

void Widget::onUserAwayCheck() {
#ifdef QTOX_PLATFORM_EXT
    uint32_t autoAwayTime = settings.getAutoAwayTime() * 60 * 1000;
//  bool online = static_cast<Status>(
//                    ui->statusButton->property("status").toInt()) ==
//                Status::Online;
//  bool away = autoAwayTime && Platform::getIdleTime() >= autoAwayTime;

//  if (online && away) {
//    qDebug() << "auto away activated at" << QTime::currentTime().toString();
//    emit statusSet(Status::Away);
//    autoAwayActive = true;
//  } else if (autoAwayActive && !away) {
//    qDebug() << "auto away deactivated at" << QTime::currentTime().toString();
//    emit statusSet(Status::Online);
//    autoAwayActive = false;
//  }
#endif
}

void Widget::onEventIconTick() {
    if (eventFlag) {
        eventIcon ^= true;
        updateIcons();
    }
}

void Widget::setStatusOnline() {
    //  if (!ui->statusButton->isEnabled()) {
    //    return;
    //  }

    core->setStatus(Status::Online);
}

void Widget::setStatusAway() {
    //  if (!ui->statusButton->isEnabled()) {
    //    return;
    //  }

    core->setStatus(Status::Away);
}

void Widget::setStatusBusy() {
    //  if (!ui->statusButton->isEnabled()) {
    //    return;
    //  }

    core->setStatus(Status::Busy);
}

void Widget::saveWindowGeometry() {
    // auto settings = Nexus::getProfile()->getSettings();
    // settings->setWindowGeometry(saveGeometry());
    //  settings.setWindowState(saveState());
}

void Widget::saveSplitterGeometry() {
    //    auto settings = Nexus::getProfile()->getSettings();
    //    if (!settings.getSeparateWindow()) {
    //    settings.setSplitterState(ui->mainSplitter->saveState());
    //    }
}

void Widget::onSplitterMoved(int pos, int index) {
    Q_UNUSED(pos);
    Q_UNUSED(index);
    saveSplitterGeometry();
}

void Widget::cycleContacts(bool forward) {
    //  contactListWidget->cycleContacts(activeChatroomWidget, forward);
}

void Widget::clearAllReceipts() {
    chatWidget->clearAllReceipts();
}

void Widget::reloadTheme() {
    auto& style = lib::settings::Style::getStylesheet("window/general.css");
    this->setStyleSheet(style);
    chatWidget->reloadTheme();
    contactWidget->reloadTheme();
}

void Widget::retranslateUi() {
    ui->retranslateUi(this);
    ui->tabWidget->setTabText(0, tr("Chat"));
    ui->tabWidget->setTabText(1, tr("Contacts"));
    ui->tabWidget->setTabText(2, tr("Settings"));
}

void Widget::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
}
}  // namespace module::im