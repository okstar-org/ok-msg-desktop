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
#include "WindowManager.h"

#include <memory>

#include <QApplication>
#include <QObject>

#include "UI/window/main/src/MainWindow.h"
#include "base/Page.h"

namespace UI {

WindowManager::WindowManager(QObject* parent) : QObject(parent) {}

WindowManager::~WindowManager() {}

WindowManager* WindowManager::Instance() {
    static WindowManager* instance = nullptr;
    if (!instance) {
        instance = new WindowManager;
    }
    return instance;
}

QFrame* WindowManager::getPage(ok::base::PageMenu menu) { return m_mainWindow->getPage(menu); }

void WindowManager::startMainUI() {
    m_mainWindow = std::make_unique<UI::MainWindow>();

    /**
     * connect menu's button events.
     */
    connect(m_mainWindow.get(), &UI::MainWindow::toClose,  //
            [&]() { emit mainClose({m_mainWindow->saveGeometry()}); });

    // 统一注册类型
    qRegisterMetaType<ok::base::PageMenu>("ok::base::PageMenu");
    connect(m_mainWindow->menu(), &OMainMenu::menuPushed,
            [&](ok::base::PageMenu menu, bool checked) { emit menuPushed(menu, checked); });
    m_mainWindow->show();
}

void WindowManager::stopMainUI() {
    m_mainWindow->close();
    m_mainWindow.reset();
}

QWidget* WindowManager::getContainer(ok::base::PageMenu menu) {  //
    return m_mainWindow->getContainer(menu);
}

OMainMenu* WindowManager::getMainMenu() {  //
    return m_mainWindow->menu();
}

}  // namespace UI
