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

#include "UserService.h"

#include <base/logs.h>
#include <QUrl>

namespace ok::backend {

using namespace network;

UserService::UserService(const QString& base, QObject* parent) : BaseService(base, parent) {}

UserService::~UserService() {}

void UserService::search(const QString& query, ok::base::Fn<void(const QList<OrgStaff*>&)> fn,
                         network::HttpErrorFn errFn) {
    QUrl url(QString(_baseUrl + "/api/open/staff/search?q=%1").arg(query));
    http->getJson(
            QUrl(url),
            // success
            [=](QJsonDocument doc) {
                ResList<OrgStaff> res(doc);
                fn(res.data);
            },
            errFn);
}

}  // namespace ok::backend
