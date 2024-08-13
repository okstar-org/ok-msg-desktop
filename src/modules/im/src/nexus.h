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

#include "modules/module.h"

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

class Nexus : public QObject, public Module {
    Q_OBJECT
public:
    /**
     * Module
     */
    static QString Name();
    static Module* Create();

    void showMainGUI();
    void setSettings(Settings* settings);
    void setParser(QCommandLineParser* parser);

    static Nexus& getInstance();
    static Core* getCore();
    static Profile* getProfile();
    static Widget* getDesktopGUI();

    virtual void destroy() override;

    QString name() override;
    QWidget* widget() override;
    void init(Profile*) override;
    void start(std::shared_ptr<ok::session::AuthSession> session) override;
    bool isStarted() override { return stared; }
    void hide() override;
    void onSave(SavedInfo&) override;
    void cleanup() override;

    IAudioControl* audio() const { return audioControl.get(); }

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
    void updateAvatar(const QPixmap& pixmap);
    void createProfileFailed(QString msg);
    void destroyProfile(const QString& profile);
    void exit(const QString& profile);

public slots:
    void onCreateNewProfile(const QString& host, const QString& name, const QString& pass);
    void onLoadProfile(const QString& host, const QString& name, const QString& pass);
    void bootstrapWithProfile(Profile* p);
    void bootstrapWithProfileName(const QString& host, const QString& p);
    int showLogin(const QString& profileName = QString());
    void do_logout(const QString& profile);

private:
    explicit Nexus(QObject* parent = nullptr);
    ~Nexus();
    //  void connectLoginScreen(const LoginScreen &loginScreen);
    void setProfile(Profile* p);

private:
    bool stared;

    Profile* profile;

    Settings* settings;
    QPointer<Widget> m_widget;  // 某些异常情况下widget会被提前释放
    std::unique_ptr<IAudioControl> audioControl;
    QCommandLineParser* parser = nullptr;
};

#endif  // NEXUS_H
