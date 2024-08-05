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

#include "PassportService.h"
#include <base/jsons.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include "base/times.h"

namespace ok::backend {

PassportService::PassportService(const QString& base, QObject* parent)
        : BaseService(base, parent) {}

PassportService::~PassportService() {}

bool PassportService::signIn(const QString& account, const QString& password,
                             Fn<void(Res<SysToken>&)> fn, const network::HttpErrorFn& err,
                             bool rememberMe, const QString& grantType) {
    QString url = _baseUrl + "/api/auth/passport/signIn";
    QJsonObject data;
    /**
     * "ts": 0,
      "iso": "string",
      "grantType": "string",
      "account": "string",
      "password": "string",
      "rememberMe": true
     */
    data.insert("ts", ::base::Times::now().toMSecsSinceEpoch());
    data.insert("grantType", grantType);
    data.insert("account", account);
    data.insert("password", password);
    data.insert("rememberMe", rememberMe ? "true" : "false");

    return http->postJson(
            QUrl(url), QJsonDocument(data),
            [=](QByteArray doc, QString name) {
                Res<SysToken> res(Jsons::toJSON(doc));
                fn(res);
            },
            nullptr, nullptr, err);
}

bool PassportService::refresh(const SysToken& token, Fn<void(Res<SysRefreshToken>&)> fn,
                              network::HttpErrorFn err) {
    QJsonObject data;
    data.insert("ts", ::base::Times::now().toMSecsSinceEpoch());
    data.insert("accessToken", token.accessToken);
    data.insert("refreshToken", token.refreshToken);

    QString url = _baseUrl + "/api/auth/passport/refresh";
    return http->postJson(
            QUrl(url), QJsonDocument(data),
            [=](QByteArray doc, QString name) {
                Res<SysRefreshToken> res(Jsons::toJSON(doc));
                fn(res);
            },
            nullptr, nullptr, err);
}

}  // namespace ok::backend
