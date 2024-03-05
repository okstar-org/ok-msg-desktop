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
#include "base/resources.h"
#include "base/images.h"

namespace UI {

OMainMenu::OMainMenu(QWidget *parent)
    : QFrame(parent), ui(new Ui::OMainMenu), m_current(PageMenu::welcome),
      _showTimes(0) {

  OK_RESOURCE_INIT(UIWindowMain);

  ui->setupUi(this);
  delayCaller_ = (std::make_unique<base::DelayedCallTimer>());

  // 设置样式
  QString qss = base::Files::readStringAll(":/qss/menu.qss");
  setStyleSheet(qss);

  ui->chatBtn->setCursor(Qt::PointingHandCursor);
  ui->settingBtn->setCursor(Qt::PointingHandCursor);
}

OMainMenu::~OMainMenu() { delete ui; }

void OMainMenu::setAvatar(const QPixmap &pixmap) {
  auto newImage = base::Images::roundRectPixmap(pixmap, ui->label_avatar->size(), 100);
  ui->label_avatar->setPixmap(newImage);
}

void OMainMenu::showEvent(QShowEvent *e) {
  Q_UNUSED(e);
  _showTimes++;
  if (_showTimes == 1) {
    updateUI();
  }
}

void OMainMenu::updateUI() { on_chatBtn_clicked(true); }

void OMainMenu::on_personalBtn_clicked(bool checked) {}

void OMainMenu::on_chatBtn_clicked(bool checked) {
  ui->chatBtn->setChecked(true);
  ui->settingBtn->setChecked(false);
  emit menuPushed(UI::PageMenu::chat, ui->chatBtn->isChecked());
  emit onPage(UI::PageMenu::chat);
}

void OMainMenu::onSetting() {

  ui->settingBtn->setChecked(true);
  m_current = PageMenu::setting;

  emit onPage(PageMenu::setting);
}

/**
 * 设置按钮
 * @param checked
 */
void OMainMenu::on_settingBtn_clicked(bool checked) {

  ui->chatBtn->setChecked(false);

  emit menuPushed(UI::PageMenu::setting, ui->settingBtn->isChecked());
  emit onPage(UI::PageMenu::setting);
}

} // namespace UI
