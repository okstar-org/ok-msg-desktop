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

#include "OkCloudService.h"

#include <QObject>
#include "base/system/sys_info.h"

namespace ok::backend {

PluginInfo PluginInfo::fromJson(const QJsonObject &data) {

    return PluginInfo(data);
}


OkCloudService::OkCloudService(QObject *parent)
    : BaseService(BACKEND_CLOUD_URL, parent) {}

OkCloudService::~OkCloudService() {}

bool OkCloudService::GetFederalInfo(Fn<void(Res<FederalInfo> &)> fn, Fn<void(QString)> err) {
  QString url = _baseUrl + "/federal/.well-known/info.json";
  return http->getJSON(
      QUrl(url),
      // success
      [=](QJsonDocument doc) {
        Res<FederalInfo> res(doc);
        fn(res);
      },
      err);
}

bool OkCloudService::GetPluginPage(Fn<void(ResPage<ok::backend::PluginInfo> &)> fn, Fn<void(QString)> err) {

  auto osInfo = ::base::SystemInfo::instance()->osInfo();
  auto cpuInfo = ::base::SystemInfo::instance()->cpuInfo();
  auto platform = osInfo.kernelName;

  QString url = _baseUrl + "/plugin/page?platform="+platform+"&arch="+cpuInfo.arch;
  return http->getJSON(
      QUrl(url),
      // success
      [=](QJsonDocument doc) {
        ResPage<PluginInfo> res(doc);
        fn(res);
      },
      err);
}

} // namespace ok::backend
