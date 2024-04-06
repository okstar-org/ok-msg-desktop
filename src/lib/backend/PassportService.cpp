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

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace ok::backend {

using namespace session;

PassportService::PassportService(const QString& base, QObject *parent)
    : BaseService(base, parent) {}

PassportService::~PassportService() {}

bool PassportService::getAccount(const QString &account,
                                 Fn<void(Res<SysAccount> &)> fn,
                                 Fn<void(const QString &)> err) {
  QString url = _baseUrl + "/api/open/passport/account/" + account;
  http->getJSON(
      QUrl(url),
      // success
      [=](QJsonDocument doc) {
        Res<SysAccount> res(doc);
        fn(res);
      },
      // error
      [=](QString msg) {
        err(msg);
      });
  return true;
}

} // namespace ok::backend
