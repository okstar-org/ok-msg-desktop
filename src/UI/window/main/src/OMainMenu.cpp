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
#include <memory>

#include "Bus.h"
#include "OMainMenu.h"
#include "application.h"
#include "ui_OMainMenu.h"

#include "base/files.h"
#include "base/images.h"
#include "base/resources.h"
#include "lib/storeage/settings/OkSettings.h"
#include "lib/storeage/settings/translator.h"

#include <QButtonGroup>

namespace UI {

OMainMenu::OMainMenu(QWidget* parent) : QFrame(parent), ui(new Ui::OMainMenu) {
    qDebug() << __func__;

    OK_RESOURCE_INIT(UIWindowMain);

    ui->setupUi(this);

    // 设置样式
    QString qss = ok::base::Files::readStringAll(":/qss/menu.css");
    setStyleSheet(qss);

    QButtonGroup* group = new QButtonGroup(this);
    group->setExclusive(true);
    ui->chatBtn->setToolTip(tr("Message"));
    ui->chatBtn->setCursor(Qt::PointingHandCursor);
    group->addButton(ui->chatBtn, static_cast<int>(SystemMenu::chat));

    ui->settingBtn->setToolTip(tr("Setting"));
    ui->settingBtn->setCursor(Qt::PointingHandCursor);
    group->addButton(ui->settingBtn, static_cast<int>(SystemMenu::setting));

    ui->platformBtn->setToolTip(tr("Work platform"));
    ui->platformBtn->setCursor(Qt::PointingHandCursor);
    group->addButton(ui->platformBtn, static_cast<int>(SystemMenu::platform));

    ui->meetBtn->setToolTip(tr("Meeting"));
    ui->meetBtn->setCursor(Qt::PointingHandCursor);
    group->addButton(ui->meetBtn, static_cast<int>(SystemMenu::meeting));
    connect(group, &QButtonGroup::idToggled, this, &OMainMenu::onButtonToggled);

    QString locale = lib::settings::OkSettings::getInstance().getTranslation();
    settings::Translator::translate(OK_UIWindowMain_MODULE, locale);
    settings::Translator::registerHandler([this] { retranslateUi(); }, this);

    retranslateUi();

    connect(ok::Application::Instance()->bus(), &ok::Bus::languageChanged, [](QString locale0) {
        settings::Translator::translate(OK_UIWindowMain_MODULE, locale0);
    });

    delayCaller_ = new base::DelayedCallTimer(this);
    delayCaller_->call(1000, [&]() { check(SystemMenu::chat); });
}

OMainMenu::~OMainMenu() {
    qDebug() << __func__;
    settings::Translator::unregister(this);
    delete ui;
}

void OMainMenu::setAvatar(const QPixmap& pixmap) {
    QSize size = ui->label_avatar->size() * ui->label_avatar->devicePixelRatioF();
    auto newImage = ok::base::Images::roundRectPixmap(pixmap, size,
                                                      100 * ui->label_avatar->devicePixelRatioF());
    newImage.setDevicePixelRatio(ui->label_avatar->devicePixelRatioF());
    ui->label_avatar->setPixmap(newImage);
}

void OMainMenu::showEvent(QShowEvent* e) {
    Q_UNUSED(e);
}

void OMainMenu::retranslateUi() {
    ui->chatBtn->setToolTip(tr("Message"));
    ui->settingBtn->setToolTip(tr("Setting"));
    ui->platformBtn->setToolTip(tr("Work platform"));
    ui->meetBtn->setToolTip(tr("Meeting"));
    ui->retranslateUi(this);
}

void OMainMenu::check(SystemMenu menu) {
    ui->chatBtn->setChecked(false);
    ui->settingBtn->setChecked(false);
    ui->platformBtn->setChecked(false);
    ui->meetBtn->setChecked(false);

    switch (menu) {
        case SystemMenu::chat:
            ui->chatBtn->setChecked(true);
            break;
        case SystemMenu::platform:
            ui->platformBtn->setChecked(true);
            break;
        case SystemMenu::meeting:
            ui->meetBtn->setChecked(true);
            break;
        case SystemMenu::setting:
            ui->settingBtn->setChecked(true);
            break;
    }
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

}  // namespace UI
