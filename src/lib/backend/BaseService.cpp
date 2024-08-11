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

#include "BaseService.h"

#include <QObject>
#include <QString>

#include "base/r.h"
#include "lib/network/NetworkHttp.h"

#include <base/singleton.h>
#include <lib/session/AuthSession.h>
#include <algorithm>

namespace ok::backend {

BaseService::BaseService(const QString& baseUrl, QObject* parent)  //
        : QObject(parent), http(std::make_unique<network::NetworkHttp>(this)), _baseUrl(baseUrl) {
    if (!headers.isEmpty()) http->setHeaders(headers);
}

BaseService::~BaseService() {}

void BaseService::setHeader(QString k, QString v) { headers.insert(k, v); }

}  // namespace ok::backend
