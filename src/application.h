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
  void initScreenCaptor();

  void initIM();

private:
  QMap<QString, Module *> m_moduleMap;

  void *profile = nullptr;

  int _argc;
  char **_argv;

  std::unique_ptr<ok::session::AuthSession> _session;

  std::unique_ptr<SettingManager> _settingManager;

  std::unique_ptr<ControllerManager> _controllerManager;

  std::unique_ptr<base::DelayedCallTimer> _delayCaller;
  UI::WindowManager *m_windowManager;
  ok::session::SignInInfo m_signInInfo;

  std::unique_ptr<UI::LoginWindow> m_loginWindow;

  void loadService();

  void createLoginUI();
  void deleteLoginUI();
  void closeLoginUI();

  void startMainUI();
  void stopMainUI();

  void initModuleIM(ok::session::SignInInfo &signInInfo);

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
};
} // namespace core
