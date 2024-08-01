/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

#include "AuthSession.h"

#include <list>
#include <memory>
#include <string>
#include <thread>

#include <QMutexLocker>
#include <QTimer>

#include "base/logs.h"
#include "lib/backend/PassportService.h"
#include "lib/messenger/IM.h"
#include "lib/network/NetworkHttp.h"

namespace ok {
namespace session {

using namespace network;
using namespace ok::backend;

AuthSession::AuthSession(QObject* parent)
        : QObject(parent)
        , m_networkManager(std::make_unique<network::NetworkHttp>(this))
        ,                      //
        _status(Status::NONE)  //
{
    qRegisterMetaType<SignInInfo>("SignInInfo");
    qRegisterMetaType<LoginResult>("LoginResult");
    qRegisterMetaType<::lib::messenger::IMConnectStatus>("IMConnectStatus");
    connect(this, &AuthSession::loginSuccessed, this, &AuthSession::onLoginSuccessed);
}

AuthSession::~AuthSession() { qDebug() << "~AuthSession"; }

AuthSession* AuthSession::Instance() {
    static AuthSession* self = nullptr;
    if (!self) {
        self = new AuthSession;
    }
    return self;
}

Status AuthSession::status() const { return _status; }

void AuthSession::onLoginSuccessed() {
    qDebug() << __func__ << "username" << m_signInInfo.username;

    okAccount = std::make_unique<ok::base::OkAccount>(m_signInInfo.username);
    okAccount->setJid(::base::Jid(m_signInInfo.username, m_signInInfo.host));

    _im = new ::lib::messenger::IM(
            m_signInInfo.host, m_signInInfo.username, m_signInInfo.password, l);

    connect(_im, &::lib::messenger::IM::connectResult, this, &AuthSession::onIMConnectStatus);
    qRegisterMetaType<ok::session::SignInInfo>("ok::session::SignInInfo");

    connect(_im, &::lib::messenger::IM::started, this, &AuthSession::onIMStarted);

    _im->start();
}

void AuthSession::onIMStarted() { emit imStarted(m_signInInfo); }

void AuthSession::doConnect() {
    qDebug() << __func__;

    _status = Status::CONNECTING;
    LoginResult result{_status, tr("...")};
    emit loginResult(m_signInInfo, result);

    passportService->getAccount(
            m_signInInfo.account,
            [&](Res<SysAccount>& res) {
                qDebug() << "Res.code:" << res.code;

                if (res.code != 0) {
                    _status = Status::FAILURE;
                    LoginResult result{_status, res.msg, 200};
                    emit loginResult(m_signInInfo, result);
                    return;
                }

                if (!res.success()) {
                    _status = Status::FAILURE;
                    LoginResult result{_status, res.msg, 200};
                    emit loginResult(m_signInInfo, result);
                    return;
                }

                qDebug() << "Res.data=>" << res.data->toString();
                m_signInInfo.username = res.data->username.toLower();
                emit loginSuccessed();
            },
            [&](int statusCode, const QString& msg) {
                _status = Status::FAILURE;
                LoginResult result{Status::FAILURE, msg, statusCode};
                emit loginResult(m_signInInfo, result);
            });
}

void AuthSession::doLogin(const SignInInfo& signInInfo) {
    QMutexLocker locker(&_mutex);

    m_signInInfo = signInInfo;

    if (_status == ok::session::Status::CONNECTING) {
        qDebug(("The connection is connecting."));
        return;
    }

    if (_status == ok::session::Status::SUCCESS) {
        qDebug(("The connection is connected."));
        return;
    }

    qDebug() << "account:" << signInInfo.account << "password:" << signInInfo.password;

    qDebug() << "stackUrl:" << signInInfo.stackUrl;
    passportService = std::make_unique<ok::backend::PassportService>(signInInfo.stackUrl);

    // 建立连接
    doConnect();
}

void AuthSession::onIMConnectStatus(::lib::messenger::IMConnectStatus status) {
    QString msg;
    if (status == ::lib::messenger::IMConnectStatus::CONNECTED) {
        _status = Status::SUCCESS;
        LoginResult result{Status::SUCCESS, msg};
        emit loginResult(m_signInInfo, result);
        return;
    }

    // 错误处理
    _status = Status::FAILURE;
    switch (status) {
        case ::lib::messenger::IMConnectStatus::NO_SUPPORT: {
            msg = tr("NO_SUPPORT");
            break;
        }
        case ::lib::messenger::IMConnectStatus::AUTH_FAILED: {
            msg = tr("AUTH_FAILED");
            break;
        }
        case ::lib::messenger::IMConnectStatus::DISCONNECTED: {
            msg = tr("DISCONNECTED");
            break;
        }
        case ::lib::messenger::IMConnectStatus::CONN_ERROR: {
            msg = tr("CONN_ERROR");
            break;
        }
        case ::lib::messenger::IMConnectStatus::CONNECTING: {
            msg = tr("...");
            break;
        }
        case ::lib::messenger::IMConnectStatus::TLS_ERROR: {
            msg = tr("TLS_ERROR");
            break;
        }
        case ::lib::messenger::IMConnectStatus::OUT_OF_RESOURCE: {
            msg = tr("OUT_OF_RESOURCE");
            break;
        }
        case ::lib::messenger::IMConnectStatus::TIMEOUT: {
            msg = tr("TIMEOUT");
            break;
        }
    }

    LoginResult result{Status::FAILURE, msg};
    emit loginResult(m_signInInfo, result);
}

}  // namespace session
}  // namespace ok
