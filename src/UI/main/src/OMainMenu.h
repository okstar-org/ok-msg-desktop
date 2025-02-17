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
#include <QMap>
#include "base/resources.h"
#include "base/timer.h"


OK_RESOURCE_LOADER(UIMainWindow)

namespace ok::base {
class StyledIconButton;
}



namespace Ui {
class OMainMenu;
}

namespace UI {
class OMenuWidget;
class OMenuItem;
// 系统级别主菜单
enum class SystemMenu;

struct MenuItem{
    SystemMenu menu;
    QString key;
    QString toolTip;
    ok::base::Fn<OMenuWidget*()> fn;
};

/**
 * 菜单
 */
class OMainMenu : public QFrame {
    Q_OBJECT
public:
    explicit OMainMenu(QWidget* parent = nullptr);
    ~OMainMenu() override;

    void check(SystemMenu menu);

    OMenuWidget* createWidget(SystemMenu menu);



protected:
    virtual void showEvent(QShowEvent* e) override;

    void retranslateUi();

private:
    OK_RESOURCE_PTR(UIMainWindow);

    Ui::OMainMenu* ui;

    base::DelayedCallTimer* delayCaller_;

    QList<OMenuItem*> items;

    QMap<SystemMenu, OMenuWidget*> menuMap;

    std::unique_ptr<OMenuWidget> createChatModule();
    std::unique_ptr<OMenuWidget> createConfigModule();

#ifdef ENABLE_Platform
    std::unique_ptr<OMenuWidget> createPlatformModule();
#endif
#ifdef ENABLE_Meet
    std::unique_ptr<OMenuWidget> createMeetingModule();
#endif
#ifdef ENABLE_Classroom
    std::unique_ptr<OMenuWidget> createClassroomModule();
#endif
#ifdef ENABLE_Document
    std::unique_ptr<OMenuWidget> createDocumentModule();
#endif

signals:
    void menuPushed(SystemMenu menu, bool checked);

public slots:
    void onButtonToggled(int id, bool toggle);
    void setAvatar(const QByteArray&);
};

}  // namespace UI

#endif  // OMAINMENU_H
