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
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QMap>

#include "base/Page.h"
#include "OMainMenu.h"

namespace Ui {
class MainWindow;
}

namespace UI {

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void init();
  QFrame *getPage(PageMenu menu);
  QFrame *initPage(PageMenu menu);
  inline OMainMenu *menu() { return m_menu; }
  QWidget *getContainer(PageMenu menu);

protected:
  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void updateIcons();

private:
  Ui::MainWindow *ui;

  OMainMenu *m_menu;
  QMap<PageMenu, QWidget *> m_pageMap;

  std::unique_ptr<QSystemTrayIcon> icon;
  QMenu *trayMenu;
  QTimer *timer;
  QAction *actionQuit;
  QAction *actionShow;
  bool wasMaximized = false;
  //  bool autoAwayActive = false;

  //  void saveWindowGeometry();
  //  void saveSplitterGeometry();
  static inline QIcon prepareIcon(QString path, int w = 0, int h = 0);

signals:
  void toClose();
 void menuPushed(PageMenu menu, bool checked);

private slots:
  void onSwitchPage(PageMenu menu);
  void onToggleChat(bool);

  void onIconClick(QSystemTrayIcon::ActivationReason);

  void onTryCreateTrayIcon();

  void forceShow();
};


} // namespace UI

#endif // MAINWINDOW_H
