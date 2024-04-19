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
#include <QTranslator>

#include "UI/core/FontManager.h"
#include "UI/window/login/src/LoginWindow.h"
#include "base/OkSettings.h"
#include "base/files.h"
#include "base/logs.h"
#include "base/r.h"
#include "base/system/sys_info.h"
#include "lib/log/LogManager.h"
#include "lib/plugin/pluginmanager.h"
#include "lib/settings/translator.h"
#include "modules/im/src/nexus.h"

using namespace core;
using namespace base;

namespace core {

Application::Application(int &argc, char *argv[])
    : QApplication(argc, argv), _argc(argc), _argv(argv) {

  //Qt application settings.
  setApplicationName(APPLICATION_NAME);
  setApplicationVersion(APPLICATION_VERSION_ID);

#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  setDesktopFileName(APPLICATION_NAME);
#endif

  //Initialize log manager.
  ok::lib::LogManager::Instance();

  qDebug() << QString("argc:%1").arg(argc);
  for (int i = 0; i < argc; i++) {
    qDebug() << QString("argv:%1->%2").arg(i).arg(argv[i]);
  }

  ok::base::CpuInfo cpuInfo;
  ok::base::SysInfo::GetCpuInfo(cpuInfo);

  qDebug() << "CpuInfo  :"          //
           << cpuInfo.arch         //
           << cpuInfo.manufacturer //
           << cpuInfo.name         //
           << cpuInfo.cores        //
           << cpuInfo.processors;  //

  ok::base::OsInfo osInfo;
  ok::base::SysInfo::GetOsInfo(osInfo);
  qDebug() << "OsInfo   :"           //
           << osInfo.kernelName    //"linux"
           << osInfo.kernelVersion //"5.19.0-50-generic"
           << osInfo.name          //"ubuntu"
           << osInfo.version       //"22.04"
           << osInfo.prettyName    //"Ubuntu 22.04.2 LTS"
           << osInfo.hostName      //"root-host"
           << osInfo.uniqueId;     //"OWVjYjNmZTY0OTFmNGZiZGFhYjI0ODA2OTgwY2QxODQ="

  qDebug() <<"APPLICATION_RELEASE   :" << APPLICATION_RELEASE;
  qDebug() <<"APPLICATION_VERSION_ID:" << APPLICATION_VERSION_ID;
  qDebug() <<"APPLICATION_ID        :" << APPLICATION_ID;
  qDebug() <<"APPLICATION_NAME      :" << APPLICATION_NAME;

  auto configDir = ok::base::OkSettings::configDir();
  qDebug()<< "ConfigDir  :"<< configDir.path();
  auto cacheDir = ok::base::OkSettings::cacheDir();
  qDebug()<<"CacheDir   :"<< cacheDir.path();
  auto dataDir = ok::base::OkSettings::dataDir();
  qDebug()<<"DataDir    :"<< dataDir.path();
  auto downloadDir = ok::base::OkSettings::downloadDir();
  qDebug()<<"DownloadDir:"<< downloadDir.path();

  auto pluginDir = ok::base::OkSettings::getAppPluginPath();
  qDebug()<<"PluginDir  :"<< pluginDir.path();
  auto logDir = ok::base::OkSettings::getAppLogPath();
  qDebug()<<"LogDir     :"<< logDir.path();

  // Windows platform plugins DLL hell fix
  QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
  addLibraryPath("platforms");

  // 统一注册类型
  qRegisterMetaType<UI::PageMenu>("PageMenu");

  QString qss = ok::base::Files::readStringAll("application.qss");
  qApp->setStyleSheet(qss);

  // 字体
  FontManager fm;
  fm.loadFonts();

  // 延时器
  _delayCaller = std::make_unique<DelayedCallTimer>();

  // 设置
  _settingManager = std::make_unique<SettingManager>(this);

  connect(this, &QApplication::aboutToQuit, this, &Application::cleanup);

  _session = std::make_unique<ok::session::AuthSession>(this);

  qDebug() << "Application has be created";
}

Application *Application::Instance() {
  return qobject_cast<Application *>(qApp);
}

void Application::start() {

  // if (!session()->authenticated()) {
  this->createLoginUI();
  // } else {
  //   this->startMainUI();
  // }
  // nexus->start();
}

void Application::createLoginUI() {
  m_loginWindow = std::make_unique<UI::LoginWindow>();

  connect(m_loginWindow.get(), &UI::LoginWindow::loginResult,
        [&](ok::session::SignInInfo &signInInfo,
            ok::session::LoginResult &result) {
          if (result.status == ok::session::Status::SUCCESS) {
            onLoginSuccess(signInInfo);
            disconnect(m_loginWindow.get());
          }
        });

  connect(_session.get(), &ok::session::AuthSession::loginResult,
          [&](ok::session::SignInInfo signInInfo, ok::session::LoginResult result) {
            qDebug()<<"result:" << result.msg;
          }
  );
  m_loginWindow->show();

}

void Application::deleteLoginUI() {
  disconnect(m_loginWindow.get());
  m_loginWindow.reset();
}

void Application::closeLoginUI() {
  // 关闭login窗口
  deleteLoginUI();
}

void Application::onLoginSuccess(ok::session::SignInInfo &signInInfo) {
  qDebug() << qsl("onLoginSuccess account:%1").arg(signInInfo.account);
  m_signInInfo = signInInfo;

  // 初始化 IM 模块
  initModuleIM(signInInfo);

  // 初始化 Painter 模块
#ifdef OK_MODULE_PAINTER
  initModulePainter();
#endif

  // 初始化截屏模块
  initScreenCaptor();

  // 启动主界面
  startMainUI();

#ifdef OK_PLUGIN
  // 初始化插件平台
  initPluginManager();
#endif
  // 关闭登录界面
  closeLoginUI();
}

void Application::startMainUI() {
  this->loadService();

  m_windowManager = UI::WindowManager::Instance();

  connect(m_windowManager, &UI::WindowManager::menuPushed, this,
          &Application::onMenuPushed);
  connect(m_windowManager, &UI::WindowManager::mainClose, this,
          [&](SavedInfo savedInfo) {
            for (auto m : m_moduleMap) {
              m->onSave(savedInfo);
            }
          });

  m_windowManager->startMainUI();
}

void Application::stopMainUI() {
  m_windowManager->stopMainUI();
  delete m_windowManager;
  m_windowManager = nullptr;
}

void Application::loadService() {}

void Application::initScreenCaptor() {
  //  qDebug(("initScreenCaptor ..."));
  //  auto _screenCaptor = new OEScreenshot();
  //  m_moduleMap.insert(_screenCaptor->name(), _screenCaptor);
  //  qDebug(("initScreenCaptor finished"));
}

void Application::cleanup() {
  qDebug(("Cleanup..."));
  for (auto e : m_moduleMap) {
    e->cleanup();
  }
}

void Application::finish() {}

void Application::onMenuPushed(UI::PageMenu menu, bool checked) {
  qDebug() << QString("menu:%1 checked:%2").arg((int)menu).arg(checked);

  switch (menu) {
  case UI::PageMenu::chat: {
    Module *m = m_moduleMap.value(Nexus::Name());
    if (checked) {
      if (!m->isStarted()) {
        auto container = m_windowManager->getContainer(menu);
        m->start(m_signInInfo, container);
      }
    }
    break;
  }
  default:{
    //ignore
  }
  }
}

void Application::onMenuReleased(UI::PageMenu menu, bool checked) {}

void Application::initModuleIM(ok::session::SignInInfo &signInInfo) {
  qDebug(("IM..."));

  auto im = Nexus::Create();

  //  connect(im, &Module::createProfileFailed, //
  //          m_loginWindow.get(), &LoginWindow::onProfileLoadFailed);

  //  Module &nexus = Nexus::getInstance();
  //  nexus.init(static_cast<Profile *>(profile));

  connect(static_cast<Nexus *>(im), &Nexus::updateAvatar, this,
          &Application::onAvatar);

  m_moduleMap.insert(im->name(), im);
}

#ifdef OK_PLUGIN
void Application::initPluginManager() {
  ok::plugin::PluginManager *pm = ok::plugin::PluginManager::instance();
  QStringList plugins = pm->availablePlugins();
  for (const QString &plugin : plugins) {
    qDebug() << "load plugin:" << plugin;
  }
}
#endif

void Application::onAvatar(const QPixmap &pixmap) {
  auto menu = m_windowManager->getMainMenu();
  if (!menu)
    return;

  menu->setAvatar(pixmap);
}

#ifdef OK_MODULE_PAINTER
void Application::initModulePainter() {
  auto p = Painter::Create();
  qDebug() << "painter:" << p;
}
#endif

} // namespace core
