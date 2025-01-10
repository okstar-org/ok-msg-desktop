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
#include "UI/window/WindowManager.h"
#include "UI/window/login/src/LoginWindow.h"
#include "UI/window/login/src/SettingManager.h"
#include "lib/session/profile.h"
#include "lib/storage/StorageManager.h"
#include "modules/module.h"

namespace UI {
class MainWindow;
}

namespace ok {

class IPC;
class Bus;

class Application : public QApplication {
    Q_OBJECT
public:
    Application(int& argc, char** argv);

    static Application* Instance() {
        return dynamic_cast<Application*>(qApp);
    }

    void start();
    void finish();

    inline Bus* bus() const {
        return _bus.get();
    }

    inline lib::session::AuthSession* getSession() {
        return session.get();
    }

    inline lib::session::Profile* getProfile() {
        return profile.get();
    }

    inline QWidget* getMainWidget() const {
        if (m_mainWindow) {
            return m_mainWindow.get();
        }
        return m_loginWindow.get();
    }

private:
    lib::storage::StorageManager* storageManager;
    std::shared_ptr<lib::session::AuthSession> session;
    std::unique_ptr<lib::session::Profile> profile;
    IPC* ipc;
    std::unique_ptr<Bus> _bus;

    std::unique_ptr<UI::LoginWindow> m_loginWindow;
    std::unique_ptr<UI::MainWindow> m_mainWindow;
    QMap<QString, Module*> m_moduleMap;

    void doLogout();
    /**
     * bootstrap: 打开程序首次启动为true，登出启动为false
     */
    void createLoginUI(bool bootstrap);
    void closeLoginUI();

    void startMainUI();
    void stopMainUI();

#ifdef OK_MODULE_PAINTER
    void initModulePainter();
#endif

#ifdef OK_PLUGIN
    void initPluginManager();
#endif

public slots:
    void cleanup();

    void onAvatar(const QPixmap&);

    void on_logout(const QString& profile);
    void on_exit();
};
}  // namespace ok
