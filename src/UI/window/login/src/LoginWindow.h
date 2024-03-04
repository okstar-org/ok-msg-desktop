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

#include <QMainWindow>
#include <memory>

#include "LoginWidget.h"
#include "base/resources.h"

//初始化资源加载器
OK_RESOURCE_LOADER(UIWindowLogin)

namespace Ui {
class LoginWindow;
} // namespace Ui

namespace UI {


class LoginWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit LoginWindow(QWidget *parent = nullptr);
  ~LoginWindow();

private:
  Ui::LoginWindow *ui;

  //资源指针申明
  OK_RESOURCE_PTR(UIWindowLogin);

signals:
  void loginResult(ok::session::SignInInfo &, ok::session::LoginResult &);

public slots:
  void onProfileLoadFailed(QString msg);
};

} // namespace UI
