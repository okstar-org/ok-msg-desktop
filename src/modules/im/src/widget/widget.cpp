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
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QShortcut>
#include <QString>
#include <QSvgRenderer>
#include <QWindow>
#include <QTabWidget>

#ifdef Q_OS_MAC
#include <QMenuBar>
#include <QSignalMapper>
#include <QWindow>
#endif

#include "ChatWidget.h"
#include "base/OkSettings.h"
#include "base/Page.h"
#include "base/SvgUtils.h"
#include "base/images.h"
#include "circlewidget.h"
#include "contentdialog.h"
#include "contentlayout.h"
#include "form/groupchatform.h"
#include "friendlistwidget.h"
#include "friendwidget.h"
#include "groupwidget.h"
#include "lib/settings/translator.h"
#include "maskablepixmapwidget.h"
#include "splitterrestorer.h"
#include "src/audio/audio.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/corefile.h"
#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/model/profile/profileinfo.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/platform/timer.h"
#include "src/widget/contentdialogmanager.h"
#include "src/widget/form/addfriendform.h"
#include "src/widget/form/filesform.h"
#include "src/widget/form/groupinviteform.h"
#include "src/widget/form/profileform.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/gui.h"
#include "src/widget/style.h"
#include "src/widget/ContactWidget.h"
#include "tool/removefrienddialog.h"
#include "ui_mainwindow.h"


bool toxActivateEventHandler(const QByteArray &) {
  Widget *widget = Nexus::getDesktopGUI();
  if (!widget) {
    return true;
  }

  qDebug() << "Handling [activate] event from other instance";
  widget->forceShow();

  return true;
}


static Widget *instance = nullptr;

Widget *Widget::getInstance(){
    assert(instance);
    return instance;
};

Widget::Widget(IAudioControl &audio, QWidget *parent)//
    : QFrame(parent), icon{nullptr}, trayMenu{nullptr},//
      ui(new Ui::IMMainWindow),  eventFlag(false),//
      eventIcon(false), audio(audio),       //
      settings(Settings::getInstance()),    //
      delayCaller(std::make_unique<base::DelayedCallTimer>()) //
{
  instance = this;

  ui->setupUi(this);
  layout()->setMargin(0);
  layout()->setSpacing(0);

  setMinimumWidth(777);

  setObjectName(qsl("Page:%1").arg(static_cast<int>(UI::PageMenu::chat)));

  ui->tabWidget->setObjectName("mainTab");

  chatWidget = new ChatWidget(this);
  ui->tabWidget->addTab(chatWidget, tr("Chat"));

  contactWidget = new ContactWidget(this);
  ui->tabWidget->addTab(contactWidget, tr("Contact"));

  settingsWidget = new SettingsWidget(this);
  ui->tabWidget->addTab(settingsWidget, tr("Settings"));

  installEventFilter(this);
  QString locale = settings.getTranslation();
  settings::Translator::translate(OK_IM_MODULE, locale);
  qRegisterMetaType<ToxFile>("ToxFile");

    QIcon themeIcon = QIcon::fromTheme("qtox");
    if (!themeIcon.isNull()) {
      setWindowIcon(themeIcon);
    }

    timer = new QTimer();
    timer->start(1000);

    icon_size = 15;

    actionShow = new QAction(this);
    connect(actionShow, &QAction::triggered, this, &Widget::forceShow);

    // Preparing icons and set their size
    statusOnline = new QAction(this);
    statusOnline->setIcon(SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Online),
                                      icon_size, icon_size));
    connect(statusOnline, &QAction::triggered, this, &Widget::setStatusOnline);

    statusAway = new QAction(this);
    statusAway->setIcon(SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Away),
                                    icon_size, icon_size));
    connect(statusAway, &QAction::triggered, this, &Widget::setStatusAway);

    statusBusy = new QAction(this);
    statusBusy->setIcon(SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Busy),
                                    icon_size, icon_size));
    connect(statusBusy, &QAction::triggered, this, &Widget::setStatusBusy);

    actionLogout = new QAction(this);
    actionLogout->setIcon(SvgUtils::
        prepareIcon(":/img/others/logout-icon.svg", icon_size, icon_size));

    actionQuit = new QAction(this);
  #ifndef Q_OS_OSX
    actionQuit->setMenuRole(QAction::QuitRole);
  #endif

    actionQuit->setIcon(SvgUtils::prepareIcon(
        Style::getImagePath("rejectCall/rejectCall.svg"), icon_size, icon_size));
    connect(actionQuit, &QAction::triggered, qApp, &QApplication::quit);

    layout()->setContentsMargins(0, 0, 0, 0);
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

    Style::setThemeColor(settings.getThemeColor());

    onStatusSet(Status::Status::Offline);

    // NOTE: We intentionally do not connect the fileUploadFinished and
    // fileDownloadFinished signals because they are duplicates of
    // fileTransferFinished NOTE: We don't hook up the fileNameChanged signal
    // since it is only emitted before a fileReceiveRequest. We get the initial
    // request with the sanitized name so there is no work for us to do

    // keyboard shortcuts
    new QShortcut(QKeySequence(Qt::CTRL, Qt::Key_Q), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::CTRL, Qt::SHIFT , Qt::Key_Tab), this,
                  SLOT(previousContact()));
    new QShortcut(QKeySequence(Qt::CTRL, Qt::Key_Tab), this, SLOT(nextContact()));
    new QShortcut(QKeySequence(Qt::CTRL , Qt::Key_PageUp), this, SLOT(previousContact()));
    new QShortcut(QKeySequence(Qt::CTRL , Qt::Key_PageDown), this, SLOT(nextContact()));
    new QShortcut(Qt::Key_F11, this, SLOT(toggleFullscreen()));

#ifdef Q_OS_MAC
    QMenuBar *globalMenu = Nexus::getInstance().globalMenuBar;
    QAction *windowMenu = Nexus::getInstance().windowMenu->menuAction();
    QAction *viewMenu = Nexus::getInstance().viewMenu->menuAction();
    QAction *frontAction = Nexus::getInstance().frontAction;

    fileMenu = globalMenu->insertMenu(viewMenu, new QMenu(this));

    editProfileAction = fileMenu->menu()->addAction(QString());
    connect(editProfileAction, &QAction::triggered, this, &Widget::showProfile);

    changeStatusMenu = fileMenu->menu()->addMenu(QString());
    fileMenu->menu()->addAction(changeStatusMenu->menuAction());
    changeStatusMenu->addAction(statusOnline);
    changeStatusMenu->addSeparator();
    changeStatusMenu->addAction(statusAway);
    changeStatusMenu->addAction(statusBusy);

    fileMenu->menu()->addSeparator();
    logoutAction = fileMenu->menu()->addAction(QString());
    connect(logoutAction, &QAction::triggered,
            [this]() { Nexus::getInstance().showLogin(); });

    editMenu = globalMenu->insertMenu(viewMenu, new QMenu(this));
    editMenu->menu()->addSeparator();

    viewMenu->menu()->insertMenu(Nexus::getInstance().fullscreenAction,
                                 filterMenu);

    viewMenu->menu()->insertSeparator(Nexus::getInstance().fullscreenAction);

    contactMenu = globalMenu->insertMenu(windowMenu, new QMenu(this));

    addContactAction = contactMenu->menu()->addAction(QString());
    connect(addContactAction, &QAction::triggered, this, &Widget::onAddClicked);

    nextConversationAction = new QAction(this);
    Nexus::getInstance().windowMenu->insertAction(frontAction,
                                                  nextConversationAction);
    nextConversationAction->setShortcut(QKeySequence::SelectNextPage);
    connect(nextConversationAction, &QAction::triggered, [this]() {
      if (ContentDialogManager::getInstance()->current() ==
          QApplication::activeWindow())
        ContentDialogManager::getInstance()->current()->cycleContacts(true);
      else if (QApplication::activeWindow() == this)
        cycleContacts(true);
    });

    previousConversationAction = new QAction(this);
    Nexus::getInstance().windowMenu->insertAction(frontAction,
                                                  previousConversationAction);
    previousConversationAction->setShortcut(QKeySequence::SelectPreviousPage);
    connect(previousConversationAction, &QAction::triggered, [this] {
      if (ContentDialogManager::getInstance()->current() ==
          QApplication::activeWindow())
        ContentDialogManager::getInstance()->current()->cycleContacts(false);
      else if (QApplication::activeWindow() == this)
        cycleContacts(false);
    });

    windowMenu->menu()->insertSeparator(frontAction);

    QAction *preferencesAction = viewMenu->menu()->addAction(QString());
    preferencesAction->setMenuRole(QAction::PreferencesRole);
    connect(preferencesAction, &QAction::triggered, this,
            &Widget::onShowSettings);

    QAction *aboutAction = viewMenu->menu()->addAction(QString());
    aboutAction->setMenuRole(QAction::AboutRole);
    connect(aboutAction, &QAction::triggered, [this]() {
      onShowSettings();
      settingsWidget->showAbout();
    });

    QMenu *dockChangeStatusMenu = new QMenu(tr("Status"), this);
    dockChangeStatusMenu->addAction(statusOnline);
    statusOnline->setIconVisibleInMenu(true);
    dockChangeStatusMenu->addSeparator();
    dockChangeStatusMenu->addAction(statusAway);
    dockChangeStatusMenu->addAction(statusBusy);
    Nexus::getInstance().dockMenu->addAction(dockChangeStatusMenu->menuAction());

    connect(this, &Widget::windowStateChanged, &Nexus::getInstance(),
            &Nexus::onWindowStateChanged);
#endif


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
    updateIcons();
    retranslateUi();
    settings::Translator::registerHandler(std::bind(&Widget::retranslateUi, this),
                                          this);

    // settings
    auto &okSettings = ok::base::OkSettings::getInstance();
    connect(&okSettings, &ok::base::OkSettings::showSystemTrayChanged, this,
            &Widget::onSetShowSystemTray);
    if (!okSettings.getShowSystemTray()) {
      show();
    }


    connect(&settings, &Settings::separateWindowChanged, this,
            &Widget::onSeparateWindowClicked);

#ifdef Q_OS_MAC
    Nexus::getInstance().updateWindows();
#endif

  init();
}

void Widget::init() {
  profile = Nexus::getProfile();

  connect(this, &Widget::toSendMessage, [&](){
      ui->tabWidget->setCurrentIndex(0);
  });

  connect(this, &Widget::toShowDetails, [&](){
    ui->tabWidget->setCurrentIndex(1);
  });

#if UPDATE_CHECK_ENABLED
  updateCheck = std::unique_ptr<UpdateCheck>(new UpdateCheck(settings));
  connect(updateCheck.get(), &UpdateCheck::updateAvailable, this,
          &Widget::onUpdateAvailable);
#endif

#if UPDATE_CHECK_ENABLED
  updateCheck->checkForUpdate();
#endif



}

bool Widget::eventFilter(QObject *obj, QEvent *event) {
  QWindowStateChangeEvent *ce = nullptr;
  Qt::WindowStates state = windowState();

  switch (event->type()) {
  case QEvent::Close:
    // It's needed if user enable `Close to tray`
    wasMaximized = state.testFlag(Qt::WindowMaximized);
    break;
  case QEvent::WindowStateChange:
    ce = static_cast<QWindowStateChangeEvent *>(event);
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

void Widget::updateIcons() {
  if (!icon) {
    return;
  }


  // Some builds of Qt appear to have a bug in icon loading:
  // QIcon::hasThemeIcon is sometimes unaware that the icon returned
  // from QIcon::fromTheme was a fallback icon, causing hasThemeIcon to
  // incorrectly return true.
  //
  // In qTox this leads to the tray and window icons using the static qTox logo
  // icon instead of an icon based on the current presence status.
  //
  // This workaround checks for an icon that definitely does not exist to
  // determine if hasThemeIcon can be trusted.
  //
  // On systems with the Qt bug, this workaround will always use our included
  // icons but user themes will be unable to override them.
  static bool checkedHasThemeIcon = false;
  static bool hasThemeIconBug = false;

  if (!checkedHasThemeIcon) {
    hasThemeIconBug = QIcon::hasThemeIcon("qtox-asjkdfhawjkeghdfjgh");
    checkedHasThemeIcon = true;

    if (hasThemeIconBug) {
      qDebug() << "Detected buggy QIcon::hasThemeIcon. Icon overrides from "
                  "theme will be ignored.";
    }
  }

//  QIcon ico;
//  if (!hasThemeIconBug && QIcon::hasThemeIcon("qtox-" + assetSuffix)) {
//    ico = QIcon::fromTheme("qtox-" + assetSuffix);
//  } else {
//    QString color = settings.getLightTrayIcon() ? "light" : "dark";
//    QString path =
//        ":/img/taskbar/" + color + "/taskbar_" + assetSuffix + ".svg";
//    QSvgRenderer renderer(path);
//
//    // Prepare a QImage with desired characteritisc
//    QImage image = QImage(250, 250, QImage::Format_ARGB32);
//    image.fill(Qt::transparent);
//    QPainter painter(&image);
//    renderer.render(&painter);
//    ico = QIcon(QPixmap::fromImage(image));
//  }

//  setWindowIcon(ico);
//  if (icon) {
//    icon->setIcon(ico);
//  }
}

Widget::~Widget() {
  qDebug() << __func__;

  settings::Translator::unregister(this);
  if (icon) {
    icon->hide();
  }

  delete timer;
  delete trayMenu;
  delete ui;
}

/**
 * @brief Switches to the About settings page.
 */
void Widget::showUpdateDownloadProgress() {
  onShowSettings();
  settingsWidget->showAbout();
}

void Widget::moveEvent(QMoveEvent *event) {
  if (event->type() == QEvent::Move) {
    saveWindowGeometry();
    saveSplitterGeometry();
  }

  QWidget::moveEvent(event);
}

void Widget::closeEvent(QCloseEvent *event) {
  //  if (settings.getShowSystemTray() && settings.getCloseToTray()) {
  QWidget::closeEvent(event);
  //  } else {
  //    if (autoAwayActive) {
  //      emit statusSet(Status::Status::Online);
  //      autoAwayActive = false;
  //    }
  //    saveWindowGeometry();
  //    saveSplitterGeometry();
  //    QWidget::closeEvent(event);
  //    qApp->quit();
  //  }
}

void Widget::changeEvent(QEvent *event) {
  //  if (event->type() == QEvent::WindowStateChange) {
  //    if (isMinimized() && settings.getShowSystemTray() &&
  //        settings.getMinimizeToTray()) {
  //      this->hide();
  //    }
  //  }
}

void Widget::resizeEvent(QResizeEvent *event) {
  saveWindowGeometry();
  QWidget::resizeEvent(event);
}

QString Widget::getUsername() { return core->getUsername(); }

void Widget::onSelfAvatarLoaded(const QPixmap &pic) {

  if(pic.size().isEmpty()){
    qWarning()<<__func__<<"pic is empty.";
    return;
  }
//  profilePicture->setPixmap(pic);
}

void Widget::onCoreChanged(Core &coreRef) {
  core = &coreRef;
  connectToCore(coreRef);
}

void Widget::connectToCore(Core &core) {
  connect(&core, &Core::connected, this, &Widget::onConnected);
  connect(&core, &Core::disconnected, this, &Widget::onDisconnected);
  connect(&core, &Core::statusSet, this, &Widget::onStatusSet);
  connect(&core, &Core::usernameSet, this, &Widget::setUsername);
  connect(&core, &Core::avatarSet, this, &Widget::setAvatar);
  connect(&core, &Core::failedToAddFriend, this, &Widget::addFriendFailed);

  connect(this, &Widget::statusSet, &core, &Core::setStatus);
  connect(this, &Widget::changeGroupTitle, &core, &Core::setGroupName);

  connect(timer, &QTimer::timeout, this, &Widget::onUserAwayCheck);
  connect(timer, &QTimer::timeout, this, &Widget::onEventIconTick);
  connect(timer, &QTimer::timeout, this, &Widget::onTryCreateTrayIcon);



  core.setUIStarted();
}

void Widget::onConnected() {
//  ui->statusButton->setEnabled(true);
//  emit core->statusSet(core->getStatus());
}

void Widget::onDisconnected() {
//  ui->statusButton->setEnabled(false);
//  emit core->statusSet(Status::Status::Offline);
}

void Widget::onFailedToStartCore() {
  QMessageBox critical(this);
  critical.setText(tr("toxcore failed to start, the application will terminate "
                      "after you close this message."));
  critical.setIcon(QMessageBox::Critical);
  critical.exec();
  qApp->exit(EXIT_FAILURE);
}

void Widget::onBadProxyCore() {
  settings.setProxyType(Settings::ProxyType::ptNone);
  QMessageBox critical(this);
  critical.setText(tr("toxcore failed to start with your proxy settings. "
                      "qTox cannot run; please modify your "
                      "settings and restart.",
                      "popup text"));
  critical.setIcon(QMessageBox::Critical);
  critical.exec();
  onShowSettings();
}

void Widget::onStatusSet(Status::Status status) {
//  ui->statusButton->setProperty("status", static_cast<int>(status));
//  ui->statusButton->setIcon(
//      prepareIcon(getIconPath(status), icon_size, icon_size));
//  updateIcons();
}

void Widget::onSeparateWindowClicked(bool separate) {
  onSeparateWindowChanged(separate, true);
}

void Widget::onSeparateWindowChanged(bool separate, bool clicked) {
  if (!separate) {
    QWindowList windowList = QGuiApplication::topLevelWindows();

    for (QWindow *window : windowList) {
      if (window->objectName() == "detachedWindow") {
        window->close();
      }
    }


//    SplitterRestorer restorer(ui->mainSplitter);
//    restorer.restore(settings.getSplitterState(), size());

    onShowSettings();
  } else {
//    int width = ui->friendList->size().width();
    QSize size;
    QPoint pos;

//    if (contentLayout) {
//      pos = mapToGlobal(ui->mainSplitter->widget(1)->pos());
//      size = ui->mainSplitter->widget(1)->size();
//    }

//    if (contentLayout) {
//      contentLayout->clear();
//      contentLayout->parentWidget()->setParent(
//          nullptr); // Remove from splitter.
//      contentLayout->parentWidget()->hide();
//      contentLayout->parentWidget()->deleteLater();
//      contentLayout->deleteLater();
//      contentLayout = nullptr;
//    }

//    setMinimumWidth(ui->tooliconsZone->sizeHint().width());

    if (clicked) {
      showNormal();
//      resize(width, height());

      if (settingsWidget) {
        ContentLayout *contentLayout =
            createContentDialog((DialogType::SettingDialog));
        contentLayout->parentWidget()->resize(size);
        contentLayout->parentWidget()->move(pos);
        settingsWidget->show(contentLayout);
      }
    }

    setWindowTitle(QString());
  }
}

void Widget::setWindowTitle(const QString &title) {
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

void Widget::confirmExecutableOpen(const QFileInfo &file) {
  static const QStringList dangerousExtensions = {
      "app",     "bat",    "com",  "cpl", "dmg", "exe",  "hta",     "jar",
      "js",      "jse",    "lnk",  "msc", "msh", "msh1", "msh1xml", "msh2",
      "msh2xml", "mshxml", "msi",  "msp", "pif", "ps1",  "ps1xml",  "ps2",
      "ps2xml",  "psc1",   "psc2", "py",  "reg", "scf",  "sh",      "src",
      "vb",      "vbe",    "vbs",  "ws",  "wsc", "wsf",  "wsh"};

  if (dangerousExtensions.contains(file.suffix())) {
    bool answer = GUI::askQuestion(
        tr("Executable file", "popup title"),
        tr("You have asked qTox to open an executable file. "
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

void Widget::onShowSettings() {
  if (settings.getSeparateWindow()) {
    if (!settingsWidget->isShown()) {
      settingsWidget->show(createContentDialog(DialogType::SettingDialog));
    }

//    setActiveToolMenuButton(ActiveToolMenuButton::None);
  } else {
    hideMainForms(nullptr);
//    settingsWidget->show(contentLayout);
    //    setWindowTitle(fromDialogType(DialogType::SettingDialog));
//    setActiveToolMenuButton(ActiveToolMenuButton::SettingButton);
  }
}


void Widget::hideMainForms(GenericChatroomWidget *chatroomWidget) {

}

void Widget::setUsername(const QString &username) {

}

void Widget::onStatusMessageChanged(const QString &newStatusMessage) {
  // Keep old status message until Core tells us to set it.
  core->setStatusMessage(newStatusMessage);
}

void Widget::setAvatar( QByteArray avatar) {
    if (avatar.isEmpty())
    {
        qWarning() << __func__ <<"avatar is empty!";
        return;
    }

    QPixmap pixmap;
  if(!base::Images::putToPixmap(avatar, pixmap))
  {
    qWarning()<<"loadFromData failed.";
    return;
  }

  emit avatarSet(pixmap);

//  profilePicture->setPixmap(pixmap);
//  profileInfo->setAvatar(pixmap);

}

/**
 * @brief Plays a sound via the audioNotification AudioSink
 * @param sound Sound to play
 * @param loop if true, loop the sound until onStopNotification() is called
 */
void Widget::playNotificationSound(IAudioSink::Sound sound, bool loop) {
  if (!settings.getAudioOutDevEnabled()) {
    // don't try to play sounds if audio is disabled
    return;
  }

  if (audioNotification == nullptr) {
    audioNotification = std::unique_ptr<IAudioSink>(audio.makeSink());
    if (audioNotification == nullptr) {
      qDebug() << "Failed to allocate AudioSink";
      return;
    }
  }

  audioNotification->connectTo_finishedPlaying(
      this, [this]() { cleanupNotificationSound(); });

  audioNotification->playMono16Sound(sound);

  if (loop) {
    audioNotification->startLoop();
  }
}

void Widget::cleanupNotificationSound() { audioNotification.reset(); }

void Widget::incomingNotification(QString friendnumber) {
  const auto &friendId = FriendId(friendnumber);
  newFriendMessageAlert(friendId, {}, false);

  // loop until call answered or rejected
  playNotificationSound(IAudioSink::Sound::IncomingCall, true);
}

void Widget::outgoingNotification() {
  // loop until call answered or rejected
  playNotificationSound(IAudioSink::Sound::OutgoingCall, true);
}

/**
 * @brief Widget::onStopNotification Stop the notification sound.
 */
void Widget::onStopNotification() { audioNotification.reset(); }


void Widget::addFriendFailed(const FriendId &, const QString &errorInfo) {
  QString info = QString(tr("Couldn't request friendship"));
  if (!errorInfo.isEmpty()) {
    info = info + QStringLiteral(": ") + errorInfo;
  }

  QMessageBox::critical(nullptr, "Error", info);
}


void Widget::onChatroomWidgetClicked(GenericChatroomWidget *widget) {
  openDialog(widget, /* newWindow = */ false);
}

void Widget::openNewDialog(GenericChatroomWidget *widget) {
  openDialog(widget, /* newWindow = */ true);
}

void Widget::openDialog(GenericChatroomWidget *widget, bool newWindow) {
  widget->resetEventFlags();
//  widget->updateStatusLight();

//  GenericChatForm *form;
//  GroupId id;
////  const IMFriend *frnd = widget->getFriend();
////  const Group *group = widget->getGroup();
////  if (frnd) {
////    form = chatForms[frnd->getPublicKey()];
////  } else if (group) {
////    id = group->getPersistentId();
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
////      groupChatForms[group->getPersistentId()]->show(contentLayout);
//    }
//    widget->setAsActiveChatroom();
//    setWindowTitle(widget->getSubject());
//  }
}

bool Widget::newFriendMessageAlert(const FriendId &friendId, const QString &text,
                                   bool sound, bool file) {
  bool hasActive;
  QWidget *currentWindow;
  ContentDialog *contentDialog =
      ContentDialogManager::getInstance()->getFriendDialog(friendId);
  Friend *f = FriendList::findFriend(friendId);

  if (contentDialog != nullptr) {
    currentWindow = contentDialog->window();
    hasActive = ContentDialogManager::getInstance()->isContactActive(friendId);
  } else {
    if (settings.getSeparateWindow() && settings.getShowWindow()) {
      if (settings.getDontGroupWindows()) {
        contentDialog = createContentDialog();
      } else {
        contentDialog = ContentDialogManager::getInstance()->current();
        if (!contentDialog) {
          contentDialog = createContentDialog();
        }
      }

      currentWindow = contentDialog->window();
      hasActive =
          ContentDialogManager::getInstance()->isContactActive(friendId);
    } else {
      currentWindow = window();
   }
  }

  if (newMessageAlert(currentWindow, hasActive, sound)) {
#if DESKTOP_NOTIFICATIONS
    if (settings.getNotifyHide()) {
      notifier.notifyMessageSimple(file
                                       ? DesktopNotify::MessageType::FRIEND_FILE
                                       : DesktopNotify::MessageType::FRIEND);
    } else {
      QString title = f->getDisplayedName();
      if (file) {
        title += " - " + tr("File sent");
      }
      notifier.notifyMessagePixmap(
          title, text, Nexus::getProfile()->loadAvatar(f->getPublicKey()));
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

bool Widget::newGroupMessageAlert(const GroupId &groupId, const FriendId &authorPk,
                                  const QString &message, bool notify) {

    qDebug() << __func__ <<"groupId" << groupId.toString()<< "message"<< message;
    bool hasActive = false;
    QWidget *currentWindow =
    currentWindow = window();


  if (!newMessageAlert(currentWindow, hasActive, true, notify)) {
    return false;
  }

//  g->setEventFlag(true);
//  widget->updateStatusLight();
#if DESKTOP_NOTIFICATIONS
  if (settings.getNotifyHide()) {
    notifier.notifyMessageSimple(DesktopNotify::MessageType::GROUP);
  } else {
    IMFriend *f = FriendList::findFriend(authorPk);
    QString title =
        g->getPeerList().value(authorPk) + " (" + g->getDisplayedName() + ")";
    if (!f) {
      notifier.notifyMessage(title, message);
    } else {
      notifier.notifyMessagePixmap(
          title, message, Nexus::getProfile()->loadAvatar(f->getPublicKey()));
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

bool Widget::newMessageAlert(QWidget *currentWindow, bool isActive, bool sound,
                             bool notify) {
  bool inactiveWindow = isMinimized() || !currentWindow->isActiveWindow();

  if (!inactiveWindow && isActive) {
    return false;
  }

  if (notify) {
      auto &settings = Settings::getInstance();

    if (settings.getShowWindow()) {
      currentWindow->show();
    }

    if (settings.getNotify()) {
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
      bool isBusy = core->getStatus() == Status::Status::Busy;
      bool busySound = settings.getBusySound();
      bool notifySound = settings.getNotifySound();

      if (notifySound && sound && (!isBusy || busySound)) {
        playNotificationSound(IAudioSink::Sound::NewMessage);
      }
    }
  }

  return true;
}

void Widget::friendRequestedTo(const ToxId &friendAddress, const QString &nick, const QString &message){
  qDebug()<<"friendRequestedTo"<<friendAddress.getPublicKey().toString()<<nick<<message;
  auto frd = FriendList::findFriend(friendAddress.getPublicKey());
  if(frd){
    return;
  }
  emit friendRequested(friendAddress, nick, message);
}



void Widget::onFileReceiveRequested(const ToxFile &file) {
  const FriendId &friendPk = FriendId(file.receiver);
  newFriendMessageAlert(
      friendPk,
      file.fileName + " (" +
          FileTransferWidget::getHumanReadableSize(file.fileSize) + ")",
      true, true);
}

void Widget::onDialogShown(GenericChatroomWidget *widget) {
  widget->resetEventFlags();
//  widget->updateStatusLight();

//  ui->friendList->updateTracking(widget);
  resetIcon();
}

void Widget::onFriendDialogShown(const Friend *f) {
//  onDialogShown(contactListWidget->getFriend(f->getPublicKey()));
}

void Widget::onGroupDialogShown(const Group *g) {
  const GroupId &groupId = g->getPersistentId();
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

ContentDialog *Widget::createContentDialog() const {
  ContentDialog *contentDialog = new ContentDialog();

  registerContentDialog(*contentDialog);
  return contentDialog;
}

void Widget::registerContentDialog(ContentDialog &contentDialog) const {
  ContentDialogManager::getInstance()->addContentDialog(contentDialog);
  connect(&contentDialog, &ContentDialog::friendDialogShown, this,
          &Widget::onFriendDialogShown);
  connect(&contentDialog, &ContentDialog::groupDialogShown, this,
          &Widget::onGroupDialogShown);
  connect(core, &Core::usernameSet, &contentDialog,
          &ContentDialog::setUsername);
  connect(&settings, &Settings::groupchatPositionChanged, &contentDialog,
          &ContentDialog::reorderLayouts);

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
}

ContentLayout *Widget::createContentDialog(DialogType type) const {
  class Dialog : public ActivateDialog {
  public:
    explicit Dialog(DialogType type, Settings &settings, Core *core)
        : ActivateDialog(nullptr, Qt::Window), type(type), settings(settings),
          core{core} {
      restoreGeometry(settings.getDialogSettingsGeometry());
      settings::Translator::registerHandler(
          std::bind(&Dialog::retranslateUi, this), this);
      retranslateUi();
      setWindowIcon(QIcon(":/img/icons/qtox.svg"));
      setStyleSheet(Style::getStylesheet("window/general.css"));

      connect(core, &Core::usernameSet, this, &Dialog::retranslateUi);
    }

    ~Dialog() { settings::Translator::unregister(this); }

  public slots:

    void retranslateUi() {
      setWindowTitle(core->getUsername() + QStringLiteral(" - ") +
                     Widget::fromDialogType(type));
    }

  protected:
    void resizeEvent(QResizeEvent *event) override {
      settings.setDialogSettingsGeometry(saveGeometry());
      QDialog::resizeEvent(event);
    }

    void moveEvent(QMoveEvent *event) override {
      settings.setDialogSettingsGeometry(saveGeometry());
      QDialog::moveEvent(event);
    }

  private:
    DialogType type;
    Settings &settings;
    Core *core;
  };

  Dialog *dialog = new Dialog(type, settings, core);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  ContentLayout *contentLayoutDialog = new ContentLayout(dialog);

  dialog->setObjectName("detached");
  dialog->setLayout(contentLayoutDialog);
  dialog->layout()->setMargin(0);
  dialog->layout()->setSpacing(0);
  dialog->setMinimumSize(720, 400);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();

#ifdef Q_OS_MAC
  connect(dialog, &Dialog::destroyed, &Nexus::getInstance(),
          &Nexus::updateWindowsClosed);
  connect(dialog, &ActivateDialog::windowStateChanged, &Nexus::getInstance(),
          &Nexus::updateWindowsStates);
  connect(dialog->windowHandle(), &QWindow::windowTitleChanged,
          &Nexus::getInstance(), &Nexus::updateWindows);
  Nexus::getInstance().updateWindows();
#endif

  return contentLayoutDialog;
}

void Widget::copyFriendIdToClipboard(const FriendId &friendId) {
  Friend *f = FriendList::findFriend(friendId);
  if (f != nullptr) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(friendId.toString(), QClipboard::Clipboard);
  }
}



void Widget::titleChangedByUser(const QString &title) {
  const auto *group = qobject_cast<Group *>(sender());
  assert(group != nullptr);
  emit changeGroupTitle(group->getId(), title);
}

void Widget::onGroupPeerAudioPlaying(QString groupnumber, FriendId peerPk) {
  const GroupId &groupId = GroupId(groupnumber);
  Group *g = GroupList::findGroup(groupId);
  if (!g) {
    qWarning() << "Can not find the group named:" << groupnumber;
    return;
  }

//  auto form = groupChatForms[groupId].data();
//  form->peerAudioPlaying(peerPk);
}

void Widget::removeGroup(Group *g, bool fake) {
//  const auto &groupId = g->getPersistentId();
//  const auto groupnumber = g->getId();
//  auto groupWidgetIt = groupWidgets.find(groupId);
//  if (groupWidgetIt == groupWidgets.end()) {
//    qWarning() << "Tried to remove group" << groupnumber
//               << "but GroupWidget doesn't exist";
//    return;
//  }
//  auto widget = groupWidgetIt.value();
//  widget->setAsInactiveChatroom();
//  if (static_cast<GenericChatroomWidget *>(widget) == activeChatroomWidget) {
//    activeChatroomWidget = nullptr;
//    onAddClicked();
//  }
//
//  GroupList::removeGroup(groupId, fake);
//  ContentDialog *contentDialog =
//      ContentDialogManager::getInstance()->getGroupDialog(groupId);
//  if (contentDialog != nullptr) {
//    contentDialog->removeGroup(groupId);
//  }
//
//  if (!fake) {
//    core->destroyGroup(groupnumber);
//  }else{
//    core->leaveGroup(groupnumber);
//  }
//
//  contactListWidget->removeGroupWidget(widget); // deletes widget
//
//  groupWidgets.remove(groupId);
//  auto groupChatFormIt = groupChatForms.find(groupId);
//  if (groupChatFormIt == groupChatForms.end()) {
//    qWarning() << "Tried to remove group" << groupnumber
//               << "but GroupChatForm doesn't exist";
//    return;
//  }
//  groupChatForms.erase(groupChatFormIt);
//  delete g;
////  if (contentLayout && contentLayout->mainHead->layout()->isEmpty()) {
////    onAddClicked();
////  }
//
//  groupAlertConnections.remove(groupId);
//
//  contactListWidget->reDraw();
}

void Widget::removeGroup(const GroupId &groupId) {
  removeGroup(GroupList::findGroup(groupId));
}

void Widget::destroyGroup(const GroupId &groupId) {
  removeGroup(GroupList::findGroup(groupId), false);
}

GroupWidget *Widget::createGroup(QString groupnumber,
                           const GroupId &groupId,
                           const QString& groupName) {


//  auto newgroup = contactListWidget->addGroup(groupnumber, groupId, groupName);
//  qDebug() << "createGroup" << groupnumber
//            << groupName;
//
//  Group *g = GroupList::findGroup(groupId);
//  if (g) {
//    qWarning() << "Group already exists" << groupnumber << "=>group:" << g;
//    return g;
//  }
//
//  const bool enabled = core->getGroupAvEnabled(groupnumber);
//  Group *newgroup = GroupList::addGroup(groupnumber, groupId, groupName,
//                                        enabled, core->getUsername());
//
//  auto dialogManager = ContentDialogManager::getInstance();
//  auto rawChatroom = new GroupChatroom(newgroup, dialogManager);
//  std::shared_ptr<GroupChatroom> chatroom(rawChatroom);
//
//  const auto compact = settings.getCompactLayout();
//  auto widget = new GroupWidget(chatroom, compact);
//  auto messageProcessor = MessageProcessor(sharedMessageProcessorParams);
//  auto messageDispatcher = std::make_shared<GroupMessageDispatcher>(
//      *newgroup, std::move(messageProcessor), *core, *core,
//      Settings::getInstance());
//  auto groupChatLog = std::make_shared<SessionChatLog>(*core);
//
//  connect(messageDispatcher.get(), &IMessageDispatcher::messageReceived,
//          groupChatLog.get(), &SessionChatLog::onMessageReceived);
//  connect(messageDispatcher.get(), &IMessageDispatcher::messageSent,
//          groupChatLog.get(), &SessionChatLog::onMessageSent);
//  connect(messageDispatcher.get(), &IMessageDispatcher::messageComplete,
//          groupChatLog.get(), &SessionChatLog::onMessageComplete);
//
//  auto notifyReceivedCallback = [this, groupId](const ToxPk &author,
//                                                const Message &message) {
//    auto isTargeted =
//        std::any_of(message.metadata.begin(), message.metadata.end(),
//                    [](MessageMetadata metadata) {
//                      return metadata.type == MessageMetadataType::selfMention;
//                    });
//    newGroupMessageAlert(groupId, author, message.content,
//                         isTargeted || settings.getGroupAlwaysNotify());
//  };
//
//  auto notifyReceivedConnection =
//      connect(messageDispatcher.get(), &IMessageDispatcher::messageReceived,
//              notifyReceivedCallback);
//  groupAlertConnections.insert(groupId, notifyReceivedConnection);
//
//  auto form =
//      new GroupChatForm(newgroup, *groupChatLog, *messageDispatcher, settings);
//  connect(&settings, &Settings::nameColorsChanged, form,
//          &GenericChatForm::setColorizedNames);
//  form->setColorizedNames(settings.getEnableGroupChatsColor());
//  groupMessageDispatchers[groupId] = messageDispatcher;
//  groupChatLogs[groupId] = groupChatLog;
//  groupWidgets[groupId] = widget;
//  groupChatrooms[groupId] = chatroom;
//  groupChatForms[groupId] = QSharedPointer<GroupChatForm>(form);
//
//  contactListWidget->addGroupWidget(widget);
//
//  widget->updateStatusLight();
//  contactListWidget->activateWindow();
//
//  connect(widget, &GroupWidget::chatroomWidgetClicked, this,
//          &Widget::onChatroomWidgetClicked);
//  connect(widget, &GroupWidget::newWindowOpened, this, &Widget::openNewDialog);
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
//  auto widgetRemoveGroup = QOverload<const GroupId &>::of(&Widget::removeGroup);
//  auto widgetDestroyGroup = QOverload<const GroupId &>::of(&Widget::destroyGroup);
//#else
//  auto widgetRemoveGroup =
//      static_cast<void (Widget::*)(const GroupId &)>(&Widget::removeGroup);
//  auto widgetDestroyGroup =
//      static_cast<void (Widget::*)(const GroupId &)>(&Widget::destroyGroup);
//#endif
//  connect(widget, &GroupWidget::removeGroup, this, widgetRemoveGroup);
//  connect(widget, &GroupWidget::destroyGroup, this, widgetDestroyGroup);
////  connect(widget, &GroupWidget::middleMouseClicked, this,
////          [this]() { removeGroup(groupId); });
//  connect(widget, &GroupWidget::chatroomWidgetClicked, form,
//          &ChatForm::focusInput);
//  connect(newgroup, &Group::titleChangedByUser, this,
//          &Widget::titleChangedByUser);
//  connect(core, &Core::usernameSet, newgroup, &Group::setSelfName);
//
//  FilterCriteria filter = getFilterCriteria();
//  widget->searchName(ui->searchContactText->text(), filterGroups(filter));

//  return newgroup;

return nullptr;
}

/**
 * @brief Used to reset the blinking icon.
 */
void Widget::resetIcon() {
  eventIcon = false;
  eventFlag = false;
  updateIcons();
}

bool Widget::event(QEvent *e) {
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
    Nexus::getInstance().updateWindowsStates();
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
//  bool online = static_cast<Status::Status>(
//                    ui->statusButton->property("status").toInt()) ==
//                Status::Status::Online;
//  bool away = autoAwayTime && Platform::getIdleTime() >= autoAwayTime;

//  if (online && away) {
//    qDebug() << "auto away activated at" << QTime::currentTime().toString();
//    emit statusSet(Status::Status::Away);
//    autoAwayActive = true;
//  } else if (autoAwayActive && !away) {
//    qDebug() << "auto away deactivated at" << QTime::currentTime().toString();
//    emit statusSet(Status::Status::Online);
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

void Widget::onTryCreateTrayIcon() {
  static int32_t tries = 15;
  if (!icon && tries--) {
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
      icon = std::unique_ptr<QSystemTrayIcon>(new QSystemTrayIcon);
      updateIcons();
      trayMenu = new QMenu(this);

      // adding activate to the top, avoids accidentally clicking quit
      trayMenu->addAction(actionShow);
      trayMenu->addSeparator();
      trayMenu->addAction(statusOnline);
      trayMenu->addAction(statusAway);
      trayMenu->addAction(statusBusy);
      trayMenu->addSeparator();
      trayMenu->addAction(actionLogout);
      trayMenu->addAction(actionQuit);
      icon->setContextMenu(trayMenu);

      connect(icon.get(), &QSystemTrayIcon::activated, this,
              &Widget::onIconClick);

      auto &okSettings = ok::base::OkSettings::getInstance();
      if (okSettings.getShowSystemTray()) {
        icon->show();
        setHidden(okSettings.getAutostartInTray());
      } else {
        show();
      }

#ifdef Q_OS_MAC
      Nexus::getInstance().dockMenu->setAsDockMenu();
#endif
    } else if (!isVisible()) {
      show();
    }
  } else {
    disconnect(timer, &QTimer::timeout, this, &Widget::onTryCreateTrayIcon);
    if (!icon) {
      qWarning() << "No system tray detected!";
      show();
    }
  }
}

void Widget::setStatusOnline() {
//  if (!ui->statusButton->isEnabled()) {
//    return;
//  }

  core->setStatus(Status::Status::Online);
}

void Widget::setStatusAway() {
//  if (!ui->statusButton->isEnabled()) {
//    return;
//  }

  core->setStatus(Status::Status::Away);
}

void Widget::setStatusBusy() {
//  if (!ui->statusButton->isEnabled()) {
//    return;
//  }

  core->setStatus(Status::Status::Busy);
}



void Widget::onSetShowSystemTray(bool newValue) {
  if (icon) {
    icon->setVisible(newValue);
  }
}

void Widget::saveWindowGeometry() {
  settings.setWindowGeometry(saveGeometry());
  //  settings.setWindowState(saveState());
}

void Widget::saveSplitterGeometry() {
  if (!settings.getSeparateWindow()) {
//    settings.setSplitterState(ui->mainSplitter->saveState());
  }
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
  this->setStyleSheet(Style::getStylesheet("window/general.css"));
  chatWidget->reloadTheme();
  contactWidget->reloadTheme();
}

void Widget::retranslateUi() {
  ui->retranslateUi(this);
  ui->tabWidget->setTabText(0, tr("Chat"));
  ui->tabWidget->setTabText(1, tr("Contact"));
  ui->tabWidget->setTabText(2, tr("Settings"));

#ifdef Q_OS_MAC
  Nexus::getInstance().retranslateUi();

  filterMenu->menuAction()->setText(tr("Filter..."));

  fileMenu->setText(tr("File"));
  editMenu->setText(tr("Edit"));
  contactMenu->setText(tr("Contacts"));
  changeStatusMenu->menuAction()->setText(tr("Change Status"));
  editProfileAction->setText(tr("Edit Profile"));
  logoutAction->setText(tr("Log out"));
  addContactAction->setText(tr("Add Contact..."));
  nextConversationAction->setText(tr("Next Conversation"));
  previousConversationAction->setText(tr("Previous Conversation"));
#endif
}
