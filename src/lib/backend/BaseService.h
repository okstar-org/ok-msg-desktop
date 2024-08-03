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

#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <string>

#include "BaseService.h"
#include "base/r.h"
#include "lib/network/NetworkHttp.h"

#include <QJsonArray>
#include <QList>

namespace ok::backend {

typedef struct {
    bool success;
    QString url;
    QString key;
    QString contentType;
    QString extension;
    QString name;
} FileResult;

template <typename T>  //
class Res {
public:
    int code;
    QString msg;
    T* data;
    QMap<QString, QJsonValue> extra;

    Res(const QJsonDocument& doc) : code{-1}, data{nullptr} {
        if (doc.isEmpty()) {
            code = -1;
            return;
        }

        auto obj = doc.object();
        if (obj.contains("code")) {
            code = obj.value(("code")).toInt();
            msg = obj.value("msg").toString();
            if (code == 0) {
                data = new T(obj.value("data").toObject());
                auto extra = obj.value("extra").toObject();
                for (auto key : extra.keys()) {
                    extra.insert(key, extra.value(key));
                }
            }
        } else {
            data = new T(obj);
        }
    }

    Res(const QString& err) {
        code = -1;
        msg = err;
    }

    ~Res() {
        if (code == 0) delete data;
    }

    bool success() const { return code == 0; }
};

template <typename T>  //
class ResList {
public:
    int code;
    QString msg;
    QList<T*> data;

    ResList(const QJsonDocument& doc) : code{-1} {
        if (doc.isEmpty()) {
            code = -1;
            return;
        }

        auto obj = doc.object();
        code = obj.value(("code")).toInt();
        msg = obj.value("msg").toString();
        if (code == 0) {
            QJsonArray arr = obj.value("data").toArray();
            for (auto i : arr) {
                data.append(new T(i.toObject()));
            }
        }
    }

    ResList(const QString& err) {
        code = -1;
        msg = err;
    }

    ~ResList() {
        if (code == 0) {
            //      qDeleteAll(data.begin(), data.end());
            data.clear();
        }
    }

    bool success() const { return code == 0; }
};

template <typename T>  //
class Page {
public:
    int totalCount;
    int pageCount;
    QList<T> list;

    Page() : totalCount(0), pageCount(0) {}

    Page(const QJsonObject& obj) {
        totalCount = obj.value(("totalCount")).toInt();
        pageCount = obj.value(("pageCount")).toInt();
        QJsonArray arr = obj.value("list").toArray();
        for (auto i : arr) {
            list.append(T(i.toObject()));
        }
    }
};

template <typename T>  //
class ResPage {
public:
    int code;
    QString msg;
    Page<T> data;

    ResPage() : code{-1} {}

    ResPage(const QJsonDocument& doc) : code{-1} {
        if (doc.isEmpty()) {
            code = -1;
            return;
        }

        auto obj = doc.object();
        code = obj.value(("code")).toInt();
        msg = obj.value("msg").toString();
        if (code == 0 && obj.contains("data")) {
            data = Page<T>(obj.value("data").toObject());
        }
    }

    ResPage(const QString& err) {
        code = -1;
        msg = err;
    }

    ~ResPage() {}

    bool success() const { return code == 0; }
};

class BaseService : public QObject {
    Q_OBJECT
public:
    BaseService(const QString& baseUrl, QObject* parent = nullptr);
    ~BaseService();

    [[maybe_unused]] inline const QString& baseUrl() const { return _baseUrl; }
    void setHeader(QString k, QString v);
protected:
    std::unique_ptr<network::NetworkHttp> http;
    QString _baseUrl;
    QMap<QString, QString> headers;
};

}  // namespace ok::backend
