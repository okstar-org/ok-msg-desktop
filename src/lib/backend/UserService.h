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

#include <QJsonObject>
#include <QObject>
#include <QString>

#include <base/basic_types.h>

#include <lib/backend/BaseService.h>
#include <lib/backend/domain/AuthInfo.h>
#include <lib/session/AuthSession.h>

namespace ok::backend {

struct OrgStaff {
    QString no;
    QString name;
    QString phone;
    QString email;
    QString username;
    QString host;
    QString posts;

    OrgStaff(const QJsonObject& data) {
        no = data.value("no").toString();              //
        email = data.value("email").toString();        //
        name = data.value("name").toString();          //
        username = data.value("username").toString();  //
        phone = data.value("phone").toString();        //
        host = data.value("host").toString();          //
        posts = data.value("posts").toString();        //
    }

    QString toString() {
        return QString("{no:%1, username:%2, name:%3, phone:%4, email:%5, host:%6, posts: %7}")  //
                .arg(no)
                .arg(username)
                .arg(name)
                .arg(phone)
                .arg(host)
                .arg(posts);
    }

    QString toContactId() { return QString("%1@%2").arg(username).arg(host); }
};

class UserService : public BaseService {
    Q_OBJECT
public:
    UserService(const QString& base, QObject* parent = nullptr);
    ~UserService();

    void search(const QString& query,
                ok::base::Fn<void(const QList<OrgStaff*>&)>
                        callback,
                network::HttpErrorFn errFn = nullptr);
};

}  // namespace ok::backend
