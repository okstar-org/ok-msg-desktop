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
#include "src/model/chathistory.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/chatroom/groupchatroom.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/model/profile/profileinfo.h"
#include "src/model/status.h"
#include "src/net/updatecheck.h"
#include "src/nexus.h"
#include "src/persistence/offlinemsgengine.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/platform/timer.h"
#include "src/widget/contentdialogmanager.h"
#include "src/widget/form/addfriendform.h"
#include "src/widget/form/chatform.h"
#include "src/widget/form/filesform.h"
#include "src/widget/form/groupinviteform.h"
#include "src/widget/form/profileform.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/gui.h"
#include "src/widget/style.h"
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

namespace {

/**
 * @brief Dangerous way to find out if a path is writable.
 * @param filepath Path to file which should be deleted.
 * @return True, if file writeable, false otherwise.
 */
bool tryRemoveFile(const QString &filepath) {
  QFile tmp(filepath);
  bool writable = tmp.open(QIODevice::WriteOnly);
  tmp.remove();
  return writable;
}

void acceptFileTransfer(const ToxFile &file, const QString &path) {
  QString filepath;
  int number = 0;

  QString suffix = QFileInfo(file.fileName).completeSuffix();
  QString base = QFileInfo(file.fileName).baseName();

  do {
    filepath =
        QString("%1/%2%3.%4")
            .arg(path, base,
                 number > 0 ? QString(" (%1)").arg(QString::number(number))
                            : QString(),
                 suffix);
    ++number;
  } while (QFileInfo(filepath).exists());

  // Do not automatically accept the file-transfer if the path is not writable.
  // The user can still accept it manually.
  if (tryRemoveFile(filepath)) {
    CoreFile *coreFile = Core::getInstance()->getCoreFile();
    coreFile->acceptFileRecvRequest(file.friendId, file.fileNum, filepath);
  } else {
    qWarning() << "Cannot write to " << filepath;
  }
}
} // namespace

static Widget *instance = nullptr;

Widget *Widget::getInstance(IAudioControl *audio){
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

  setMinimumWidth(775);

  setObjectName(qsl("Page:%1").arg(static_cast<int>(UI::PageMenu::chat)));
//
//  QWidget *contentWidget = new QWidget(this);
//  contentWidget->setObjectName("contentWidget");
////  ui->mainSplitter->addWidget(contentWidget);

  chatWidget = std::make_unique<ChatWidget>();
  ui->tabWidget->addTab(chatWidget.get(), tr("Chat"));

  settingsWidget = std::make_unique<SettingsWidget>(updateCheck.get(), audio, this);
  ui->tabWidget->addTab(settingsWidget.get(), tr("Settings"));


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


    onSeparateWindowChanged(settings.getSeparateWindow(), false);

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

//  centralLayout = ui->mainSplitter;
//  setLayout(ui->centralLayout);
  // connect logout tray menu action
//  connect(actionLogout, &QAction::triggered,
//          profileForm,&ProfileForm::onLogoutClicked);

//  connect(profile, &Profile::selfAvatarChanged,
//          profileForm,&ProfileForm::onSelfAvatarLoaded);

//  connect(Nexus::getProfile(), &Profile::coreChanged,
//          chatWidget.get(), &ChatWidget::onCoreChanged);



  filesForm = new FilesForm();
//  addFriendForm = new AddFriendForm;
//  groupInviteForm = new GroupInviteForm;

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
  QWidgetList windowList = QApplication::topLevelWidgets();

  for (QWidget *window : windowList) {
    if (window != this) {
      window->close();
    }
  }

  settings::Translator::unregister(this);
  if (icon) {
    icon->hide();
  }

  for (Group *g : GroupList::getAllGroups()) {
    removeGroup(g, true);
  }

  for (Friend *f : FriendList::getAllFriends()) {
    removeFriend(f, true);
  }

  delete profileForm;
  delete profileInfo;


  delete filesForm;
  delete timer;


  FriendList::clear();
  GroupList::clear();
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
  qDebug() << "onSelfAvatarLoaded" << pic.size();
  if(pic.size().isEmpty()){
    qWarning()<<"pic is empty.";
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

  connect(&core, &Core::friendAddedDone, this, &Widget::addFriendDone);
  connect(&core, &Core::failedToAddFriend, this, &Widget::addFriendFailed);


  connect(&core, &Core::usernameSet, this, &Widget::refreshPeerListsLocal);
  connect(this, &Widget::statusSet, &core, &Core::setStatus);
  connect(this, &Widget::friendRequested, &core, &Core::requestFriendship);
  connect(this, &Widget::friendRequestAccepted, &core,
          &Core::acceptFriendRequest);
  connect(this, &Widget::friendRequestRejected, &core,
          &Core::rejectFriendRequest);
  connect(this, &Widget::changeGroupTitle, &core, &Core::changeGroupTitle);

  //  sharedMessageProcessorParams.setPublicKey(core.getSelfPublicKey().toString());






  //  connect(coreFile, &CoreFile::fileReceiveRequested, this,
  //          &Widget::onFileReceiveRequested);
  //  connect(coreFile, &CoreFile::fileDownloadFinished, filesForm,
  //          &FilesForm::onFileDownloadComplete);
  //  connect(coreFile, &CoreFile::fileUploadFinished, filesForm,
  //          &FilesForm::onFileUploadComplete);
  //  connect(ui->addButton, &QPushButton::clicked, this, &Widget::onAddClicked);
  //  connect(ui->groupButton, &QPushButton::clicked, this,
  //          &Widget::onGroupClicked);
  //  connect(ui->transferButton, &QPushButton::clicked, this,
  //          &Widget::onTransferClicked);
  //  connect(ui->settingsButton, &QPushButton::clicked, this,
  //          &Widget::onShowSettings);
  //  connect(profilePicture, &MaskablePixmapWidget::clicked, this,
  //          &Widget::showProfile);
  //  connect(ui->nameLabel, &CroppingLabel::clicked, this, &Widget::showProfile);
  //  connect(ui->statusLabel, &CroppingLabel::editFinished, this,
  //          &Widget::onStatusMessageChanged);
  //  connect(ui->mainSplitter, &QSplitter::splitterMoved, this,
  //          &Widget::onSplitterMoved);
  //  connect(addFriendForm, &AddFriendForm::friendRequested, this,
  //          &Widget::friendRequestedTo);

  profileInfo = new ProfileInfo(&core, profile);
  profileForm = new ProfileForm(profileInfo);



  connect(timer, &QTimer::timeout, this, &Widget::onUserAwayCheck);
  connect(timer, &QTimer::timeout, this, &Widget::onEventIconTick);
  //  connect(timer, &QTimer::timeout, this, &Widget::onTryCreateTrayIcon);


  //  connect(ui->friendList, &QWidget::customContextMenuRequested, this,
  //          &Widget::friendListContextMenu);

  auto coreFile = core.getCoreFile();
  connect(coreFile, &CoreFile::fileSendStarted, this, &Widget::dispatchFile);
  connect(coreFile, &CoreFile::fileReceiveRequested, this,
          &Widget::dispatchFile);
  connect(coreFile, &CoreFile::fileTransferAccepted, this,
          &Widget::dispatchFile);
  connect(coreFile, &CoreFile::fileTransferCancelled, this,
          &Widget::dispatchFile);
  connect(coreFile, &CoreFile::fileTransferFinished, this,
          &Widget::dispatchFile);
  connect(coreFile, &CoreFile::fileTransferPaused, this, &Widget::dispatchFile);
  connect(coreFile, &CoreFile::fileTransferInfo, this, &Widget::dispatchFile);
  connect(coreFile, &CoreFile::fileTransferRemotePausedUnpaused, this,
          &Widget::dispatchFileWithBool);
  connect(coreFile, &CoreFile::fileTransferBrokenUnbroken, this,
          &Widget::dispatchFileWithBool);
  connect(coreFile, &CoreFile::fileSendFailed, this,
          &Widget::dispatchFileSendFailed);


  //  core->setUIStarted();
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
        setActiveToolMenuButton(ActiveToolMenuButton::None);
      }
    }

    setWindowTitle(QString());
    setActiveToolMenuButton(ActiveToolMenuButton::None);
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
  if (settings.getSeparateWindow()) {
    if (!filesForm->isShown()) {
      filesForm->show(createContentDialog(DialogType::TransferDialog));
    }

    setActiveToolMenuButton(ActiveToolMenuButton::None);
  } else {
    hideMainForms(nullptr);
//    filesForm->show(contentLayout);
    setWindowTitle(fromDialogType(DialogType::TransferDialog));
    setActiveToolMenuButton(ActiveToolMenuButton::TransferButton);
  }
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

    setActiveToolMenuButton(ActiveToolMenuButton::None);
  } else {
    hideMainForms(nullptr);
//    settingsWidget->show(contentLayout);
    //    setWindowTitle(fromDialogType(DialogType::SettingDialog));
    setActiveToolMenuButton(ActiveToolMenuButton::SettingButton);
  }
}

void Widget::showProfile() // onAvatarClicked, onUsernameClicked
{
  if (settings.getSeparateWindow()) {
    if (!profileForm->isShown()) {
      profileForm->show(createContentDialog(DialogType::ProfileDialog));
    }

    setActiveToolMenuButton(ActiveToolMenuButton::None);
  } else {
    hideMainForms(nullptr);
//    profileForm->show(contentLayout);
    setWindowTitle(fromDialogType(DialogType::ProfileDialog));
    setActiveToolMenuButton(ActiveToolMenuButton::None);
  }
}

void Widget::hideMainForms(GenericChatroomWidget *chatroomWidget) {
  setActiveToolMenuButton(ActiveToolMenuButton::None);
}

void Widget::setUsername(const QString &username) {
  if (username.isEmpty()) {
//    ui->nameLabel->setText(tr("Your name"));
//    ui->nameLabel->setToolTip(tr("Your name"));
  } else {
//    ui->nameLabel->setText(username);
//    ui->nameLabel->setToolTip(Qt::convertFromPlainText(
//        username, Qt::WhiteSpaceNormal)); // for overlength names
  }

  sharedMessageProcessorParams.onUserNameSet(username);
}

void Widget::onStatusMessageChanged(const QString &newStatusMessage) {
  // Keep old status message until Core tells us to set it.
  core->setStatusMessage(newStatusMessage);
}

void Widget::setAvatar( QByteArray avatar) {
  qDebug() << "Widget::setAvatar size:" << avatar.size();

  if (avatar.isEmpty())
    return;

  QPixmap pixmap;
  if(!base::Images::putToPixmap(avatar, pixmap))
  {
    qWarning()<<"loadFromData failed.";
    return;
  }
//  profilePicture->setPixmap(pixmap);
  profileInfo->setAvatar(pixmap);

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
  const auto &friendId = FriendList::id2Key(friendnumber);
  newFriendMessageAlert(friendId, {}, false);

  // loop until call answered or rejected
  playNotificationSound(IAudioSink::Sound::IncomingCall, true);
}

void Widget::outgoingNotification() {
  // loop until call answered or rejected
  playNotificationSound(IAudioSink::Sound::OutgoingCall, true);
}

void Widget::onCallEnd() { playNotificationSound(IAudioSink::Sound::CallEnd); }

/**
 * @brief Widget::onStopNotification Stop the notification sound.
 */
void Widget::onStopNotification() { audioNotification.reset(); }

/**
 * @brief Dispatches file to the appropriate chatlog and accepts the transfer if
 * necessary
 */
void Widget::dispatchFile(ToxFile file) {
  const auto &friendId = FriendList::id2Key(file.friendId);
  Friend *f = FriendList::findFriend(friendId);
  if (!f) {
    return;
  }

  auto pk = f->getPublicKey();

  if (file.status == ToxFile::INITIALIZING &&
      file.direction == ToxFile::RECEIVING) {
    auto sender = (file.direction == ToxFile::SENDING)
                      ? Core::getInstance()->getSelfPublicKey()
                      : pk;

    const Settings &settings = Settings::getInstance();
    QString autoAcceptDir = settings.getAutoAcceptDir(f->getPublicKey());
    if (autoAcceptDir.isEmpty() &&
        ok::base::OkSettings::getInstance().getAutoSaveEnabled()) {
      autoAcceptDir = settings.getGlobalAutoAcceptDir();
    }

    auto maxAutoAcceptSize = settings.getMaxAutoAcceptSize();
    bool autoAcceptSizeCheckPassed =
        maxAutoAcceptSize == 0 || maxAutoAcceptSize >= file.filesize;

    if (!autoAcceptDir.isEmpty() && autoAcceptSizeCheckPassed) {
      acceptFileTransfer(file, autoAcceptDir);
    }
  }

  const auto senderPk =
      (file.direction == ToxFile::SENDING) ? core->getSelfPublicKey() : pk;
  friendChatLogs[pk]->onFileUpdated(senderPk, file);
}

void Widget::dispatchFileWithBool(ToxFile file, bool) { dispatchFile(file); }

void Widget::dispatchFileSendFailed(QString friendId, const QString &fileName) {
  const auto &friendPk = FriendList::id2Key(friendId);

//TODO
//  chatForm.value()->addSystemInfoMessage(
//      tr("Failed to send file \"%1\"").arg(fileName), ChatMessage::ERROR,
//      QDateTime::currentDateTime());
}

void Widget::onRejectCall(QString friendId) {
  core->getAv()->cancelCall(friendId);
}

void Widget::addFriend0(const ToxPk &friendPk) {
  core->requestFriendship(ToxId(friendPk.toString()), core->getNick(), "请求添加好友！");
}

void Widget::addFriend1(const ToxPk &friendId) {
  addFriend(friendId.username, friendId, true);
}

void Widget::addFriend(QString friendId, const ToxPk &friendPk, bool isFriend) {

//  auto friendWidget = contactListWidget->addFriend(
//                                                   friendId, friendPk, isFriend);


//  auto exist = FriendList::findFriend(friendPk);
//  if (exist) {
//    return;
//  }
//
//  settings.updateFriendAddress(friendPk.toString());
//
//  Friend *newfriend = FriendList::addFriend(friendId, friendPk, isFriend);
//  auto dialogManager = ContentDialogManager::getInstance();
//
//  auto rawChatroom = new FriendChatroom(newfriend, dialogManager);
//
//  std::shared_ptr<FriendChatroom> chatRoom(rawChatroom);
//  const auto compact = settings.getCompactLayout();
//
//  auto widget = new FriendWidget(chatRoom, compact);
//  connectFriendWidget(*widget);
//  auto history = Nexus::getProfile()->getHistory();
//
//  auto messageProcessor = MessageProcessor(sharedMessageProcessorParams);
//  auto messageDispatcher = std::make_shared<FriendMessageDispatcher>(
//      *newfriend, std::move(messageProcessor), *core);
//
//  // Note: We do not have to connect the message dispatcher signals since
//  // ChatHistory hooks them up in a very specific order
//  auto chatHistory = std::make_shared<ChatHistory>(*newfriend, history, *core,
//                                                   Settings::getInstance(),
//                                                   *messageDispatcher);
//  auto chatForm =
//      new ChatForm(newfriend, *chatHistory, *messageDispatcher);
//  connect(chatForm, &ChatForm::updateFriendActivity, this,
//          &Widget::updateFriendActivity);
//
//  friendMessageDispatchers[friendPk] = messageDispatcher;
//  friendChatLogs[friendPk] = chatHistory;
//  friendChatrooms[friendPk] = chatRoom;
//  friendWidgets[friendPk] = widget;
//  chatForms[friendPk] = chatForm;
//
//  const auto activityTime = settings.getFriendActivity(friendPk);
//  const auto chatTime = chatForm->getLatestTime();
//  if (chatTime > activityTime && chatTime.isValid()) {
//    settings.setFriendActivity(friendPk, chatTime);
//  }
//
//  contactListWidget->addFriendWidget(widget, Status::Status::Offline,
//                                     settings.getFriendCircleID(friendPk));
//
//  auto notifyReceivedCallback = [this, friendPk](const ToxPk &author,
//                                                 const Message &message) {
//    auto isTargeted =
//        std::any_of(message.metadata.begin(), message.metadata.end(),
//                    [](MessageMetadata metadata) {
//                      return metadata.type == MessageMetadataType::selfMention;
//                    });
//    newFriendMessageAlert(friendPk, message.content);
//  };
//
//  auto notifyReceivedConnection =
//      connect(messageDispatcher.get(),
//              &IMessageDispatcher::messageReceived, notifyReceivedCallback);
//
//  friendAlertConnections.insert(friendPk, notifyReceivedConnection);
//
//  connect(newfriend, &Friend::aliasChanged, this,
//          &Widget::onFriendAliasChanged);
//  connect(newfriend, &Friend::displayedNameChanged, this,
//          &Widget::onFriendDisplayedNameChanged);
//
//  connect(chatForm, &ChatForm::incomingNotification, this,
//          &Widget::incomingNotification);
//  connect(chatForm, &ChatForm::outgoingNotification, this,
//          &Widget::outgoingNotification);
//  connect(chatForm, &ChatForm::stopNotification, this,
//          &Widget::onStopNotification);
//  connect(chatForm, &ChatForm::endCallNotification, this, &Widget::onCallEnd);
//  connect(chatForm, &ChatForm::rejectCall, this, &Widget::onRejectCall);
//
//  connect(widget, &FriendWidget::newWindowOpened, this, &Widget::openNewDialog);
//  connect(widget, &FriendWidget::chatroomWidgetClicked, this,
//          &Widget::onChatroomWidgetClicked);
//  connect(widget, &FriendWidget::chatroomWidgetClicked, chatForm,
//          &ChatForm::focusInput);
//  connect(widget, &FriendWidget::friendHistoryRemoved, chatForm,
//          &ChatForm::clearChatArea);
//  connect(widget, &FriendWidget::copyFriendIdToClipboard, this,
//          &Widget::copyFriendIdToClipboard);
//
//  connect(widget, &FriendWidget::contextMenuCalled, widget,
//          &FriendWidget::onContextMenuCalled);
//
//  connect(widget, &FriendWidget::addFriend, //
//          this, &Widget::addFriend0);
//
//  connect(widget, SIGNAL(removeFriend(const ToxPk &)), this,
//          SLOT(removeFriend(const ToxPk &)));
//
//  Profile *profile = Nexus::getProfile();
//  connect(profile, &Profile::friendAvatarSet, widget,
//          &FriendWidget::onAvatarSet);
//  connect(profile, &Profile::friendAvatarRemoved, widget,
//          &FriendWidget::onAvatarRemoved);
//
//  // Try to get the avatar from the cache
//  QPixmap avatar = Nexus::getProfile()->loadAvatar(friendPk);
//  if (!avatar.isNull()) {
//    //chatForm->onAvatarChanged(friendPk, avatar);
//    //widget->onAvatarSet(friendPk, avatar);
//  }
//
//  FilterCriteria filter = getFilterCriteria();
//  widget->search(ui->searchContactText->text(), filterOffline(filter));

//  core->getFriendInfo(friendPk.toString());
}

void Widget::addFriendDone() {
  core->requestBookmarks();
}

void Widget::addFriendFailed(const ToxPk &, const QString &errorInfo) {
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
  widget->updateStatusLight();

  GenericChatForm *form;
  GroupId id;
  const Friend *frnd = widget->getFriend();
  const Group *group = widget->getGroup();
//  if (frnd) {
//    form = chatForms[frnd->getPublicKey()];
//  } else if (group) {
//    id = group->getPersistentId();
//    form = groupChatForms[id].data();
//  }
  bool chatFormIsSet;
  ContentDialogManager::getInstance()->focusContact(id);
  chatFormIsSet = ContentDialogManager::getInstance()->contactWidgetExists(id);

  if ((chatFormIsSet || form->isVisible()) && !newWindow) {
    return;
  }

  if (settings.getSeparateWindow() || newWindow) {
    ContentDialog *dialog = nullptr;

    if (!settings.getDontGroupWindows() && !newWindow) {
      dialog = ContentDialogManager::getInstance()->current();
    }

    if (dialog == nullptr) {
      dialog = createContentDialog();
    }

    dialog->show();

    if (frnd) {
      addFriendDialog(frnd, dialog);
    } else {
      Group *group = widget->getGroup();
      addGroupDialog(group, dialog);
    }

    dialog->raise();
    dialog->activateWindow();
  } else {
    hideMainForms(widget);
    if (frnd) {
//      chatForms[frnd->getPublicKey()]->show(contentLayout);
//    } else {
//      groupChatForms[group->getPersistentId()]->show(contentLayout);
    }
    widget->setAsActiveChatroom();
    setWindowTitle(widget->getTitle());
  }
}





void Widget::addFriendDialog(const Friend *frnd, ContentDialog *dialog) {
  QString friendId = frnd->getId();
  qDebug() << "addFriendDialog " << friendId;

  const ToxPk &friendPk = frnd->getPublicKey();
  qDebug() << "friendPk " << friendPk.toString();

  ContentDialog *contentDialog =
      ContentDialogManager::getInstance()->getFriendDialog(friendPk);

  bool isSeparate = settings.getSeparateWindow();
//  FriendWidget *widget = friendWidgets[friendPk];
//  bool isCurrent = activeChatroomWidget == widget;
//  if (!contentDialog && !isSeparate && isCurrent) {
//    onAddClicked();
//  }

//  auto form = chatForms[friendPk];
//  auto chatroom = friendChatrooms[friendPk];

//      ContentDialogManager::getInstance()->addFriendToDialog(friendPk,
//                                                             dialog,
//                                                             chatroom.get(),
//                                                             form);

//  friendWidget->setStatusMsg(widget->getStatusMsg());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
  auto widgetRemoveFriend = QOverload<const ToxPk &>::of(&Widget::removeFriend);
#else
  auto widgetRemoveFriend =
      static_cast<void (Widget::*)(const ToxPk &)>(&Widget::removeFriend);
#endif
//  connect(friendWidget, &FriendWidget::removeFriend, this, widgetRemoveFriend);
//  connect(friendWidget, &FriendWidget::addFriend, this, &Widget::addFriend0);


  // FIXME: emit should be removed
//  emit friendWidget->chatroomWidgetClicked(friendWidget);

  Profile *profile = Nexus::getProfile();
//  connect(profile, &Profile::friendAvatarSet, friendWidget,
//          &FriendWidget::onAvatarSet);
//  connect(profile, &Profile::friendAvatarRemoved, friendWidget,
//          &FriendWidget::onAvatarRemoved);

  QPixmap avatar = Nexus::getProfile()->loadAvatar(frnd->getPublicKey());
  if (!avatar.isNull()) {
   // friendWidget->onAvatarSet(frnd->getPublicKey(), avatar);
  }
}

void Widget::addGroupDialog(Group *group, ContentDialog *dialog) {
  const GroupId &groupId = group->getPersistentId();
  ContentDialog *groupDialog =
      ContentDialogManager::getInstance()->getGroupDialog(groupId);
  bool separated = settings.getSeparateWindow();
//  GroupWidget *widget = contactListWidget->getGroup(groupId);
//  bool isCurrentWindow = activeChatroomWidget == widget;
//  if (!groupDialog && !separated && isCurrentWindow) {
//    onAddClicked();
//  }

//  auto chatForm = groupChatForms[groupId].data();
//  auto chatroom = groupChatrooms[groupId];
//  auto groupWidget = ContentDialogManager::getInstance()->addGroupToDialog(groupId,
//      dialog, chatroom.get(), chatForm);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
  auto removeGroup = QOverload<const GroupId &>::of(&Widget::removeGroup);
  auto destroyGroup = QOverload<const GroupId &>::of(&Widget::destroyGroup);
#else
  auto removeGroup =
      static_cast<void (Widget::*)(const GroupId &)>(&Widget::destroyGroup);
  auto destroyGroup =
      static_cast<void (Widget::*)(const GroupId &)>(&Widget::destroyGroup);
#endif
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

  // FIXME: emit should be removed
//  emit widget->chatroomWidgetClicked(widget);
}

bool Widget::newFriendMessageAlert(const ToxPk &friendId, const QString &text,
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

      addFriendDialog(f, contentDialog);
      currentWindow = contentDialog->window();
      hasActive =
          ContentDialogManager::getInstance()->isContactActive(friendId);
    } else {
      currentWindow = window();
//      FriendWidget *widget = contactListWidget->getFriend(friendId);
//      hasActive = widget == activeChatroomWidget;
    }
  }

  if (newMessageAlert(currentWindow, hasActive, sound)) {
//    FriendWidget *widget = contactListWidget->getFriend(friendId);
//    f->setEventFlag(true);
//    widget->updateStatusLight();
//    ui->friendList->trackWidget(widget);
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
//        setWindowTitle(widget->getTitle());
      }
    } else {
      ContentDialogManager::getInstance()->updateFriendStatus(friendId);
    }

    return true;
  }

  return false;
}

bool Widget::newGroupMessageAlert(const GroupId &groupId, const ToxPk &authorPk,
                                  const QString &message, bool notify) {

    qDebug() << __func__ <<"groupId" << groupId.toString()<< "message"<< message;
    bool hasActive;
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
    Friend *f = FriendList::findFriend(authorPk);
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
//      setWindowTitle(widget->getTitle());
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
  const ToxPk &friendPk = FriendList::id2Key(file.friendId);
  newFriendMessageAlert(
      friendPk,
      file.fileName + " (" +
          FileTransferWidget::getHumanReadableSize(file.filesize) + ")",
      true, true);
}


void Widget::removeFriend(Friend *f, bool fake) {
  qDebug()<<"removeFriend:"<<f->getPublicKey().toString();

  if (!fake) {
    RemoveFriendDialog ask(this, f);
    ask.exec();

    if (!ask.accepted()) {
      return;
    }

    if (ask.removeHistory()) {
      auto *his = Nexus::getProfile()->getHistory();
      if (his) {
        his->removeFriendHistory(f->getPublicKey().toString());
      }
    }
  }

  const ToxPk friendPk = f->getPublicKey();
//  auto frdWidget = contactListWidget->getFriend(friendPk);
//  frdWidget->setAsInactiveChatroom();
//  if (frdWidget == activeChatroomWidget) {
//    activeChatroomWidget = nullptr;
//    onAddClicked();
//  }
//
//  friendAlertConnections.remove(friendPk);
//
//  contactListWidget->removeFriend(friendPk);
//
  ContentDialog *lastDialog =
      ContentDialogManager::getInstance()->getFriendDialog(friendPk);
  if (lastDialog != nullptr) {
    lastDialog->removeFriend(friendPk);
  }


  if (!fake) {
    core->removeFriend(f->getPublicKey().toString());
    // aliases aren't supported for non-friend peers in groups, revert to basic
    // username
    for (Group *g : GroupList::getAllGroups()) {
      if (g->getPeerList().contains(friendPk)) {
        g->updateUsername(friendPk, f->getUserName());
      }
    }
  }

//  frdWidget->removeFriend(friendPk);
//  delete frdWidget;

//  auto chatForm = chatForms[friendPk];
//  chatForms.remove(friendPk);
//  delete chatForm;

  delete f;
//  if (contentLayout && contentLayout->mainHead->layout()->isEmpty()) {
//    onAddClicked();
//  }

//  contactListWidget->reDraw();
}

void Widget::removeFriend(const ToxPk &friendId) {
  removeFriend(FriendList::findFriend(friendId), false);
}

void Widget::onDialogShown(GenericChatroomWidget *widget) {
  widget->resetEventFlags();
  widget->updateStatusLight();

//  ui->friendList->updateTracking(widget);
  resetIcon();
}

void Widget::onFriendDialogShown(const Friend *f) {
//  onDialogShown(contactListWidget->getFriend(f->getPublicKey()));
}

void Widget::onGroupDialogShown(Group *g) {
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
  connect(&contentDialog, &ContentDialog::addFriendDialog, this,
          &Widget::addFriendDialog);
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

void Widget::copyFriendIdToClipboard(const ToxPk &friendId) {
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

void Widget::onGroupPeerAudioPlaying(QString groupnumber, ToxPk peerPk) {
  const GroupId &groupId = GroupList::id2Key(groupnumber);
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

void Widget::onEmptyGroupCreated(QString groupnumber,
                                 const GroupId &groupId,
                                 const QString &title) {
  GroupWidget *group = createGroup(groupnumber, groupId, title);
  if (!group) {
    qWarning() <<"Unable to create group" << groupnumber;
    return;
  }

  if (!title.isEmpty()) {
    group->setName(title);
    // Only rename group if groups are visible.
//    if (groupsVisible()) {
//      groupWidgets[groupId]->editName();
//    }
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

bool Widget::event(QEvent *e) {
  switch (e->type()) {
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonDblClick:
    focusChatInput();
    break;
  case QEvent::Paint:
//    ui->friendList->updateVisualTracking();
    break;
  case QEvent::WindowActivate:

    if (eventFlag) {
      resetIcon();
    }

    focusChatInput();

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
  QList<Friend *> frnds = FriendList::getAllFriends();
  for (Friend *f : frnds) {
//    friendMessageDispatchers[f->getPublicKey()]->clearOutgoingMessages();
  }
}

void Widget::reloadTheme() {
  this->setStyleSheet(Style::getStylesheet("window/general.css"));

}

void Widget::nextContact() { cycleContacts(true); }

void Widget::previousContact() { cycleContacts(false); }









void Widget::friendListContextMenu(const QPoint &pos) {
  QMenu menu(this);
  QAction *createGroupAction = menu.addAction(tr("Create new group..."));
//  QAction *chosenAction = menu.exec(ui->friendList->mapToGlobal(pos));
//  if (chosenAction == createGroupAction) {
//    core->createGroup();
//  }
//
//  QAction *addCircleAction = menu.addAction(tr("Add new circle..."));
//  if (chosenAction == addCircleAction) {
//    contactListWidget->addCircleWidget();
//  } else
}




void Widget::setActiveToolMenuButton(ActiveToolMenuButton newActiveButton) {
//  ui->addButton->setChecked(newActiveButton == ActiveToolMenuButton::AddButton);
//  ui->addButton->setDisabled(newActiveButton ==
//                             ActiveToolMenuButton::AddButton);
//  ui->groupButton->setChecked(newActiveButton ==
//                              ActiveToolMenuButton::GroupButton);
//  ui->groupButton->setDisabled(newActiveButton ==
//                               ActiveToolMenuButton::GroupButton);
//  ui->transferButton->setChecked(newActiveButton ==
//                                 ActiveToolMenuButton::TransferButton);
//  ui->transferButton->setDisabled(newActiveButton ==
//                                  ActiveToolMenuButton::TransferButton);
//  ui->settingsButton->setChecked(newActiveButton ==
//                                 ActiveToolMenuButton::SettingButton);
//  ui->settingsButton->setDisabled(newActiveButton ==
//                                  ActiveToolMenuButton::SettingButton);
}

void Widget::retranslateUi() {
  ui->retranslateUi(this);
  //  setUsername(core->getUsername());
  //  setStatusMessage(core->getStatusMessage());




  if (!settings.getSeparateWindow() &&
      (settingsWidget && settingsWidget->isShown())) {
    setWindowTitle(fromDialogType(DialogType::SettingDialog));
  }


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

void Widget::focusChatInput() {
//  if (activeChatroomWidget) {
//    if (const Friend *f = activeChatroomWidget->getFriend()) {
//      chatForms[f->getPublicKey()]->focusInput();
//    } else if (Group *g = activeChatroomWidget->getGroup()) {
//      groupChatForms[g->getPersistentId()]->focusInput();
//    }
//  }
}

void Widget::refreshPeerListsLocal(const QString &username) {
  for (Group *g : GroupList::getAllGroups()) {
    g->updateUsername(core->getSelfPublicKey(), username);
  }
}







