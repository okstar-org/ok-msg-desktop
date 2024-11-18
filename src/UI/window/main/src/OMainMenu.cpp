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

#include "application.h"
#include "Bus.h"
#include "OMainMenu.h"
#include "ui_OMainMenu.h"

#include "base/files.h"
#include "base/images.h"
#include "base/resources.h"
#include "base/OkSettings.h"
#include "lib/settings/translator.h"

#include <QButtonGroup>

namespace UI {

OMainMenu::OMainMenu(QWidget* parent) : QFrame(parent), ui(new Ui::OMainMenu), _showTimes(0) {
    qDebug() << __func__;

    OK_RESOURCE_INIT(UIWindowMain);

    ui->setupUi(this);

    // 设置样式
    QString qss = ok::base::Files::readStringAll(":/qss/menu.css");
    setStyleSheet(qss);

    ui->chatBtn->setCursor(Qt::PointingHandCursor);
    ui->settingBtn->setCursor(Qt::PointingHandCursor);
    ui->platformBtn->setCursor(Qt::PointingHandCursor);
    ui->meetBtn->setCursor(Qt::PointingHandCursor);

    ui->chatBtn->setToolTip(tr("Message"));
    ui->settingBtn->setToolTip(tr("Meeting"));
    ui->platformBtn->setToolTip(tr("Work platform"));
    ui->meetBtn->setToolTip(tr("Setting"));

    delayCaller_ = std::make_unique<base::DelayedCallTimer>();

    QButtonGroup* group = new QButtonGroup(this);
    group->setExclusive(true);
    group->addButton(ui->chatBtn, static_cast<int>(ok::base::PageMenu::chat));
    group->addButton(ui->settingBtn, static_cast<int>(ok::base::PageMenu::setting));
    group->addButton(ui->platformBtn, static_cast<int>(ok::base::PageMenu::platform));
    group->addButton(ui->meetBtn, static_cast<int>(ok::base::PageMenu::metting));
    connect(group, &QButtonGroup::idToggled, this, &OMainMenu::onButtonToggled);

    QString locale = ok::base::OkSettings::getInstance().getTranslation();
    settings::Translator::translate(OK_UIWindowMain_MODULE, locale);
    settings::Translator::registerHandler([this] { retranslateUi(); }, this);

    retranslateUi();

    connect(ok::Application::Instance()->bus(), &ok::Bus::languageChanged,
            [](QString locale0) { settings::Translator::translate(OK_UIWindowMain_MODULE, locale0); });

}

OMainMenu::~OMainMenu() {
    qDebug() << __func__;
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
    _showTimes++;
    if (_showTimes == 1) {
        ui->chatBtn->setChecked(true);
    }
}

void OMainMenu::retranslateUi()
{
    ui->chatBtn->setToolTip(tr("Message"));
    ui->settingBtn->setToolTip(tr("Meeting"));
    ui->platformBtn->setToolTip(tr("Work platform"));
    ui->meetBtn->setToolTip(tr("Setting"));
    ui->retranslateUi(this);
}


void OMainMenu::onButtonToggled(int id, bool toggle) {
    if (id < 0 || !toggle) {
        return;
    }
    switch (static_cast<ok::base::PageMenu>(id))
    {
        case ok::base::PageMenu::chat:
        case ok::base::PageMenu::setting:
        case ok::base::PageMenu::platform:
        case ok::base::PageMenu::metting:
            emit menuPushed(static_cast<ok::base::PageMenu>(id), true);
            break;
        default:
            break;
    }
}

}  // namespace UI
