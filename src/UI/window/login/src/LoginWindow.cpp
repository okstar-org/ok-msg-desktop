﻿/*
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

#include "BannerWidget.h"
#include "LoginWindow.h"

#include "LoginWidget.h"
#include "base/files.h"
#include "base/r.h"
#include "base/resources.h"
#include "ui_LoginWindow.h"


namespace UI {


using namespace ok::session;

/* 登录主窗口 */
LoginWindow::LoginWindow(bool bootstrap, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::LoginWindow) {

  //初始化资源
  OK_RESOURCE_INIT(UIWindowLogin);

  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(APPLICATION_NAME);
  setFixedSize(LOGIN_WINDOW_WIDTH, LOGIN_WINDOW_HEIGHT);

  ui->setupUi(this);

  bannerWidget = new BannerWidget(this);
  bannerWidget->setFixedWidth(width()/2);

  ui->hBoxLayout->addWidget(bannerWidget);

  loginWidget = new LoginWidget(bootstrap, this);
  ui->hBoxLayout->addWidget(loginWidget);


  // 设置样式
  QString qss = ok::base::Files::readStringAll(":/qss/login.qss");
  setStyleSheet(qss);

  connect(loginWidget, &UI::LoginWidget::loginResult,
          [&](ok::session::SignInInfo &info,  //
              ok::session::LoginResult &result) {
            emit loginResult(info, result);
          });
}

LoginWindow::~LoginWindow() {
    qDebug() << __func__;
}

void LoginWindow::onProfileLoadFailed(QString msg) {
  loginWidget->onError(200, msg);
}

} // namespace UI
