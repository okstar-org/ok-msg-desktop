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
#include <QMessageBox>
#include <QUrl>
#include <memory>

#include "OMainMenu.h"
#include "ui_OMainMenu.h"

#include "base/files.h"
#include "base/images.h"
#include "base/resources.h"

namespace UI {

OMainMenu::OMainMenu(QWidget* parent) : QFrame(parent), ui(new Ui::OMainMenu), _showTimes(0) {
    qDebug() << __func__;

    OK_RESOURCE_INIT(UIWindowMain);

    ui->setupUi(this);

    // 设置样式
    QString qss = ok::base::Files::readStringAll(":/qss/menu.css");
    setStyleSheet(qss);

    ui->chatBtn->setCursor(Qt::PointingHandCursor);
    ui->chatBtn->setIconSize(QSize(40, 40));
    ui->settingBtn->setCursor(Qt::PointingHandCursor);
    ui->settingBtn->setIconSize(QSize(40, 40));
    ui->platformBtn->setCursor(Qt::PointingHandCursor);
    ui->platformBtn->setIconSize(QSize(40, 40));

    delayCaller_ = std::make_unique<base::DelayedCallTimer>();
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
        updateUI();
    }
}

void OMainMenu::updateUI() { on_chatBtn_clicked(true); }

/**
 * 聊天
 * @brief OMainMenu::on_chatBtn_clicked
 * @param checked
 */
void OMainMenu::on_chatBtn_clicked(bool checked) {
    ui->chatBtn->setChecked(true);
    ui->settingBtn->setChecked(false);
    ui->platformBtn->setChecked(false);
    emit menuPushed(ok::base::PageMenu::chat, ui->chatBtn->isChecked());
}

/**
 * 设置按钮
 * @param checked
 */
void OMainMenu::on_settingBtn_clicked(bool checked) {
    ui->platformBtn->setChecked(false);
    ui->chatBtn->setChecked(false);
    ui->settingBtn->setChecked(true);
    emit menuPushed(ok::base::PageMenu::setting, ui->settingBtn->isChecked());
}

/**
 * 工作平台
 * @param checked
 */
void OMainMenu::on_platformBtn_clicked(bool checked) {
    ui->chatBtn->setChecked(false);
    ui->settingBtn->setChecked(false);
    ui->platformBtn->setChecked(true);
    emit menuPushed(ok::base::PageMenu::platform, ui->platformBtn->isChecked());
}

}  // namespace UI
