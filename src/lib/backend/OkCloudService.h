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

#include <QObject>

#include <lib/backend/BaseService.h>
#include "base/jsons.h"

namespace ok::backend {

class State {
public:
    QString no;
    QString name;
    QString xmppHost;
    QString stackUrl;
};

class FederalInfo {
public:
    FederalInfo(const QJsonObject& data) {
        /**
         * {
    "states": [
      {
        "no": "1001",
        "name": "OkStar开源社区",
        "xmppHost": "meet.okstar.org.cn"
      },
      {
        "no": "1002",
        "name": "船山信息",
        "xmppHost": "meet.chuanshaninfo.com"
      }
    ]
    }
         */

        QJsonArray arr = data.value("states").toArray();
        for (auto&& item : arr) {
            const QJsonObject& object = item.toObject();
            State state;
            state.no = object.value("no").toString();
            state.name = object.value("name").toString();
            state.xmppHost = object.value("xmppHost").toString();
            state.stackUrl = object.value("stackUrl").toString();
            states.push_back(state);
        }
    }
    QList<State> states;
};

class PluginInfo : public ok::base::JsonAble<PluginInfo> {
public:
    quint64 id;         // 插件id
    quint64 ver;        // 数字版本
    QString shortName;  // 标识符
    QString name;       // 版本名称
    QString fileName;   // 文件名称
    QString content;    // 版本说明
    QString version;    // 可读版本
    QString author;     //
    QString homeUrl;    // 主页地址
    QString logoUrl;    // logo
    QString downUrl;    // 下载地址

    PluginInfo(const QJsonObject& obj) {
        id = obj.value(("id")).toInt();
        ver = obj.value(("ver")).toInt();
        shortName = obj.value(("shortName")).toString();
        name = obj.value(("name")).toString();
        version = obj.value(("version")).toString();
        fileName = obj.value(("fileName")).toString();
        content = obj.value(("content")).toString();
        author = obj.value(("author")).toString();
        homeUrl = obj.value(("homeUrl")).toString();
        logoUrl = obj.value(("logoUrl")).toString();
        downUrl = obj.value(("downUrl")).toString();
    }

    PluginInfo fromJson(const QJsonObject& data) override;
};

class OkCloudService : public BaseService {
    Q_OBJECT
public:
    OkCloudService(QObject* parent = nullptr);
    ~OkCloudService();

    bool GetFederalInfo(ok::base::Fn<void(Res<FederalInfo>&)> fn, network::HttpErrorFn err = nullptr);
    bool GetPluginPage(ok::base::Fn<void(ResPage<PluginInfo>&)> fn, network::HttpErrorFn err = nullptr);
};
}  // namespace ok::backend
