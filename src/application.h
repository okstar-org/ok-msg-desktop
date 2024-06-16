﻿/*
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

#pragma once

#include <QApplication>
#include <QObject>
#include <memory>

#include "base/timer.h"

#include "UI/core/ControllerManager.h"
#include "UI/core/SettingManager.h"
#include "UI/window/login/src/LoginWindow.h"
#include "UI/window/main/src/MainWindow.h"
#include "UI/window/WindowManager.h"

#include "modules/module.h"



namespace core {

using namespace network;

class Application : public QApplication {
  Q_OBJECT
public:
  Application(int &argc, char **argv);

  static Application *Instance();

  void start();
  void finish();

  inline SettingManager *settingManager() { return _settingManager.get(); }

  inline ControllerManager *controllerManager() {
    return _controllerManager.get();
  }

protected:
  void initIM();

private:
  QMap<QString, Module *> m_moduleMap;

  void *profile = nullptr;

  int _argc;
  char **_argv;

  std::unique_ptr<ok::session::AuthSession> _session;

  std::unique_ptr<SettingManager> _settingManager;

  std::unique_ptr<ControllerManager> _controllerManager;

  ok::session::SignInInfo m_signInInfo;

  std::unique_ptr<UI::LoginWindow> m_loginWindow;
  std::unique_ptr<UI::MainWindow> m_mainWindow;

  void loadService();
  void doLogout();
  /**
   * bootstrap: 打开程序首次启动为true，登出启动为false
   */
  void createLoginUI(bool bootstrap);
  void closeLoginUI();

  void startMainUI();
  void stopMainUI();

  Module * initModuleIM(ok::session::SignInInfo &signInInfo);

#ifdef OK_MODULE_PAINTER
  void initModulePainter();
#endif

#ifdef OK_PLUGIN
  void initPluginManager();
#endif

public slots:
  void cleanup();

  void onLoginSuccess(ok::session::SignInInfo &);

  void onMenuPushed(UI::PageMenu menu, bool checked);
  void onMenuReleased(UI::PageMenu menu, bool checked);

  void onAvatar(const QPixmap &);

  void on_logout(const QString &profile);
  void on_exit(const QString &profile);

};
} // namespace core
