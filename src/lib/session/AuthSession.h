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

namespace lib::session {

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
    explicit AuthSession(QObject* parent = nullptr);
    ~AuthSession() override;

    [[nodiscard]] Status status() const;

    void doLogin(const SignInInfo& signInInfo);

    [[nodiscard]] const SignInInfo& getSignInInfo() const { return m_signInInfo; };

    [[nodiscard]] const lib::backend::SysToken& getToken() const {
        return m_token;
    };

    [[nodiscard]] ok::base::OkAccount* account() const { return okAccount.get(); }

    [[nodiscard]] const QString& getStackUrl() const { return m_signInInfo.stackUrl; }

protected:
    // 执行登录
    void doSignIn();

protected slots:
    // 刷新token
    void doRefreshToken();

private:
    QMutex _mutex;
    SignInInfo m_signInInfo;
    lib::backend::SysToken m_token;

    std::unique_ptr<lib::network::NetworkHttp> m_networkManager;
    std::unique_ptr<ok::base::OkAccount> okAccount;
    std::unique_ptr<lib::backend::PassportService> passportService;

    Status _status;

    void setToken(const lib::backend::SysToken& token);
    void setRefreshToken(const backend::SysRefreshToken& token);
signals:
    void loginResult(SignInInfo, LoginResult);
    void tokenSet();
    void refreshTokenSet(const lib::backend::SysRefreshToken& token);
};
}  // namespace lib::session
