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

//
// Created by gaojie on 24-8-3.
//

#include "Backend.h"
#include "base/OkSettings.h"
#include "lib/network/NetworkHttp.h"

namespace ok::platform {

Backend::Backend(const QString& baseUrl, const QString& authorization, QObject* parent)
        : ok::backend::BaseService(baseUrl, parent) {
    setHeader("Authorization", authorization);

    auto& s = ok::base::OkSettings::getInstance();
    QString trans = s.getTranslation();
    setHeader("Accept-Language", trans);
}

Backend::~Backend() { qWarning() << __func__; }

bool Backend::getAppList(const network::HttpBodyFn& fn, int pageIndex, int pageSize) {
    QString url = _baseUrl + "/api/work/app/page";
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("pageIndex", pageIndex);
    obj.insert("pageSize", pageSize);
    doc.setObject(obj);
    setHeader("Origin", _baseUrl);
    return http->postJson(QUrl(url), doc, fn, nullptr, nullptr, nullptr);
}

bool Backend::getInstance(const base::Fn<void(QJsonDocument)> fn,
                          const QString& appUuid,
                          const network::HttpErrorFn& err) {
    QString url = _baseUrl + "/api/tenant/user/" + appUuid + "/instance";
    return http->getJson(QUrl(url), fn, err);
}

}  // namespace ok::platform
