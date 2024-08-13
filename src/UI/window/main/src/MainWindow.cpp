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
#include "ui_MainWindow.h"

#include "UI/window/config/src/ConfigWindow.h"
#include "application.h"
#include "base/OkSettings.h"
#include "base/PageFactory.h"
#include "base/logs.h"
#include "modules/im/src/model/status.h"
#include "modules/platform/src/Platform.h"
#include "src/lib/settings/style.h"

#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QSvgRenderer>
#include <QTimer>
#include <cstdlib>
#include <memory>

#include <modules/im/src/nexus.h>
#include <modules/platform/src/Platform.h>

namespace UI {

static MainWindow* instance = nullptr;

MainWindow::MainWindow(std::shared_ptr<ok::session::AuthSession> session, QWidget* parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
        , delayCaller(std::make_unique<base::DelayedCallTimer>())
        , session{session} {
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
    setAttribute(Qt::WA_QuitOnClose, true);
    // 黄金分割比例 874/520 = 1.618
    setMinimumSize(QSize(874, 520));

    m_menu = ui->menu_widget;

    timer = std::make_unique<QTimer>();
    timer->start(1000);
    connect(timer.get(), &QTimer::timeout, this, &MainWindow::onTryCreateTrayIcon);

    int icon_size = 15;

    actionQuit = new QAction(this);
#ifndef Q_OS_OSX
    actionQuit->setMenuRole(QAction::QuitRole);
#endif

    actionQuit->setIcon(
            prepareIcon(Style::getImagePath("rejectCall/rejectCall.svg"), icon_size, icon_size));
    actionQuit->setText(tr("Exit", "Tray action menu to exit tox"));
    connect(actionQuit, &QAction::triggered, qApp, &QApplication::quit);

    actionShow = new QAction(this);
    actionShow->setText(tr("Show", "Tray action menu to show window"));
    connect(actionShow, &QAction::triggered, this, &MainWindow::forceShow);

    // connect to menu
    connect(m_menu, &OMainMenu::menuPushed, this, &MainWindow::onSwitchPage);

    instance = this;
}

MainWindow::~MainWindow() {
    qDebug() << __func__;
    disconnect(m_menu);
    delete ui;
}

MainWindow* MainWindow::getInstance() { return instance; }

// Preparing needed to set correct size of icons for GTK tray backend
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

// void MainWindow::saveWindowGeometry() {
//   settings.setWindowGeometry(saveGeometry());
//   //  settings.setWindowState(saveState());
// }

// void MainWindow::saveSplitterGeometry() {
//   if (!settings.getSeparateWindow()) {
//     settings.setSplitterState(ui->mainSplitter->saveState());
//   }
// }

void MainWindow::showEvent(QShowEvent* event) {}

void MainWindow::closeEvent(QCloseEvent* event) {
    qDebug() << __func__ << "closeEvent...";
    //  auto &settings = ok::base::OkSettings::getInstance();

    //  if (settings.getShowSystemTray() && settings.getCloseToTray()) {
    //    QWidget::closeEvent(event);
    //    close();
    //    return;
    //  }

    //    if (autoAwayActive) {
    //      emit statusSet(Status::Status::Online);
    //      autoAwayActive = false;
    //    }
    //    saveWindowGeometry();

    //      emit toClose();

    //    saveSplitterGeometry();
    //    QWidget::closeEvent(event);
    //    qApp->quit();

    //  emit Nexus::getInstance().exit("");
}

void MainWindow::init() {}

void MainWindow::onTryCreateTrayIcon() {
    auto& settings = ok::base::OkSettings::getInstance();

    static int32_t tries = 15;
    if (!icon && tries--) {
        if (QSystemTrayIcon::isSystemTrayAvailable()) {
            icon = std::make_unique<QSystemTrayIcon>();
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
            icon->setContextMenu(trayMenu);

            connect(icon.get(), &QSystemTrayIcon::activated, this, &MainWindow::onIconClick);

            if (settings.getShowSystemTray()) {
                icon->show();
                setHidden(settings.getAutostartInTray());
            } else {
                show();
            }

#ifdef Q_OS_MAC
            // Nexus::getInstance().dockMenu->setAsDockMenu();
#endif
        } else if (!isVisible()) {
            show();
        }
    } else {
        disconnect(timer.get(), &QTimer::timeout, this, &MainWindow::onTryCreateTrayIcon);
        if (!icon) {
            qWarning() << "No system tray detected!";
            show();
        }
    }
}

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
    // Workaround to force minimized window to be restored
    show();
    activateWindow();
}

void MainWindow::updateIcons() {
    if (!icon) {
        return;
    }

    const QString assetSuffix = "online";
    static bool checkedHasThemeIcon = false;
    static bool hasThemeIconBug = false;

    if (!checkedHasThemeIcon) {
        hasThemeIconBug = QIcon::hasThemeIcon("qtox-asjkdfhawjkeghdfjgh");
        checkedHasThemeIcon = true;

        if (hasThemeIconBug) {
            qDebug() << "Detected buggy QIcon::hasThemeIcon. Icon overrides from "
                        "theme will be ignored.";
        }
    }

    QIcon ico;
    if (!hasThemeIconBug && QIcon::hasThemeIcon("qtox-" + assetSuffix)) {
        ico = QIcon::fromTheme("qtox-" + assetSuffix);
    } else {
        //    QString color = Settings.getLightTrayIcon() ? "light" : "dark";
        QString color = "light";
        QString path = ":/img/taskbar/" + color + "/taskbar_" + assetSuffix + ".svg";

        QSvgRenderer renderer(path);

        // Prepare a QImage with desired characteritisc
        QImage image = QImage(250, 250, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        renderer.render(&painter);
        ico = QIcon(QPixmap::fromImage(image));
    }

    setWindowIcon(ico);
    if (icon) {
        icon->setIcon(ico);
    }
}

OMenuWidget* MainWindow::initMenuWindow(ok::base::PageMenu menu) {
    OMenuWidget* w = nullptr;
    switch (menu) {
        case ok::base::PageMenu::chat:
            w = createChatModule(this);
            break;
        case ok::base::PageMenu::platform:
            w = createPlatformModule(this);
            break;
        case ok::base::PageMenu::setting:
            w = new ConfigWindow(this);
            break;
    }
    if (w) {
        auto m = w->getModule();
        if (m) {
            delayCaller->call(1, [=, this]() {
                assert(w);
                assert(session);
                qDebug() << "Start module:" << m->name();
                m->start(session);
            });
        }

        menuWindow.insert(menu, w);
        ui->stacked_widget->addWidget(w);
    }
    return w;
}

OMenuWidget* MainWindow::getMenuWindow(ok::base::PageMenu menu) { return menuWindow.value(menu); }

void MainWindow::onSwitchPage(ok::base::PageMenu menu, bool checked) {
    OMenuWidget* p = getMenuWindow(menu);
    if (!p) {
        p = initMenuWindow(menu);
    }

    if (!p) {
        return;
    }

    ui->stacked_widget->setCurrentWidget(p);
}
//
// void MainWindow::on_btnMin_clicked() { showMinimized(); }
// void MainWindow::on_btnMax_clicked() {
//  if (isMaximized())
//    showNormal();
//  else
//    showMaximized();
//}
//
// void MainWindow::on_btnClose_clicked() {
//  close();
//  exit(0);
//}
//
// void MainWindow::on_bthFull_clicked() {
//  if (isFullScreen())
//    showNormal();
//  else
//    showFullScreen();
//}

// void MainWindow::onReceiveRooms(std::list<ok::backendRoomInfo> &roomInfos) {
//   RoomSelectDialog *dialog = new RoomSelectDialog(this);
//   dialog->setRooms(roomInfos);
//  TODO
//    auto _im = lib::IM::Messenger::getInstance()->im();
//    connect(dialog, SIGNAL(confirm()), this, SLOT(onRoomSelected()));

//  int retval = dialog->exec();
//  DEBUG_LOG_S(L_INFO) << "dialog=>" << retval;
// disconnect(dialog, SIGNAL(confirm()), this, SLOT(onRoomSelected()));
//}

// void MainWindow::onRoomSelected() {
//   auto messenger = lib::IM::Messenger::getInstance();
//   TODO
//   auto _im = messenger->im();
//   _im->doJoinRoom();
//   messenger->initRoom();
//}

QWidget* MainWindow::getContainer(ok::base::PageMenu menu) { return ui->stacked_widget; }

OMenuWidget* MainWindow::createChatModule(MainWindow* pWindow) {
    qDebug() << "Creating m:" << Nexus::Name();
    auto m = Nexus::Create();
    auto nexus = static_cast<Nexus*>(m);

    connect(nexus, &Nexus::updateAvatar,  //
            ok::Application::Instance(), &ok::Application::onAvatar);
    connect(nexus, &Nexus::destroyProfile,  //
            ok::Application::Instance(), &ok::Application::on_logout);
    connect(nexus, &Nexus::exit,  //
            ok::Application::Instance(), &ok::Application::on_exit);

    auto w = new OMenuWidget(this);
    w->setModule(m);
    w->setLayout(new QGridLayout());
    w->layout()->setContentsMargins(0, 0, 0, 0);
    w->layout()->addWidget(m->widget());
    return w;
}

OMenuWidget* MainWindow::createPlatformModule(MainWindow* pWindow) {
    auto m = new ok::platform::Platform();

    auto w = new OMenuWidget(this);
    w->setModule(m);
    w->setLayout(new QGridLayout());
    w->layout()->setContentsMargins(0, 0, 0, 0);
    w->layout()->addWidget(m->widget());

    return w;
}

}  // namespace UI
