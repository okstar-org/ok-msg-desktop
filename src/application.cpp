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

#include "application.h"

#include <memory>

#include <QApplication>
#include <QCoreApplication>
#include <QString>

#include <QHBoxLayout>
#include <QMenu>
#include "Bus.h"
#include "UI/login/src/LoginWidget.h"
#include "UI/login/src/LoginWindow.h"
#include "UI/main/src/MainWindow.h"
#include "UI/main/src/OMainMenu.h"
#include "base/files.h"
#include "base/r.h"
#include "base/system/sys_info.h"
#include "ipc.h"
#include "lib/plugin/pluginmanager.h"
#include "lib/storage/StorageManager.h"
#include "lib/storage/settings/OkSettings.h"
#include "lib/audio/audio.h"

namespace ok {

/**
 * 应用程序
 * @param argc
 * @param argv
 */
Application::Application(int& argc, char* argv[]) : QApplication(argc, argv) {
    // Qt application settings.
    setApplicationName(APPLICATION_NAME);
    setApplicationVersion(APPLICATION_VERSION_ID);
    setOrganizationName(ORGANIZATION_NAME);
    setOrganizationDomain(ORGANIZATION_DOMAIN);

#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    setDesktopFileName(APPLICATION_NAME);
#endif

    connect(qApp, &QApplication::aboutToQuit, this, &Application::on_exit);

    // Initialize log manager.

    storageManager = new lib::storage::StorageManager("", this);
    auto dir = storageManager->getDir().path();

    qDebug() << "StorageManager root:" << dir;
    qDebug() << "QT_VERSION:" << QT_VERSION_STR;
    qDebug() << "APPLICATION_NAME:" << APPLICATION_NAME;
    qDebug() << "APPLICATION_VERSION_ID:" << APPLICATION_VERSION_ID;
    qDebug() << "ORGANIZATION_NAME:" << ORGANIZATION_NAME;
    qDebug() << "ORGANIZATION_DOMAIN:" << ORGANIZATION_DOMAIN;

    qDebug() << QString("argc:%1").arg(argc);
    for (int i = 0; i < argc; i++) {
        qDebug() << QString("argv:%1->%2").arg(i).arg(argv[i]);
    }

    ok::base::CpuInfo cpuInfo;
    ok::base::SysInfo::GetCpuInfo(cpuInfo);
    // 打印CPU信息
    qDebug() << __func__ << "CpuInfo: "  //
             << cpuInfo.arch             //
             << cpuInfo.manufacturer     //
             << cpuInfo.name             //
             << cpuInfo.cores            //
             << cpuInfo.processors;      //

    ok::base::OsInfo osInfo;
    ok::base::SysInfo::GetOsInfo(osInfo);
    qDebug() << "OsInfo   :"          //
             << osInfo.kernelName     //"linux"
             << osInfo.kernelVersion  //"5.19.0-50-generic"
             << osInfo.name           //"ubuntu"
             << osInfo.version        //"22.04"
             << osInfo.prettyName     //"Ubuntu 22.04.2 LTS"
             << osInfo.hostName       //"root-host"
             << osInfo.uniqueId;      //"OWVjYjNmZTY0OTFmNGZiZGFhYjI0ODA2OTgwY2QxODQ="

    qDebug() << "APPLICATION_RELEASE   :" << APPLICATION_RELEASE;
    qDebug() << "APPLICATION_VERSION_ID:" << APPLICATION_VERSION_ID;
    qDebug() << "APPLICATION_ID        :" << APPLICATION_ID;
    qDebug() << "APPLICATION_NAME      :" << APPLICATION_NAME;

    auto configDir = lib::settings::OkSettings::configDir();
    qDebug() << "ConfigDir  :" << configDir.path();
    auto cacheDir = lib::settings::OkSettings::cacheDir();
    qDebug() << "CacheDir   :" << cacheDir.path();
    auto dataDir = lib::settings::OkSettings::dataDir();
    qDebug() << "DataDir    :" << dataDir.path();
    auto downloadDir = lib::settings::OkSettings::downloadDir();
    qDebug() << "DownloadDir:" << downloadDir.path();

    auto pluginDir = lib::settings::OkSettings::getAppPluginPath();
    qDebug() << "PluginDir  :" << pluginDir.path();
    auto logDir = lib::settings::OkSettings::getAppLogPath();
    qDebug() << "LogDir     :" << logDir.path();

    // Windows platform plugins DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    addLibraryPath("platforms");

    // 初始化IPC
    ipc = new IPC(0, this);
    _bus = new Bus(this);

    //获取设置
    auto& s = lib::settings::OkSettings::getInstance();
    //选择的语言
    QString locale = s.getTranslation();
    qDebug() << "locale" << locale;

    // 初始化音频
    audioControl = std::unique_ptr<lib::audio::IAudioControl>(lib::audio::Audio::makeAudio(s));
    audioPlayer = new lib::audio::Player(this);
    connect(audioPlayer,  &lib::audio::Player::stateChanged, this,
            [](QString file , lib::audio::PlayState state){
                qDebug() << "play:" << file <<" state:" << lib::audio::PlayStateAsStr(state);
            });

    // 样式
    setStyleSheet(ok::base::Files::readStringAll(":/resources/style/application.css"));

    qDebug() << "Application has be created";
}

void Application::start() {
    if (!ipc->isAttached()) {
        qWarning() << "Unable to run the app.";
        return;
    }

    if (ipc->isAlive()) {
        // 已存在活跃客户端，退出本次程序。
        qFatal("Another app instance is already running, you can not start multiple "
               "application on one device.");
    }

    // 启动登录界面
    this->createLoginUI(true);
}

/**
 * 创建登录窗口
 * @param bootstrap
 */
void Application::createLoginUI(bool bootstrap) {
    qDebug() << __func__ << "bootstrap:" << bootstrap;
    session = std::make_shared<lib::session::AuthSession>();
    connect(session.get(), &lib::session::AuthSession::tokenSet,  //
            [&]() {
                profile = std::make_unique<lib::session::Profile>(
                        storageManager->create(session->getSignInInfo().username), session.get());
                startMainUI();
            });

    m_loginWindow = std::make_unique<UI::LoginWindow>(session, bootstrap);
    m_loginWindow->show();
}

/**
 *  关闭login窗口
 */
void Application::closeLoginUI() {
    qDebug() << __func__ << "...";
    if (!m_loginWindow) {
        return;
    }
    m_loginWindow->close();
    m_loginWindow.reset();
    qDebug() << __func__ << "closed";
}

/**
 * 启动主窗口
 */
void Application::startMainUI() {
    qDebug() << __func__ << "...";

    // Check the access token.
    assert(session);
    assert(!session->getToken().accessToken.isEmpty());

    if (m_mainWindow) {
        qWarning() << "Main window was show.";
        return;
    }

    // Create main window
    m_mainWindow = std::make_unique<UI::MainWindow>(session);
    m_mainWindow->show();

    // 关闭登录窗口
    closeLoginUI();

#ifdef OK_PLUGIN
    // 初始化插件平台
    initPluginManager();
#endif

}

void Application::stopMainUI() {
    m_mainWindow.reset();
}

void Application::cleanup() {

}

void Application::finish() {}

Bus* Application::bus() const {
    assert(_bus);
    return _bus;
}

#ifdef OK_PLUGIN
void Application::initPluginManager() {
    ok::plugin::PluginManager* pm = ok::plugin::PluginManager::instance();
    QStringList plugins = pm->availablePlugins();
    for (const QString& plugin : plugins) {
        qDebug() << "load plugin:" << plugin;
    }
}
#endif

void Application::on_logout(const QString& profile) {
    qDebug() << __func__ << profile;
    stopMainUI();
    QThread::currentThread()->msleep(100);
    createLoginUI(false);
}

void Application::on_exit() {
    qDebug() << __func__;
    // auto &s = lib::settings::OkSettings::getInstance();
    // s.saveGlobal();
}

void Application::playNotificationSound(lib::audio::IAudioSink::Sound sound, bool loop) {
    auto settings = &lib::settings::OkSettings::getInstance();
    if (!settings->getAudioOutDevEnabled()) {
        // don't try to play sounds if audio is disabled
        return;
    }

    if (audioNotification == nullptr) {
        audioControl->setOutputVolumeStep(settings->getOutVolume());
        audioNotification = std::unique_ptr<lib::audio::IAudioSink>(audioControl->makeSink());
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

void Application::cleanupNotificationSound() {
    audioNotification.reset();
}

/**
 * @brief Widget::onStopNotification Stop the notification sound.
 */
void Application::onStopNotification() {
    audioNotification.reset();
}

}  // namespace ok
