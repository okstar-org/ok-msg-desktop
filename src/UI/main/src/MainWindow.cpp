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
#include "MainWindow.h"
#include "Bus.h"
#include "ui_MainWindow.h"

#include "application.h"
#include "lib/storage/settings/OkSettings.h"
#include "lib/storage/settings/style.h"
#include "lib/storage/settings/translator.h"
#include "modules/im/src/model/status.h"

#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QSvgRenderer>
#include <QSystemTrayIcon>
#include <QTimer>
#include <cstdlib>
#include <memory>

#include <modules/document/src/Document.h>
#include <modules/im/src/nexus.h>
#include <modules/meet/src/Meet.h>
#include <modules/platform/src/Platform.h>

#include "OMainMenu.h"

namespace UI {

static MainWindow* instance = nullptr;

MainWindow::MainWindow(std::shared_ptr<lib::session::AuthSession> session, QWidget* parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
        , delayCaller(std::make_unique<base::DelayedCallTimer>())
        , session{session}
        , sysTrayIcon(nullptr) {
    qDebug() << __func__;

    ui->setupUi(this);
    //  setStyleSheet("QMainWindow{background-color: white;}");
    //  setAutoFillBackground(false);

    setAutoFillBackground(true);
    // 创建一个QPalette对象
    QPalette palette = this->palette();
    // 设置背景颜色为浅蓝色
    palette.setColor(QPalette::Window, Qt::white);
    // 应用新的调色板
    this->setPalette(palette);

    setWindowTitle(APPLICATION_NAME);
    setMinimumSize(WindowSize());
    setAttribute(Qt::WA_QuitOnClose, true);

    // menu
    m_menu = ui->menu_widget;
    connect(m_menu, &OMainMenu::menuPushed, this, &MainWindow::onSwitchPage);

    // settings
    auto& okSettings = lib::settings::OkSettings::getInstance();
    connect(&okSettings, &lib::settings::OkSettings::showSystemTrayChanged, this,
            &MainWindow::onSetShowSystemTray);

    auto wg = okSettings.getWindowGeometry();
    if (!wg.isEmpty()) {
        restoreGeometry(wg);
    }

    int icon_size = 15;

    actionQuit = new QAction(this);
#ifndef Q_OS_OSX
    actionQuit->setMenuRole(QAction::QuitRole);
#endif

    actionQuit->setIcon(prepareIcon(lib::settings::Style::getImagePath("rejectCall/rejectCall.svg"),
                                    icon_size, icon_size));
    actionQuit->setText(tr("Exit", "Tray action menu to exit tox"));

    connect(actionQuit, &QAction::triggered, [&]() {
        saveWindowGeometry();
        qApp->quit();
    });

    actionShow = new QAction(this);
    actionShow->setText(tr("Show", "Tray action menu to show window"));
    connect(actionShow, &QAction::triggered, this, &MainWindow::forceShow);

    instance = this;

    // 启动桌面图标
    createSystemTrayIcon();


    auto locale = lib::settings::OkSettings().getTranslation();
    settings::Translator::translate(OK_UIMainWindow_MODULE, locale);

    auto a = ok::Application::Instance();
    connect(a->bus(), &ok::Bus::languageChanged,this, [&](const QString& locale0) {
        retranslateUi();
    });

    qDebug() << __func__ << " has be created.";
}

MainWindow::~MainWindow() {
    qDebug() << __func__;
    stop();

    for (auto k : menuMap.keys()) {
        auto menu = menuMap.value(k);
        delete menu;
    }
    menuMap.clear();

    disconnect(m_menu, &OMainMenu::menuPushed, this, &MainWindow::onSwitchPage);
    delete ui;
}

MainWindow* MainWindow::getInstance() {
    return instance;
}

void MainWindow::stop() {
    qDebug() << __func__;
    for (auto k : menuMap.keys()) {
        auto menu = menuMap.value(k);
        menu->stop();
    }
}

inline QIcon MainWindow::prepareIcon(QString path, int w, int h) {
#ifdef Q_OS_LINUX

    QString desktop = getenv("XDG_CURRENT_DESKTOP");
    if (desktop.isEmpty()) {
        desktop = getenv("DESKTOP_SESSION");
    }

    desktop = desktop.toLower();
    if (desktop == "xfce" || desktop.contains("gnome") || desktop == "mate" ||
        desktop == "x-cinnamon") {
        if (w > 0 && h > 0) {
            QSvgRenderer renderer(path);

            QPixmap pm(w, h);
            pm.fill(Qt::transparent);
            QPainter painter(&pm);
            renderer.render(&painter, pm.rect());

            return QIcon(pm);
        }
    }
#endif
    return QIcon(path);
}

void MainWindow::saveWindowGeometry() {
    auto& s = lib::settings::OkSettings::getInstance();
    s.setWindowGeometry(saveGeometry());
    s.setWindowState(saveState());
}

void MainWindow::showEvent(QShowEvent* event) {}

void MainWindow::closeEvent(QCloseEvent* event) {
    qDebug() << __func__ << "closeEvent...";
    //  auto &settings = lib::settings::OkSettings::getInstance();

    //  if (settings.getShowSystemTray() && settings.getCloseToTray()) {
    //    QWidget::closeEvent(event);
    //    close();
    //    return;
    //  }

    //  if (autoAwayActive) {
    //      emit statusSet(Status::Online);
    //      autoAwayActive = false;
    //  }
    saveWindowGeometry();

    //  emit toClose();

    //    saveSplitterGeometry();
    //    QWidget::closeEvent(event);
    //    qApp->quit();

    //  emit Nexus::getInstance()->exit("");
}

void MainWindow::init() {}

void MainWindow::onSetShowSystemTray(bool newValue) {
    if (sysTrayIcon) {
        sysTrayIcon->setVisible(newValue);
    }
}

void MainWindow::createSystemTrayIcon() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System does not support system tray!";
        return;
    }

    auto& settings = lib::settings::OkSettings::getInstance();
    if (!settings.getShowSystemTray()) return;

    sysTrayIcon = new QSystemTrayIcon(this);
    updateIcons();

    trayMenu = new QMenu(this);

    // adding activate to the top, avoids accidentally clicking quit
    trayMenu->addAction(actionShow);
    //      trayMenu->addSeparator();
    //      trayMenu->addAction(statusOnline);
    //      trayMenu->addAction(statusAway);
    //      trayMenu->addAction(statusBusy);
    //      trayMenu->addSeparator();
    //      trayMenu->addAction(actionLogout);
    trayMenu->addAction(actionQuit);

    sysTrayIcon->setContextMenu(trayMenu);
    connect(sysTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onIconClick);

    if (settings.getShowSystemTray()) {
        sysTrayIcon->show();
        setHidden(settings.getAutostartInTray());
    } else {
        show();
    }
}

/**
 * 处理任务栏图标事件
 * @param reason
 */
void MainWindow::onIconClick(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (isHidden() || isMinimized()) {
            if (wasMaximized) {
                showMaximized();
            } else {
                showNormal();
            }

            activateWindow();
        } else if (!isActiveWindow()) {
            activateWindow();
        } else {
            wasMaximized = isMaximized();
            hide();
        }
    } else if (reason == QSystemTrayIcon::Unknown) {
        if (isHidden()) {
            forceShow();
        }
    }
}

void MainWindow::forceShow() {
    hide();
    show();
    activateWindow();
}

void MainWindow::updateIcons() {
    if (!sysTrayIcon) {
        return;
    }

    const QString assetSuffix = "online";
    static bool checkedHasThemeIcon = false;
    static bool hasThemeIconBug = false;

    //    QString color = Settings.getLightTrayIcon() ? "light" : "dark";
    QString color = "light";
    QString path = ":/img/taskbar/" + color + "/taskbar_" + assetSuffix + ".svg";

    QSvgRenderer renderer(path);

    // Prepare a QImage with desired characteritisc
    QImage image = QImage(250, 250, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter);
    QIcon ico = QIcon(QPixmap::fromImage(image));

    setWindowIcon(ico);
    if (sysTrayIcon) {
        sysTrayIcon->setIcon(ico);
    }
}

void MainWindow::retranslateUi()
{

}

/**
 * 初始化菜单
 * @param menu
 * @return
 */
OMenuWidget* MainWindow::initMenuWindow(SystemMenu menu) {
    auto w = ui->menu_widget->createWidget(menu);

    assert(w);

    auto m = w->getModule();
    assert(m);
    delayCaller->call(1, [=, this]() {
        assert(session);
        qDebug() << "Start module:" << m->getName();
        m->start(session);
    });

    menuMap.insert(menu, w);
    ui->stacked_widget->addWidget(w);
    return w;
}

OMenuWidget* MainWindow::getMenuWindow(SystemMenu menu) {
    return menuMap.value(menu);
}

void MainWindow::onSwitchPage(SystemMenu menu, bool checked) {
    auto p = getMenuWindow(menu);
    if (!p) {
        p = initMenuWindow(menu);
    }

    if (!p) {
        return;
    }

    if (p != ui->stacked_widget->currentWidget()) {
        ui->stacked_widget->setCurrentWidget(p);
        if (p->getModule()) p->getModule()->activate();
    }
}

QWidget* MainWindow::getContainer(SystemMenu menu) {
    return ui->stacked_widget;
}

}  // namespace UI
