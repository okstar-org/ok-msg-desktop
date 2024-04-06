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

#include "AuthSession.h"

#include <list>
#include <memory>
#include <string>
#include <thread>

#include <QTimer>

#include "base/logs.h"
#include "lib/messenger/IM.h"
#include "lib/network/NetworkHttp.h"
#include "lib/backend/PassportService.h"

namespace ok {
namespace session {

using namespace network;
using namespace ok::backend;

AuthSession::AuthSession(QObject *parent)
    : QObject(parent),
      m_networkManager(std::make_unique<network::NetworkHttp>(this)),    //
      _status(Status::NONE)                                     //
{
  qRegisterMetaType<SignInInfo>("SignInInfo");
  qRegisterMetaType<LoginResult>("LoginResult");
}

AuthSession::~AuthSession() { qDebug() << "~AuthSession"; }

AuthSession *AuthSession::Instance() {
  static AuthSession *self = nullptr;
  if (!self) {
    self = new AuthSession;
  }
  return self;
}

Status AuthSession::status() const { return _status; }

void AuthSession::initTimer() {
  //		_timer = std::make_unique<QTimer>(this);
  //		_timer->start(10 * 1000);
  //		QObject::connect(_timer.get(), SIGNAL(timeout()), this,
  // SLOT(timerUp()));
}

void AuthSession::timerUp() {
  DEBUG_LOG(("AuthSession::timerUp"));
  //  if (_status == CONNECT_STATUS::DISCONNECTED) {
  //    DEBUG_LOG(("CONNECT_STATUS::DISCONNECTED doConnect..."));
  //    doConnect();
  //  }
}

void AuthSession::doConnect() {
  DEBUG_LOG(("doConnect..."));

  _status = Status::CONNECTING;

  LoginResult result{_status, "..."};
  emit loginResult(m_signInInfo, result);

  passportService->getAccount(m_signInInfo.account, [&](Res<SysAccount> &res) {
    DEBUG_LOG(("Res.code:%1").arg(res.code));

    if (res.code != 0) {
      _status = Status::FAILURE;
      LoginResult result{_status, res.msg};
      emit loginResult(m_signInInfo, result);
      return;
    }

    if (!res.success()) {
      _status = Status::FAILURE;
      LoginResult result{_status, res.msg};
      emit loginResult(m_signInInfo, result);
      return;
    }

    DEBUG_LOG(("Res.data=>%1").arg(res.data->toString()));

    m_signInInfo.account = res.data->username;

    QStringList l;
    _im = new ::lib::messenger::IM(m_signInInfo.host,
                                   m_signInInfo.account,
                                   m_signInInfo.password,l);
    connect(_im, &::lib::messenger::IM::connectResult,
            [&](::lib::messenger::IMStatus status) {
              QString msg;
              if (status == ::lib::messenger::IMStatus::CONNECTED) {
                _status = Status::SUCCESS;
                LoginResult result{Status::SUCCESS, msg};
                emit loginResult(m_signInInfo, result);
                return;
              }

              // 错误处理
              _status = Status::FAILURE;
              switch (status) {
              case ::lib::messenger::IMStatus::AUTH_FAILED: {
                msg = "认证失败！";
                break;
              }
              case ::lib::messenger::IMStatus::DISCONNECTED: {
                msg = "无法连接！";
                break;
              }
              case ::lib::messenger::IMStatus::CONN_ERROR: {
                msg = "连接异常！";
                break;
              }
              case ::lib::messenger::IMStatus::CONNECTING: {
                msg = "...";
                break;
              }
              case ::lib::messenger::IMStatus::TLS_ERROR: {
                msg = "TLS证书验证失败";
                break;
              }
              case ::lib::messenger::IMStatus::OUT_OF_RESOURCE: {
                msg = "系统资源受限！";
                break;
              }
              case ::lib::messenger::IMStatus::TIMEOUT: {
                msg = "请求超时！";
                break;
              }
              }

              LoginResult result{Status::FAILURE, msg};
              emit loginResult(m_signInInfo, result);
            });

    _im->start();
  }, [&](const QString &msg){
    _status = Status::FAILURE;
    LoginResult result{Status::FAILURE, msg};
    emit loginResult(m_signInInfo, result);
  });
}

void AuthSession::doLogin(const SignInInfo &signInInfo) {
  DEBUG_LOG(("account:%1 password:%2")
                .arg(signInInfo.account)
                .arg(signInInfo.password));

  m_signInInfo = signInInfo;

  passportService = (std::make_unique<ok::backend::PassportService>(m_signInInfo.stackUrl)), //
  okAccount = std::make_unique<ok::base::OkAccount>(signInInfo.account);

  if (_status == ok::session::Status::CONNECTING) {
    DEBUG_LOG(("The connection is connecting."));
    return;
  }

  if (_status == ok::session::Status::SUCCESS) {
    DEBUG_LOG(("The connection is connected."));
    return;
  }

  if (_mutex.try_lock()) {

    // 初始化定时器
    initTimer();

    // 建立连接
    doConnect();

    _mutex.unlock();
  }
}


} // namespace session
} // namespace ok