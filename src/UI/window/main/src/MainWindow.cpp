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

#include "base/PageFactory.h"
#include "UI/window/config/src/ConfigWindow.h"
#include "base/logs.h"
#include "base/r.h"
#include "lib/settings/OkSettings.h"
#include "modules/im/src/model/status.h"
#include "modules/im/src/widget/style.h"

#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QSvgRenderer>
#include <QTimer>
#include <cstdlib>

namespace UI {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {

  ui->setupUi(this);

  setAttribute(Qt::WA_QuitOnClose, true);

  setWindowTitle(APPLICATION_NAME);

  m_menu = ui->menu_widget;
  connect(m_menu, SIGNAL(toggleChat(bool)), this, SLOT(onToggleChat(bool)));
  connect(m_menu, SIGNAL(onPage(PageMenu)), this, SLOT(onSwitchPage(PageMenu)));

  timer = new QTimer();
  timer->start(1000);
  connect(timer, &QTimer::timeout, this, &MainWindow::onTryCreateTrayIcon);

  int icon_size = 15;

  actionQuit = new QAction(this);
#ifndef Q_OS_OSX
  actionQuit->setMenuRole(QAction::QuitRole);
#endif

  actionQuit->setIcon(prepareIcon(
      Style::getImagePath("rejectCall/rejectCall.svg"), icon_size, icon_size));
  actionQuit->setText(tr("Exit", "Tray action menu to exit tox"));
  connect(actionQuit, &QAction::triggered, qApp, &QApplication::quit);

  actionShow = new QAction(this);
  actionShow->setText(tr("Show", "Tray action menu to show qTox window"));
  connect(actionShow, &QAction::triggered, this, &MainWindow::forceShow);
}

MainWindow::~MainWindow() {
  qDebug() << "~MainWindow";
  disconnect(m_menu);
  delete ui;
}

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

void MainWindow::showEvent(QShowEvent *event) {}

void MainWindow::closeEvent(QCloseEvent *event) {
  DEBUG_LOG(("closeEvent..."));
  auto &settings = ok::base::OkSettings::getInstance();

  if (settings.getShowSystemTray() && settings.getCloseToTray()) {
//    QWidget::closeEvent(event);
    close();
    return;
  }

  //    if (autoAwayActive) {
  //      emit statusSet(Status::Status::Online);
  //      autoAwayActive = false;
  //    }
  //    saveWindowGeometry();

//      emit toClose();

  //    saveSplitterGeometry();
  //    QWidget::closeEvent(event);
  //    qApp->quit();
}

void MainWindow::init() {}

void MainWindow::onTryCreateTrayIcon() {
  auto &settings = ok::base::OkSettings::getInstance();

  static int32_t tries = 15;
  if (!icon && tries--) {
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
      icon = std::unique_ptr<QSystemTrayIcon>(new QSystemTrayIcon);
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

      connect(icon.get(), &QSystemTrayIcon::activated, this,
              &MainWindow::onIconClick);

      if (settings.getShowSystemTray()) {
        icon->show();
        setHidden(settings.getAutostartInTray());
      } else {
        show();
      }

#ifdef Q_OS_MAC
      Nexus::getInstance().dockMenu->setAsDockMenu();
#endif
    } else if (!isVisible()) {
      show();
    }
  } else {
    disconnect(timer, &QTimer::timeout, this, &MainWindow::onTryCreateTrayIcon);
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
  // TODO 暂时不考虑状态
  //      Status::getAssetSuffix(static_cast<Status::Status>(
  //          ui->statusButton->property("status").toInt())) +
  //      (eventIcon ? "_event" : "");

  // Some builds of Qt appear to have a bug in icon loading:
  // QIcon::hasThemeIcon is sometimes unaware that the icon returned
  // from QIcon::fromTheme was a fallback icon, causing hasThemeIcon to
  // incorrectly return true.
  //
  // In qTox this leads to the tray and window icons using the static qTox logo
  // icon instead of an icon based on the current presence status.
  //
  // This workaround checks for an icon that definitely does not exist to
  // determine if hasThemeIcon can be trusted.
  //
  // On systems with the Qt bug, this workaround will always use our included
  // icons but user themes will be unable to override them.
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
    QString path =
        ":/img/taskbar/" + color + "/taskbar_" + assetSuffix + ".svg";

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

QFrame *MainWindow::initPage(PageMenu menu) {
  DEBUG_LOG(("initPage ..."))

  QFrame *w = Q_NULLPTR;

  switch (menu) {
  case PageMenu::welcome:
    //              w = new page::Welcome(this);
    break;

  case PageMenu::chat:
    // ignore
    break;

  case PageMenu::setting:
    w = new ConfigWindow(this);
    break;
  }

  DEBUG_LOG(("initPage finished"))
  if (w) {
    ui->stacked_widget->addWidget(w);
  }
  return w;
}

QFrame *MainWindow::getPage(PageMenu menu) {
  int idx = static_cast<int>(menu);
  DEBUG_LOG(("menu: %1").arg(idx));
  for (int i = 0; i < ui->stacked_widget->count(); i++) {
    QFrame *p = static_cast<QFrame *>(ui->stacked_widget->widget(i));
    if (p->objectName().compare(qsl("Page:%1").arg(static_cast<int>(menu))) ==
        0) {
      return qobject_cast<QFrame *>(p);
    }
  }
  return nullptr;
}

void MainWindow::onToggleChat(bool checked) {

  QStackedWidget *stackedWidget = ui->stacked_widget;
  if (!stackedWidget)
    return;

  //  QFrame *classingPage = getPage(PageMenu::classing);
  //  if (classingPage) {
  //    page::PageClassing *c = qobject_cast<page::PageClassing
  //    *>(classingPage);
  //        c->toggleChat(checked);
  //  }
}

void MainWindow::onSwitchPage(PageMenu menu) {
  QWidget *p = getPage(menu);
  if (!p) {
    p = initPage(menu);
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

QWidget *MainWindow::getContainer(PageMenu menu) { return ui->stacked_widget; }

} // namespace UI
