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
#pragma once

#include <base/jsons.h>
#include <QObject>
#include "lib/backend/BaseService.h"

namespace ok::platform {

struct App {
    /**
     *   {"id":51,
     *   "key":"企业IM",
     *   "name":"OkMSG商业套装",
     *   "avatar":"https://s3.okstar.org.cn/okcloud/fdc636f5-955c-47a8-bca1-253cfbeb35c6.png",
     *   "descr":"OkMSG
是由OkStar社区维护的跨平台的企业通讯协同工具，支持独立私有化部署的集即时消息、语音和视频通话、发送文件、会议等多种功能于一身的开源项目。同时非常注重数据安全与保护，
  让您的企业更加有效开启协作、有效沟通，控制成本，开拓新业务，并帮助您加速发展业务。",
"author":null,
"mail":null,
"homePage":null,
"providerId":1,
"introduceId":null},
  */

    int64 id;
    QString key;
    QString name;
    QString avatar;
    QString descr;
    QString author;
    QString mail;
    QString homePage;
    QString type;

    App(const QJsonObject& data) {
        id = data.value("id").toInt();                 //
        mail = data.value("mail").toString();          //
        name = data.value("name").toString();          //
        descr = data.value("descr").toString();        //
        author = data.value("author").toString();      //
        homePage = data.value("homePage").toString();  //
        type = data.value("type").toString();          //
    }

    QJsonObject toJson() {
        QJsonObject jo;
        jo.insert("id", id);
        jo.insert("key", key);
        jo.insert("name", name);
        jo.insert("author", author);
        return jo;
    }

    QString toJsonString() { return QString(QJsonDocument(toJson()).toJson()); }
};

class Backend : public ok::backend::BaseService {
    Q_OBJECT
public:
    Backend(const QString& baseUrl, QObject* parent = nullptr);

    bool getAppList(const network::HttpBodyFn& fn, int res = 0, int pageSize = 50);
};

}  // namespace ok::platform
