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

#include <QUrl>
#include <QButtonGroup>
#include <algorithm>


#include "Bus.h"
#include "OMainMenu.h"
#include "OMenuItem.h"
#include "application.h"

#include "modules/platform/src/Platform.h"
#include "ui_OMainMenu.h"

#include "base/files.h"
#include "base/images.h"
#include "base/resources.h"
#include "lib/storage/settings/translator.h"
#include "modules/im/src/nexus.h"
#include "modules/document/src/Document.h"
#include "modules/classroom/src/Classroom.h"
#include "modules/config/src/Config.h"
#include "modules/meet/src/Meet.h"

namespace UI {


enum class SystemMenu {
    chat,   //消息
#ifdef ENABLE_Document
    document,   // 文档
#endif
#ifdef ENABLE_Meet
    meeting,//会议
#endif
#ifdef ENABLE_Classroom
    classroom,  // 课堂
#endif
    platform,//工作平台
    setting,//配置
};

static QList<MenuItem> MenuItemList = {
        MenuItem{SystemMenu::chat, "chatBtn", ("Message")},
#ifdef ENABLE_Document
        MenuItem{SystemMenu::document, "docBtn", "Document"},
#endif
#ifdef ENABLE_Meet
        MenuItem{SystemMenu::meeting, "meetBtn", "Meeting"},
#endif
#ifdef ENABLE_Classroom
        MenuItem{SystemMenu::classroom, "classroomBtn", "Classroom"},
#endif
#ifdef ENABLE_Platform
        MenuItem{SystemMenu::platform, "platformBtn", "Work platform"},
#endif
        };



OMainMenu::OMainMenu(QWidget* parent) : QFrame(parent), ui(new Ui::OMainMenu) {
    qDebug() << __func__;

    OK_RESOURCE_INIT(UIMainWindow);

    ui->setupUi(this);

    // 设置样式
    QString qss = ok::base::Files::readStringAll(":/qss/menu.css");
    setStyleSheet(qss);

    auto* group = new QButtonGroup(this);
    group->setExclusive(true);
    connect(group, &QButtonGroup::idToggled, this, &OMainMenu::onButtonToggled);

    for(auto &i : MenuItemList){
        auto item = new OMenuItem(i, this);
        item->setCheckable(true);
        item->setIconSize(QSize(40, 40));
        items.append(item);
        ui->menuLayout->addWidget(item, Qt::AlignHCenter);
        group->addButton(item, static_cast<int>(i.menu));
    }
    //settings
    group->addButton(ui->settingBtn, static_cast<int>(SystemMenu::setting));


    retranslateUi();

    auto profile = ok::Application::Instance()->getProfile();
    // auto bus = ok::Application::Instance()->bus();

    setAvatar(profile->getAvatar());
    connect(profile, &lib::session::Profile::selfAvatarChanged, this, &OMainMenu::setAvatar);

    delayCaller_ = new base::DelayedCallTimer(this);
    delayCaller_->call(100, [&]() { check(SystemMenu::chat); });
}

OMainMenu::~OMainMenu() {
    qDebug() << __func__;
    
    delete ui;
}

void OMainMenu::setAvatar(const QByteArray& avatar) {
    qDebug() << __func__;

    QPixmap pixmap;
    ok::base::Images::putToPixmap(avatar, pixmap);
    auto size = ui->label_avatar->size() * ui->label_avatar->devicePixelRatioF();
    auto radius = 100 * ui->label_avatar->devicePixelRatioF();
    auto newImage = ok::base::Images::roundRectPixmap(pixmap, size, radius);
    newImage.setDevicePixelRatio(ui->label_avatar->devicePixelRatioF());
    ui->label_avatar->setPixmap(newImage);
}

void OMainMenu::showEvent(QShowEvent* e) {
    Q_UNUSED(e);
}

void OMainMenu::retranslateUi() {
    ui->settingBtn->setToolTip(tr("Setting"));
    ui->retranslateUi(this);

    for (auto& m : items) {
        m->retranslateUi();
    };

}

void OMainMenu::check(SystemMenu menu) {
    ui->settingBtn->setChecked(false);
    for (auto& m : items) {
        m->setChecked(false);
    }


    auto find = std::find_if(items.begin(), items.end(), [menu](auto e){
        return e->getSysMenu() == menu;
    });
    if(find == items.end()){
        return;
    }


    auto item = *find;
    item->setChecked(true);

}

OMenuWidget* OMainMenu::createWidget(SystemMenu menu)
{
    auto find = std::find_if(items.begin(), items.end(), [menu](auto e){
        return e->getSysMenu() == menu;
    });

    std::unique_ptr<OMenuWidget> w;
    if(find == items.end()){
        w = createConfigModule();
    }else{
        switch (menu) {
            case SystemMenu::chat:{
                w = createChatModule();
                break;
            }

            case SystemMenu::setting:{
                w = createConfigModule();
                break;
            }
    #ifdef ENABLE_Platform
            case SystemMenu::platform:{
                w = createPlatformModule();
                break;
            }
    #endif
    #ifdef ENABLE_Document
            case SystemMenu::document:{
                w = createDocumentModule();
                break;
            }
    #endif

    #ifdef ENABLE_Meet
            case SystemMenu::meeting:{
                w = createMeetingModule();
                break;
            }
    #endif

    #ifdef ENABLE_Classroom
            case SystemMenu::classroom:{
                w = createClassroomModule();
                break;
            }
    #endif
        };
    }

    menuMap.insert(menu, w.get());

    return w.release();
}

void OMainMenu::onButtonToggled(int id, bool toggle) {
    if (!toggle) {
        return;
    }

    if (id < 0) {
        return;
    }

    auto menu = static_cast<SystemMenu>(id);
    check(menu);

    emit menuPushed(menu, true);
}

/**
 * 创建聊天模块
 * @param pWindow
 * @return
 */
std::unique_ptr<OMenuWidget> OMainMenu::createChatModule() {
    qDebug() << "Creating chat module...";

    auto m = module::im::Nexus::Create();
    auto nexus = static_cast<module::im::Nexus*>(m);

            // connect(nexus, &Nexus::updateAvatar,
            // ok::Application::Instance(), &ok::Application::onAvatar);

    connect(nexus, &module::im::Nexus::destroyProfile,  //
            ok::Application::Instance(), &ok::Application::on_logout);
    // connect(nexus, &Nexus::exit,
    // ok::Application::Instance(), &ok::Application::on_exit);

    auto w = new OMenuWidget(this);
    w->setModule(m);
    w->setLayout(new QGridLayout());
    w->layout()->setContentsMargins(0, 0, 0, 0);
    w->layout()->addWidget(m->widget());
    return std::unique_ptr<OMenuWidget>(w);
}

#ifdef ENABLE_Meet
/**
 * 创建会议模块
 * @return
 */
std::unique_ptr<OMenuWidget> OMainMenu::createMeetingModule()
{
    auto m = new module::meet::Meet();
    auto w = new OMenuWidget(this);
    w->setModule(m);
    w->setLayout(new QGridLayout());
    w->layout()->setContentsMargins(0, 0, 0, 0);
    w->layout()->addWidget(m->widget());
    return std::unique_ptr<OMenuWidget>(w);
}
#endif

#ifdef ENABLE_Classroom
/**
 * 创建课堂模块
 * @return
 */
std::unique_ptr<OMenuWidget> OMainMenu::createClassroomModule() {
    auto m = new module::classroom::Classroom();
    auto w = new OMenuWidget(this);
    w->setModule(m);
    w->setLayout(new QGridLayout());
    w->layout()->setContentsMargins(0, 0, 0, 0);
    w->layout()->addWidget(m->widget());
    return std::unique_ptr<OMenuWidget>(w);
}
#endif

/**
 * 创建配置模块
 * @return
 */
std::unique_ptr<OMenuWidget> OMainMenu::createConfigModule() {
    auto m = new module::config::Config();
    auto w = new OMenuWidget(this);
    w->setModule(m);
    w->setLayout(new QGridLayout());
    w->layout()->setContentsMargins(0, 0, 0, 0);
    w->layout()->addWidget(m->widget());
    return std::unique_ptr<OMenuWidget>(w);
}

#ifdef ENABLE_Document
/**
 * 创建配置模块
 * @return
 */
std::unique_ptr<OMenuWidget> OMainMenu::createDocumentModule() {
    auto m = new module::doc::Document();
    auto w = new OMenuWidget(this);
    w->setModule(m);
    w->setLayout(new QGridLayout());
    w->layout()->setContentsMargins(0, 0, 0, 0);
    w->layout()->addWidget(m->widget());
    return std::unique_ptr<OMenuWidget>(w);
}
#endif

#ifdef ENABLE_Platform
/**
 * 创建工作平台模块
 * @param pWindow
 * @return
 */
std::unique_ptr<OMenuWidget> OMainMenu::createPlatformModule() {
    auto m = new module::platform::Platform();
    auto w = new OMenuWidget(this);
    w->setModule(m);
    w->setLayout(new QGridLayout());
    w->layout()->setContentsMargins(0, 0, 0, 0);
    w->layout()->addWidget(m->widget());
    return std::unique_ptr<OMenuWidget>(w);
}
#endif
}  // namespace UI
