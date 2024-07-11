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

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QUrl>

#include <memory>
#include <utility>
#include "base/OkAccount.h"
#include "base/basic_types.h"
#include "base/jsons.h"

#include "lib/messenger/messenger.h"
#include "lib/network/NetworkHttp.h"
#include "lib/network/network.h"

namespace lib::messenger {
class IM;
}

namespace ok::backend {
class PassportService;
}

namespace ok {
namespace session {

enum class Status {
  NONE = 0,
  CONNECTING,
  SUCCESS,
  FAILURE,
};

class LoginResult {
public:
  Status status = Status::NONE;
  QString msg;
};

class AuthInfo {
public:
  AuthInfo() = default;

  AuthInfo(AuthInfo &info) {
    token_ = info.getToken();
    clientName_ = info.getClientName();
  }

  ~AuthInfo() = default;

  const QString &getToken() const { return token_; }

  const QString &getClientName() const { return clientName_; }

  void fromJSON(const QJsonObject &data) {
    token_ = data.value("token").toString();
    clientName_ = data.value("clientName").toString();
  }

  QJsonObject toJSON() {
    QJsonObject qo;
    qo.insert("token", token_);
    qo.insert("clientName", clientName_);
    return qo;
  }

private:
  QString token_;
  QString clientName_;
};

/**
 * 登录信息
 */
struct SignInInfo {
  // 账号
  QString account;
  // 密码
  QString password;

  // username
  QString username;
  // xmpp host
  QString host;
  // stack url
  QString stackUrl;
};

class AuthSession : public QObject {
  Q_OBJECT
public:
  AuthSession(QObject *parent = nullptr);
  ~AuthSession() override;

  static AuthSession *Instance();

  Status status() const;

  void doLogin(const SignInInfo &signInInfo);

  [[nodiscard]] const SignInInfo &getSignInInfo() const {
    return m_signInInfo;
  };

  //  [[nodiscard]] const QString &getToken() const { return token_; };

  [[nodiscard]] const AuthInfo &authInfo() const { return _authInfo; }

  [[nodiscard]] ok::base::OkAccount *account() const { return okAccount.get(); }

  ::lib::messenger::IM *im() { return _im; }

protected:
  void doConnect();

private:
  QStringList l;

  SignInInfo m_signInInfo;

  std::shared_ptr<AuthSession> _session;


  std::unique_ptr<network::NetworkHttp> m_networkManager;
  AuthInfo _authInfo;

  QMutex _mutex;

  std::unique_ptr<ok::base::OkAccount> okAccount;
  std::unique_ptr<ok::backend::PassportService> passportService;

  ::lib::messenger::IM *_im;
  Status _status;

signals:
  void loginResult(SignInInfo, LoginResult); // LoginResult
  void loginSuccessed();
  void imStarted(SignInInfo);

public slots:
  void onLoginSuccessed();
  void onIMConnectStatus(::lib::messenger::IMConnectStatus status);
  void onIMStarted();
};
} // namespace session
} // namespace ok
