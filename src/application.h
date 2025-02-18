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

#include "UI/login/src/LoginWindow.h"
#include "UI/main/src/MainWindow.h"
#include "lib/session/profile.h"
#include "lib/storage/StorageManager.h"
#include "lib/audio/iaudiocontrol.h"
#include "lib/audio/iaudiosink.h"
#include "lib/audio/player.h"
#include "Bus.h"
#include "ipc.h"

/**
 * @brief WindowSize 主窗口大小(黄金分割比例 1.618)
 * @return QSize
 */
constexpr QSize WindowSize() {
    return QSize{874, 520};
}

namespace lib::audio{
class Player;
}


namespace ok {

class Application : public QApplication {
    Q_OBJECT
public:
    Application(int& argc, char** argv);

    static Application* Instance() {
        return dynamic_cast<Application*>(qApp);
    }

    void start();
    void finish();

    [[nodiscard]] Bus* bus() const;

    inline lib::session::AuthSession* getSession() {
        return session.get();
    }

    inline lib::session::Profile* getProfile() {
        return profile.get();
    }

    inline lib::storage::StorageManager* getStorageManager(){
        return storageManager;
    }

    inline QWidget* getMainWidget() const {
        if (m_mainWindow) {
            return m_mainWindow.get();
        }
        return m_loginWindow.get();
    }

    // sound
    inline lib::audio::IAudioControl* getAudioControl()const{
        return audioControl.get();
    }

    //player
    inline lib::audio::Player *getAudioPlayer() const {
        return audioPlayer;
    }

    void playNotificationSound(lib::audio::IAudioSink::Sound sound, bool loop = false);
    void onStopNotification();
    void cleanupNotificationSound();

private:
    lib::storage::StorageManager* storageManager;
    std::shared_ptr<lib::session::AuthSession> session;
    std::unique_ptr<lib::session::Profile> profile;
    IPC* ipc;
    Bus* _bus;

    std::unique_ptr<UI::LoginWindow> m_loginWindow;
    std::unique_ptr<UI::MainWindow> m_mainWindow;

    //Audio controller
    std::unique_ptr<lib::audio::IAudioControl> audioControl;
    std::unique_ptr<lib::audio::IAudioSink> audioNotification;

    lib::audio::Player* audioPlayer;

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
    void on_logout(const QString& profile);
    void on_exit();
};
}  // namespace ok
