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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QBoxLayout>
#include <QMainWindow>
#include <QMap>
#include <QStackedWidget>
#include <QSystemTrayIcon>

#include "base/Page.h"
#include "lib/session/AuthSession.h"
#include "modules/module.h"

namespace Ui {
class MainWindow;
}

namespace UI {

class OMainMenu;
class OMenuWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<ok::session::AuthSession> session,
                        QWidget* parent = nullptr);
    ~MainWindow();

    static MainWindow* getInstance();

    void init();
    OMenuWidget* getMenuWindow(ok::base::PageMenu menu);
    OMenuWidget* initMenuWindow(ok::base::PageMenu menu);
    inline OMainMenu* menu() { return m_menu; }
    QWidget* getContainer(ok::base::PageMenu menu);

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void updateIcons();

private:
    std::shared_ptr<ok::session::AuthSession> session;
    std::unique_ptr<QTimer> timer;
    std::shared_ptr<::base::DelayedCallTimer> delayCaller;

    Ui::MainWindow* ui;
    OMainMenu* m_menu;
    QMap<ok::base::PageMenu, OMenuWidget*> menuWindow;

    std::unique_ptr<QSystemTrayIcon> icon;
    QMenu* trayMenu;
    QAction* actionQuit;
    QAction* actionShow;
    bool wasMaximized = false;

    //  bool autoAwayActive = false;
    //  void saveWindowGeometry();
    //  void saveSplitterGeometry();

    static inline QIcon prepareIcon(QString path, int w = 0, int h = 0);

signals:
    void toClose();

private slots:
    void onSwitchPage(ok::base::PageMenu menu, bool checked);

    void onIconClick(QSystemTrayIcon::ActivationReason);

    void onTryCreateTrayIcon();

    void forceShow();
    OMenuWidget* createChatModule(MainWindow* pWindow);
    OMenuWidget* createPlatformModule(MainWindow* pWindow);
};

}  // namespace UI

#endif  // MAINWINDOW_H
