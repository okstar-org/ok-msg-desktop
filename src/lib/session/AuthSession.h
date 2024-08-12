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

#include "lib/backend/PassportService.h"
#include "lib/messenger/messenger.h"

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
    int statusCode;
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
    AuthSession(QObject* parent = nullptr);
    ~AuthSession() override;

    Status status() const;

    void doLogin(const SignInInfo& signInInfo);

    [[nodiscard]] const SignInInfo& getSignInInfo() const { return m_signInInfo; };

    [[nodiscard]] const ok::backend::SysToken& getToken() const { return m_token; };

    [[nodiscard]] ok::base::OkAccount* account() const { return okAccount.get(); }

protected:
    // 执行登录
    void doSignIn();

protected slots:
    // 刷新token
    void doRefreshToken();

private:
    QMutex _mutex;
    SignInInfo m_signInInfo;
    ok::backend::SysToken m_token;

    std::unique_ptr<network::NetworkHttp> m_networkManager;
    std::unique_ptr<ok::base::OkAccount> okAccount;
    std::unique_ptr<ok::backend::PassportService> passportService;

    Status _status;

    void setToken(const ok::backend::SysToken& token);
    void setRefreshToken(const backend::SysRefreshToken& token);
signals:
    void loginResult(SignInInfo, LoginResult);
    void tokenSet();
    void refreshTokenSet(const ok::backend::SysRefreshToken& token);
};
}  // namespace session
}  // namespace ok
