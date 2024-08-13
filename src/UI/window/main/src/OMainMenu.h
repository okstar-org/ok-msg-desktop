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
#ifndef OMAINMENU_H
#define OMAINMENU_H

#include <QFrame>
#include <memory>

#include "base/Page.h"
#include "base/resources.h"
#include "base/timer.h"

OK_RESOURCE_LOADER(UIWindowMain)

namespace Ui {
class OMainMenu;
}

namespace UI {

class OMainMenu : public QFrame {
    Q_OBJECT

public:
    explicit OMainMenu(QWidget* parent = nullptr);
    ~OMainMenu() override;

    void onSetting();

protected:
    virtual void showEvent(QShowEvent* e) override;

private:
    OK_RESOURCE_PTR(UIWindowMain);

    Ui::OMainMenu* ui;

    int _showTimes;
    // delayCaller
    std::shared_ptr<base::DelayedCallTimer> delayCaller_;

    void updateUI();

signals:
    void menuPushed(ok::base::PageMenu menu, bool checked);

private slots:
    void on_chatBtn_clicked(bool checked);
    void on_settingBtn_clicked(bool checked);
    void on_platformBtn_clicked(bool checked);
public slots:
    void setAvatar(const QPixmap&);
};

}  // namespace UI

#endif  // OMAINMENU_H
