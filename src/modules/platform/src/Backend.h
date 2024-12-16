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

/**
 * 应用实体
 */
struct App {
    // 应用UUID
    QString uuid;
    QString key;
    QString name;
    QString avatar;
    QString descr;
    QString author;
    QString mail;
    QString homePage;
    QString type;

    App(const QJsonObject& data) {
        uuid = data.value("uuid").toString();          //
        mail = data.value("mail").toString();          //
        name = data.value("name").toString();          //
        descr = data.value("descr").toString();        //
        author = data.value("author").toString();      //
        homePage = data.value("homePage").toString();  //
        type = data.value("type").toString();          //
    }

    QJsonObject toJson() {
        QJsonObject jo;
        jo.insert("uuid", uuid);
        jo.insert("key", key);
        jo.insert("name", name);
        jo.insert("author", author);
        return jo;
    }

    QString toJsonString() { return QJsonDocument(toJson()).toJson(); }
};

/**
 * 应用实例实体
 */
struct InstanceDTO {
    QString uuid;

    /**
     * 创建时间
     */
    QDate createAt;

    /**
     * 更新时间
     */
    QDate updateAt;

    /**
     * 编号
     */
    QString no;
    /**
     * 应用ID
     */
    QString appUuid;

    /**
     * 实例名称=租户名称+应用名称+订单名词
     */
    QString name;

    /**
     * 实例描述
     */
    QString description;

    /**
     * 状态
     */
    QString status;

    QList<QString> urls;

    QList<QString> volumes;
};

class Backend : public lib::backend::BaseService {
    Q_OBJECT
public:
    explicit Backend(const QString& baseUrl, const QString& authorization = "",
                     QObject* parent = nullptr);

    ~Backend() override;
    bool getAppList(const lib::network::HttpBodyFn& fn, int res = 0, int pageSize = 50);

    bool getInstance(const base::Fn<void(QJsonDocument)>& fn, const QString& appUuid,
                     const lib::network::HttpErrorFn& err);
};

}  // namespace ok::platform
