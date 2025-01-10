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

#include "nexus.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDesktopWidget>

#include <cassert>
#include "Bus.h"
#include "application.h"
#include "lib/audio/audio.h"
#include "lib/storage/settings/OkSettings.h"
#include "lib/storage/settings/translator.h"
#include "persistence/settings.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/model/groupinvite.h"
#include "src/model/status.h"
#include "src/persistence/profile.h"
#include "src/widget/widget.h"
#include "video/camerasource.h"

#ifdef Q_OS_MAC
#include <QActionGroup>
#include <QMenuBar>
#include <QSignalMapper>
#include <QWindow>
#endif

/**
 * @class Nexus
 *
 * This class is in charge of connecting various systems together
 * and forwarding signals appropriately to the right objects,
 * it is in charge of starting the GUI and the Core.
 */

static Nexus* m_self;

Nexus::Nexus(QObject* parent)
        : name{OK_IM_MODULE}, stared(false), profile{nullptr}, m_widget{nullptr} {
    qDebug() << __func__;

    Q_INIT_RESOURCE(res);
    Q_INIT_RESOURCE(emojione);
    Q_INIT_RESOURCE(smileys);
    Q_INIT_RESOURCE(IM);

    // Setup the environment
    qRegisterMetaType<Status::Status>("Status::Status");

    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<const int16_t*>("const int16_t*");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<QPixmap>("QPixmap");
    qRegisterMetaType<Profile*>("Profile*");

    qRegisterMetaType<FileDirection>("FileDirection");
    qRegisterMetaType<FileStatus>("FileStatus");
    qRegisterMetaType<std::shared_ptr<VideoFrame>>("std::shared_ptr<VideoFrame>");
    qRegisterMetaType<FriendId>("ToxPk");
    qRegisterMetaType<ToxId>("ToxId");
    qRegisterMetaType<GroupId>("GroupId");
    qRegisterMetaType<ContactId>("ContactId");
    qRegisterMetaType<GroupInvite>("GroupInvite");
    qRegisterMetaType<MsgId>("MsgId");
    qRegisterMetaType<RowId>("RowId");

    // Create GUI
    m_widget = new Widget();

    //    connect(this, &Nexus::destroyProfile, this, &Nexus::do_logout);
}

Nexus::~Nexus() {
    qDebug() << __func__;
    if (m_widget) m_widget->deleteLater();

    emit saveGlobal();
#ifdef Q_OS_MAC
    delete globalMenuBar;
#endif
}

void Nexus::init(Profile* p) {
    //    profile = p;
    //    assert(profile);
}

void Nexus::onSave(SavedInfo& savedInfo) {
    auto s = getProfile()->getSettings();
    s->setWindowGeometry(savedInfo.windowGeometry);
    s->saveGlobal();
    m_widget->close();
}

/**
 * @brief Sets up invariants and calls showLogin
 *
 * Hides the login screen and shows the GUI for the given profile.
 * Will delete the current GUI, if it exists.
 */
void Nexus::start(std::shared_ptr<lib::session::AuthSession> session) {
    auto& signInInfo = session->getSignInInfo();
    qDebug() << __func__ << "for user:" << signInInfo.username;

    if (stared) {
        qWarning("This module is already started.");
        return;
    }

    //    QCommandLineParser parser;
    //    if (!Profile::exists(signInInfo.username)) {
    profile = Profile::createProfile(signInInfo.host, signInInfo.username, signInInfo.password);
    //    } else {
    //        profile = Profile::loadProfile(signInInfo.host, signInInfo.username, &parser,
    //                                       signInInfo.password);
    //    }

    if (!profile) {
        qWarning() << tr("Can not create profile!");
        emit createProfileFailed(tr("Can not create profile!"));
        return;
    }

    qDebug() << "Starting up";
    stared = true;

    auto& s = lib::settings::OkSettings::getInstance();
    QString locale = s.getTranslation();
    qDebug() << "locale" << locale;

    audioControl = std::unique_ptr<IAudioControl>(Audio::makeAudio(*profile->getSettings()));

    //  add_definitions(-D${PROJECT_NAME}_MODULE="${PROJECT_NAME}")
    settings::Translator::translate(OK_IM_MODULE, locale);

    qApp->setQuitOnLastWindowClosed(false);

    auto bus = ok::Application::Instance()->bus();

    // Connections
    connect(profile.get(), &Profile::selfAvatarChanged, m_widget, &Widget::onSelfAvatarLoaded);

    connect(profile.get(), &Profile::selfAvatarChanged, [&, bus](const QPixmap& pixmap) {
        emit updateAvatar(pixmap);
        emit bus->avatarChanged(pixmap);
    });

    connect(bus, &ok::Bus::getAvatar, [&, bus]() { bus->avatarChanged(profile->loadAvatar()); });

    connect(profile.get(), &Profile::coreChanged,
            [&, bus](Core& core) { emit bus->coreChanged(&core); });

    profile->startCore();

    emit ok::Application::Instance() -> bus()->profileChanged(profile.get());

#ifdef Q_OS_MAC
    // TODO: still needed?
    globalMenuBar = new QMenuBar(0);
    dockMenu = new QMenu(globalMenuBar);

    viewMenu = globalMenuBar->addMenu(QString());

    windowMenu = globalMenuBar->addMenu(QString());
    globalMenuBar->addAction(windowMenu->menuAction());

    fullscreenAction = viewMenu->addAction(QString());
    fullscreenAction->setShortcut(QKeySequence::FullScreen);
    connect(fullscreenAction, &QAction::triggered, this, &Nexus::toggleFullscreen);

    minimizeAction = windowMenu->addAction(QString());
    minimizeAction->setShortcut(Qt::CTRL + Qt::Key_M);
    connect(minimizeAction, &QAction::triggered, [this]() {
        minimizeAction->setEnabled(false);
        QApplication::focusWindow()->showMinimized();
    });

    windowMenu->addSeparator();
    frontAction = windowMenu->addAction(QString());
    connect(frontAction, &QAction::triggered, this, &Nexus::bringAllToFront);

    QAction* quitAction = new QAction(globalMenuBar);
    quitAction->setMenuRole(QAction::QuitRole);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    retranslateUi();
#endif
}

void Nexus::stop() {
    // TODO 断开IM连接
}

void Nexus::hide() {
    m_widget->hide();
}

const QString& Nexus::getName() const {
    return name;
}

void Nexus::do_logout(const QString& profileName) {
    //    Nexus::getProfile()->getSettings()->saveGlobal();
    profile->stopCore();
}

void Nexus::bootstrapWithProfile(Profile* p) {
    // kriby: This is a hack until a proper controller is written
    //
    //  Q_INIT_RESOURCE(res);
    //  Q_INIT_RESOURCE(emojione);
    //  Q_INIT_RESOURCE(smileys);
    //  Q_INIT_RESOURCE(translations_IM);
    //
    //  Settings &settings = Nexus::getProfile()->getSettings();
    //  QString locale = settings.getTranslation();
    //  qDebug() << "locale" << locale;
    //
    ////  add_definitions(-D${PROJECT_NAME}_MODULE="${PROJECT_NAME}")
    //  settings::Translator::translate(OK_IM_MODULE, locale);
    //
    //  profile = p;
    //  assert(profile);
    //
    //  if (profile) {
    //    audioControl =
    //    std::unique_ptr<IAudioControl>(Audio::makeAudio(settings));
    //    assert(audioControl != nullptr);
    //    profile->getCore()->getAv()->setAudio(*audioControl);
    //    start();
    //  }
}
//
// void Nexus::setSettings(Settings* settings) {
//    if (this->settings) {
//        QObject::disconnect(this, &Nexus::saveGlobal, this->settings, &Settings::saveGlobal);
//    }
//    this->settings = settings;
//    if (this->settings) {
//        QObject::connect(this, &Nexus::saveGlobal, this->settings, &Settings::saveGlobal);
//    }
//}

//  void Nexus::connectLoginScreen(const LoginScreen &loginScreen) {
// TODO(kriby): Move connect sequences to a controller class object instead

//    // Nexus -> LoginScreen
//    QObject::connect(this, &Nexus::profileLoaded, &loginScreen,
//    &LoginScreen::onProfileLoaded); QObject::connect(this,
//    &Nexus::profileLoadFailed, &loginScreen,
//    &LoginScreen::onProfileLoadFailed);
//    // LoginScreen -> Nexus
//    QObject::connect(&loginScreen, &LoginScreen::createNewProfile, this,
//    &Nexus::onCreateNewProfile); QObject::connect(&loginScreen,
//    &LoginScreen::loadProfile, this, &Nexus::onLoadProfile);
//    // LoginScreen -> Settings
//    QObject::connect(&loginScreen, &LoginScreen::autoLoginChanged,
//    settings, &Settings::setAutoLogin); QObject::connect(&loginScreen,
//    &LoginScreen::autoLoginChanged, settings, &Settings::saveGlobal);
//    // Settings -> LoginScreen
//    QObject::connect(settings, &Settings::autoLoginChanged, &loginScreen,
//                     &LoginScreen::onAutoLoginChanged);
//  }

void Nexus::showMainGUI() {
    assert(profile);

    m_widget->init();

    // Connections
    connect(profile.get(), &Profile::selfAvatarChanged, m_widget, &Widget::onSelfAvatarLoaded);

    connect(profile.get(), &Profile::selfAvatarChanged,
            [&](const QPixmap& pixmap) { emit updateAvatar(pixmap); });

    connect(profile.get(), &Profile::coreChanged, [&](Core& core) { emit coreChanged(core); });

    connect(profile.get(), &Profile::coreChanged, m_widget, &Widget::onCoreChanged);

    connect(profile.get(), &Profile::failedToStart, m_widget, &Widget::onFailedToStartCore,
            Qt::BlockingQueuedConnection);

    connect(profile.get(), &Profile::badProxy, m_widget, &Widget::onBadProxyCore,
            Qt::BlockingQueuedConnection);

    profile->startCore();
}

Module* Nexus::Create() {
    Nexus& inst = createInstance();
    return (Module*)(&inst);
}

/**
 * @brief Returns the singleton instance.
 */
Nexus& Nexus::getInstance() {
    assert(m_self);
    return *m_self;
}

Nexus& Nexus::createInstance() {
    if (m_self) {
        delete m_self;
        m_self = nullptr;
    }
    m_self = new Nexus();
    return *m_self;
}

void Nexus::cleanup() {
    qDebug() << __func__ << "...";

    profile->quit();

    CameraSource::destroyInstance();
    Settings::destroyInstance();

    qDebug() << __func__ << ".";
}

/**
 * @brief Get core instance.
 * @return nullptr if not started, core instance otherwise.
 */
Core* Nexus::getCore() {
    Nexus& nexus = getInstance();
    if (!nexus.profile) return nullptr;

    return nexus.profile->getCore();
}

/**
 * @brief Get current user profile.
 * @return nullptr if not started, profile otherwise.
 * @deprecated
 */
Profile* Nexus::getProfile() {
    assert(getInstance().profile);
    return getInstance().profile.get();
}

/**
 * @brief Creates a new profile and replaces the current one.
 * @param name New username
 * @param pass New password
 */
void Nexus::onCreateNewProfile(const QString& host, const QString& name, const QString& pass) {
    //    setProfile(Profile::createProfile(host, name, pass));
    //    parser = nullptr;  // only apply cmdline proxy settings once
}

/**
 * Loads an existing profile and replaces the current one.
 */
void Nexus::onLoadProfile(const QString& host, const QString& name, const QString& pass) {
    //    setProfile(Profile::loadProfile(host, name, pass));
    //    parser = nullptr;  // only apply cmdline proxy settings once
}
/**
 * Changes the loaded profile and notifies listeners.
 * @param p
 */
void Nexus::setProfile(Profile* p) {
    if (!p) {
        emit profileLoadFailed();
        // Warnings are issued during respective createNew/load calls
        return;
    } else {
        emit profileLoaded();
    }

    emit currentProfileChanged(p);
}

/**
 * @brief Get desktop GUI widget.
 * @return nullptr if not started, desktop widget otherwise.
 */
Widget* Nexus::getDesktopGUI() {
    return dynamic_cast<Widget*>(getInstance().widget());
}

void Nexus::playNotificationSound(IAudioSink::Sound sound, bool loop) {
    auto settings = Nexus::getProfile()->getSettings();
    if (!settings->getAudioOutDevEnabled()) {
        // don't try to play sounds if audio is disabled
        return;
    }

    if (audioNotification == nullptr) {
        audioNotification = std::unique_ptr<IAudioSink>(audioControl->makeSink());
        if (audioNotification == nullptr) {
            qDebug() << "Failed to allocate AudioSink";
            return;
        }
    }

    audioNotification->connectTo_finishedPlaying(this, [this]() { cleanupNotificationSound(); });
    audioNotification->playMono16Sound(sound);

    if (loop) {
        audioNotification->startLoop();
    }
}

void Nexus::cleanupNotificationSound() {
    audioNotification.reset();
}

void Nexus::incomingNotification(const QString& friendnumber) {
    qDebug() << __func__ << friendnumber;
    const auto& friendId = FriendId(friendnumber);
    m_widget->newFriendMessageAlert(friendId, {}, false);

    // loop until call answered or rejected
    playNotificationSound(IAudioSink::Sound::IncomingCall, true);
}

void Nexus::outgoingNotification() {
    // loop until call answered or rejected
    playNotificationSound(IAudioSink::Sound::OutgoingCall, true);
}

/**
 * @brief Widget::onStopNotification Stop the notification sound.
 */
void Nexus::onStopNotification() {
    audioNotification.reset();
}

void Nexus::bootstrapWithProfileName(const QString& host, const QString& profileName) {
    qDebug() << "bootstrapWithProfileName" << profileName;

    //    Profile* profile = nullptr;
    //    Settings& settings = Nexus::getProfile()->getSettings();
    //    setSettings(&settings);

    //    QString profileName = settings.getCurrentProfile();
    //    QCommandLineParser parser;

    //    if (Profile::exists(profileName)) {
    //        profile = Profile::loadProfile(host, profileName, &parser);
    //    }
    //    if (profile) {
    //        bootstrapWithProfile(profile);
    //    }
}

QWidget* Nexus::widget() {
    return m_widget->getInstance();
}

#ifdef Q_OS_MAC
void Nexus::retranslateUi() {
    viewMenu->menuAction()->setText(tr("View", "OS X Menu bar"));
    windowMenu->menuAction()->setText(tr("Window", "OS X Menu bar"));
    minimizeAction->setText(tr("Minimize", "OS X Menu bar"));
    frontAction->setText((tr("Bring All to Front", "OS X Menu bar")));
}

void Nexus::onWindowStateChanged(Qt::WindowStates state) {
    minimizeAction->setEnabled(QApplication::activeWindow() != nullptr);

    if (QApplication::activeWindow() != nullptr && sender() == QApplication::activeWindow()) {
        if (state & Qt::WindowFullScreen) minimizeAction->setEnabled(false);

        if (state & Qt::WindowFullScreen)
            fullscreenAction->setText(tr("Exit Fullscreen"));
        else
            fullscreenAction->setText(tr("Enter Fullscreen"));

        updateWindows();
    }

    updateWindowsStates();
}

void Nexus::updateWindows() {
    updateWindowsArg(nullptr);
}

void Nexus::updateWindowsArg(QWindow* closedWindow) {
    QWindowList windowList = QApplication::topLevelWindows();
    delete windowActions;
    windowActions = new QActionGroup(this);

    windowMenu->addSeparator();

    QAction* dockLast;
    if (!dockMenu->actions().isEmpty())
        dockLast = dockMenu->actions().first();
    else
        dockLast = nullptr;

    QWindow* activeWindow;

    if (QApplication::activeWindow())
        activeWindow = QApplication::activeWindow()->windowHandle();
    else
        activeWindow = nullptr;

    for (int i = 0; i < windowList.size(); ++i) {
        if (closedWindow == windowList[i]) continue;

        QAction* action = windowActions->addAction(windowList[i]->title());
        action->setCheckable(true);
        action->setChecked(windowList[i] == activeWindow);
        connect(action, &QAction::triggered, [&] { onOpenWindow(windowList[i]); });
        windowMenu->addAction(action);
        dockMenu->insertAction(dockLast, action);
    }

    if (dockLast && !dockLast->isSeparator()) dockMenu->insertSeparator(dockLast);
}

void Nexus::updateWindowsClosed() {
    updateWindowsArg(static_cast<QWidget*>(sender())->windowHandle());
}

void Nexus::updateWindowsStates() {
    bool exists = false;
    QWindowList windowList = QApplication::topLevelWindows();

    for (QWindow* window : windowList) {
        if (!(window->windowState() & Qt::WindowMinimized)) {
            exists = true;
            break;
        }
    }
    if (frontAction) {
        frontAction->setEnabled(exists);
    }
}

void Nexus::onOpenWindow(QObject* object) {
    QWindow* window = static_cast<QWindow*>(object);

    if (window->windowState() & QWindow::Minimized) window->showNormal();

    window->raise();
    window->requestActivate();
}

void Nexus::toggleFullscreen() {
    QWidget* window = QApplication::activeWindow();

    if (window->isFullScreen())
        window->showNormal();
    else
        window->showFullScreen();
}

void Nexus::bringAllToFront() {
    QWindowList windowList = QApplication::topLevelWindows();
    QWindow* focused = QApplication::focusWindow();

    for (QWindow* window : windowList) window->raise();

    focused->raise();
}
#endif
