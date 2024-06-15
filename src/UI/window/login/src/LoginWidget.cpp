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

#include "LoginWidget.h"

#include <QMessageBox>
#include <QPaintEvent>
#include <QShortcut>
#include <QTranslator>
#include <QWidget>
#include <memory>

#include "UI/core/SettingManager.h"
#include "base/logs.h"
#include "base/widgets.h"
#include "lib/backend/OkCloudService.h"
#include "base/OkSettings.h"
#include "lib/settings/translator.h"
#include "ui_LoginWidget.h"

namespace UI {

using namespace core;
using namespace ok::session;
using namespace ok::base;

LoginWidget::LoginWidget(bool bootstrap, QWidget *parent)
    : QWidget(parent),           //
      ui(new Ui::LoginWidget),   //
      bootstrap{bootstrap},
      m_loginKey(nullptr),       //
      m_settingManager(nullptr), //
      m_loaded(0) {
    qDebug() <<__func__;

  ui->setupUi(this);
  ui->loginBtn->setCursor(Qt::PointingHandCursor);
  /**
   * 安装事件过滤器
   */
  ui->signUp->installEventFilter(this);
  ui->findPwd->installEventFilter(this);

  /**
   * 增加快捷键
   */
  m_loginKey = new QShortcut(QKeySequence(Qt::Key_Return), this);
  connect(m_loginKey, SIGNAL(activated()), //
          this, SLOT(on_loginBtn_released()));

  okCloudService = new ok::backend::OkCloudService(this);

  settings::Translator::registerHandler([&] { retranslateUi(); }, this);

  m_settingManager = new SettingManager(this);


  if(bootstrap){
    qDebug()<<__func__ <<"Init timer";
    m_timer=std::make_unique<QTimer>();
    m_timer->start(1000);
    connect(m_timer.get(), &QTimer::timeout, this, &LoginWidget::onTimeout);
  }

  // 初始化
  init();
}

LoginWidget::~LoginWidget() {
    qDebug() <<__func__;
    settings::Translator::unregister(this);
    delete ui;
}

void LoginWidget::init() {

  m_settingManager->getAccount([&](const QString& acc, const QString& password) {
    ui->rember->setChecked(!acc.isEmpty());
    ui->accountInput->setText(acc);
    ui->passwordInput->setText(password);
  });

  //==========国际化==========//
  // 先获取当前语言
  auto &s = ok::base::OkSettings::getInstance();
  qDebug() << "Settings translation:" << s.getTranslation();

  for (int i = 0; i < s.getLocales().size(); ++i) {
    auto &locale = s.getLocales().at(i);
    QString langName = QLocale(locale).nativeLanguageName();
    ui->language->addItem(langName);
  }

  // 当前语言状态
  auto i = s.getLocales().indexOf(s.getTranslation());
  if (i >= 0 && i < ui->language->count())
    ui->language->setCurrentIndex(i + 1);



  retranslateUi();
  //==========国际化==========//

  //服务供应商
  okCloudService->GetFederalInfo(
      [&](ok::backend::Res<ok::backend::FederalInfo> &res) {
        for (const auto &item : res.data->states) {
          if (!item.xmppHost.isEmpty()) {
            ui->providers->addItem(item.name);
            m_hosts.push_back(item.xmppHost);
            m_stacks.push_back(item.stackUrl);
          }
        }
        if (ui->providers->maxCount() > 0) {
          ui->providers->setCurrentIndex(1);
          m_loaded++;
        }
      },
      [&](const QString& error) {
      onError(error);
  });
}

void LoginWidget::deinit()
{

}

void LoginWidget::doLogin() {
    if(m_error)
        return;

  if (m_loaded < 1) {
    onError(tr("Please waiting the page is loaded"));
    return;
  }

  // 获取服务提供商
  auto providerIdx = ui->providers->currentIndex();
  if (!(providerIdx > 0)) {
    onError(tr("Please select service provider"));
    return;
  }

  // 对登录时状态判断
  auto status = ok::session::AuthSession::Instance()->status();
  switch (status) {
  case ok::session::Status::SUCCESS: {
    qDebug(("SUCCESS ..."));
    return;
  }
  case ok::session::Status::CONNECTING: {
    qDebug(("CONNECTING ..."));
    //    sess->interrupt();
    return;
  }
  default: {
    QString host = m_hosts.at(providerIdx - 1);
    qDebug() << "Select provider host:" << host;

    // 登陆
    QString account(ui->accountInput->text());
    QString password(ui->passwordInput->text());
    // 对账号和密码判断
    if (account.isEmpty()) {
      return;
    }
    if (ui->rember->isCheckable()) {
      m_settingManager->saveAccount(account, password);
    } else {
      m_settingManager->clearAccount();
    }

    SignInInfo info = {
        .account = account,
        .password = password,
        .host = host,
        .stackUrl = m_stacks.at(providerIdx - 1)};
    auto _session = ok::session::AuthSession::Instance();
    connect(_session, &AuthSession::loginResult, //
            this, &LoginWidget::onConnectResult);
    _session->doLogin(info);
    break;
  }
  }
}

void LoginWidget::onConnectResult(ok::session::SignInInfo info,
                                  ok::session::LoginResult result) {

  qDebug()<<("msg:")<<(result.msg);

  switch (result.status) {
  case ok::session::Status::NONE:
    break;
  case ok::session::Status::CONNECTING: {
    ui->loginMessage->setText(tr("..."));
    ui->loginBtn->setText(tr("Logging in"));
    QString account(ui->accountInput->text());
    QString password(ui->passwordInput->text());
    emit loginFailed(account, password);
    break;
  }
  case ok::session::Status::SUCCESS: {
    ui->loginMessage->setText(tr("login success"));
    QString account(ui->accountInput->text());
    QString password(ui->passwordInput->text());
    emit loginSuccess(account, password);
    break;
  }
  case ok::session::Status::FAILURE:
    ui->loginBtn->setText(tr("Login"));
    onError(result.msg);
    break;
  }
  emit loginResult(info, result);
}

void LoginWidget::on_loginBtn_released() { doLogin(); }

/**
 * 语言选择事件
 */
void LoginWidget::on_language_currentIndexChanged(int index) {
  qDebug() << "Select language:" << index;
  if (index == 0) {
    return;
  }

  auto &s = ok::base::OkSettings::getInstance();
  const QString &locale = s.getLocales().at(index - 1);
  s.setTranslation(locale);
  s.saveGlobal();
  qDebug() << "Selected locale:" << locale;

  settings::Translator::translate(OK_UIWindowLogin_MODULE, locale);
  qDebug() << "Translated locale:" << locale;
}

/**
 * 服务提供者事件
 */
void LoginWidget::on_providers_currentIndexChanged(int index) {
  qDebug() << "Select provider:" << index;
}

void LoginWidget::retranslateUi() { ui->retranslateUi(this); }

void LoginWidget::onError(const QString &msg) {
  qWarning() <<__func__ << msg;
  setMsg(msg);
  //登录失败退出定时器
  m_timer.reset();
  m_error = true;
}

void LoginWidget::setMsg(const QString &msg)
{
  ui->loginMessage->setText(msg);
}

bool LoginWidget::eventFilter(QObject *obj, QEvent *event) {
  switch (event->type()) {
  case QEvent::MouseButtonPress: {
    if (obj == ui->signUp) {
      QDesktopServices::openUrl(QUrl("http://stack.okstar.org.cn/auth/register"));
    } else if (obj == ui->findPwd) {
      QDesktopServices::openUrl(QUrl("http://stack.okstar.org.cn/auth/forgot"));
    }
    break;
  }
  default:
    break;
  };
  return QObject::eventFilter(obj, event);
}

void LoginWidget::showEvent(QShowEvent *e)
{

}

void LoginWidget::onTimeout()
{
    if(ui->rember->isChecked() && ui->providers->count()>0){
        if(!ui->passwordInput->text().isEmpty()&&!ui->accountInput->text().isEmpty()){
            on_loginBtn_released();
        }
    }
}

} // namespace UI
