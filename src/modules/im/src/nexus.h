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

#ifndef NEXUS_H
#define NEXUS_H

#include <QObject>
#include <QPointer>

#include "lib/audio/iaudiocontrol.h"
#include "lib/audio/iaudiosink.h"
#include "modules/module.h"
#include "src/base/compatiblerecursivemutex.h"

class Widget;
class Profile;
class Settings;
class Core;
class QCommandLineParser;

#ifdef Q_OS_MAC
class QMenuBar;
class QMenu;
class QAction;
class QWindow;
class QActionGroup;
class QSignalMapper;
#endif

/**
 * 聊天模块关系组织者，模块实现。
 */
class Nexus : public QObject, public Module {
    Q_OBJECT
public:
    explicit Nexus(QObject* parent = nullptr);
    ~Nexus() override;

    static Module* Create();

    static Nexus* getInstance();
    static Core* getCore();
    static Profile* getProfile();
    static Widget* getDesktopGUI();

    void showMainGUI();
    [[nodiscard]] IAudioControl* audio() const {
        return audioControl.get();
    }

    void playNotificationSound(IAudioSink::Sound sound, bool loop = false);
    void incomingNotification(const QString& friendId);
    void onStopNotification();
    void outgoingNotification();
    void cleanupNotificationSound();

protected:
    const QString& getName() const override;
    QWidget* widget() override;
    void init(Profile*) override;
    void start(std::shared_ptr<lib::session::AuthSession> session) override;
    void stop() override;
    bool isStarted() override {
        return stared;
    }
    void hide() override;
    void onSave(SavedInfo&) override;
    void cleanup() override;

private:

    void setProfile(Profile* p);

private:
    QString name;
    bool stared;
    std::unique_ptr<Profile> profile;

    // 某些异常情况下widget会被提前释放
    QPointer<Widget> m_widget;

    std::unique_ptr<IAudioControl> audioControl;
    std::unique_ptr<IAudioSink> audioNotification;

    CompatibleRecursiveMutex mutex;

#ifdef Q_OS_MAC
public:
    QMenuBar* globalMenuBar;
    QMenu* viewMenu;
    QMenu* windowMenu;
    QAction* minimizeAction;
    QAction* fullscreenAction;
    QAction* frontAction;
    QMenu* dockMenu;

public slots:
    void retranslateUi();
    void onWindowStateChanged(Qt::WindowStates state);
    void updateWindows();
    void updateWindowsClosed();
    void updateWindowsStates();
    void onOpenWindow(QObject* object);
    void toggleFullscreen();
    void bringAllToFront();

private:
    void updateWindowsArg(QWindow* closedWindow);

    QActionGroup* windowActions = nullptr;
#endif

signals:
    void currentProfileChanged(Profile* Profile);
    void profileLoaded();
    void profileLoadFailed();
    void coreChanged(Core&);
    void saveGlobal();
    void createProfileFailed(QString msg);
    void destroyProfile(const QString& profile);
    void exit(const QString& profile);

public slots:
    void do_logout(const QString& profile);
};

#endif  // NEXUS_H
