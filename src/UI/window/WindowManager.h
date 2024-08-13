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

#include <QMap>
#include <QObject>
#include <memory>

#include "UI/window/main/src/MainWindow.h"
#include "base/Page.h"
#include "modules/module.h"

namespace UI {

class WindowManager : public QObject {
    Q_OBJECT
public:
    WindowManager(QObject* parent = nullptr);
    ~WindowManager();

    static WindowManager* Instance();

    void startMainUI();
    void stopMainUI();

    void putPage(ok::base::PageMenu menu, QFrame* p);

    QFrame* getPage(ok::base::PageMenu menu);

    inline UI::MainWindow* window() { return m_mainWindow.get(); }

    QWidget* getContainer(ok::base::PageMenu menu);

    OMainMenu* getMainMenu();

private:
    std::unique_ptr<UI::MainWindow> m_mainWindow;

signals:
    void menuPushed(ok::base::PageMenu menu, bool checked);
    void mainClose(SavedInfo savedInfo);
};
}  // namespace ok::base::UI
