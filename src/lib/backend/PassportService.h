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

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <memory>
#include <string>

#include <base/basic_types.h>

#include <lib/backend/BaseService.h>

namespace ok::backend {

struct SysRefreshToken {
    QString accessToken;
    quint64 expiresIn;  // 有效期（s）
    QString refreshToken;
    quint64 refreshExpiresIn;

    SysRefreshToken() : expiresIn{0}, refreshExpiresIn{0} {}

    SysRefreshToken(const QJsonObject& data) {
        expiresIn = data.value("expiresIn").toVariant().toULongLong();                //
        refreshExpiresIn = data.value("refreshExpiresIn").toVariant().toULongLong();  //
        refreshToken = data.value("refreshToken").toString();                         //
        accessToken = data.value("accessToken").toString();                           //
    }
};

struct SysToken : public SysRefreshToken {
    /**
     * "tokenType": "string",
  "accessToken": "string",
  "expiresIn": 0,
  "refreshToken": "string",
  "refreshExpiresIn": 0,
  "session_state": "string"
     */
    QString username;
    QString tokenType;
    QString session_state;
    SysToken() : SysRefreshToken() {}
    SysToken(const QJsonObject& data) {
        expiresIn = data.value("expiresIn").toVariant().toULongLong();                //
        refreshExpiresIn = data.value("refreshExpiresIn").toVariant().toULongLong();  //
        username = data.value("username").toString();
        tokenType = data.value("tokenType").toString();          //
        refreshToken = data.value("refreshToken").toString();    //
        session_state = data.value("session_state").toString();  //
        accessToken = data.value("accessToken").toString();      //
    }
};

struct SysAccount {
    /**
     * {
     * "id":2001,
     * "iso":"CN",
     * "username":"On1qv1AfQmap",
     * "nickname":"OkStar",
     * "firstName":"",
     * "lastName":"",
     * "no":null,
     * "avatar":"/assets/images/avatar.jpg",
     * "name":"OkStar"
     * }
     */
    quint64 id;
    QString iso;
    QString name;
    QString username;
    QString nickname;

    SysAccount(const QJsonObject& data) {
        id = data.value("id").toVariant().toULongLong();  //
        iso = data.value("iso").toString();               //
        name = data.value("name").toString();             //
        username = data.value("username").toString();     //
        nickname = data.value("nickname").toString();     //
    }

    QString toString() {
        return QString("{id:%1, username:%2, name:%3, nickname:%4 iso:%5}")  //
                .arg(QString::number(id), username, name, nickname, iso);
    }
};

class PassportService : BaseService {
    Q_OBJECT
public:
    PassportService(const QString& base, QObject* parent = nullptr);
    ~PassportService();

    /**
     * 登录
     * @param account 帐号（邮箱、手机号）
     * @param password 密码
     * @param rememberMe
     * @param grantType
     * @return
     */
    bool signIn(const QString& account, const QString& password, ok::base::Fn<void(Res<SysToken>&)> fn,
                const network::HttpErrorFn& err, bool rememberMe = false,
                const QString& grantType = "password");

    bool refresh(const SysToken& token, ok::base::Fn<void(Res<SysRefreshToken>&)> fn,
                 network::HttpErrorFn err = nullptr);
};
}  // namespace ok::backend
