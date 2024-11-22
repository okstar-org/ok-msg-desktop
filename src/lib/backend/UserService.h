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
#include <lib/session/AuthSession.h>

namespace lib::backend {

struct SysProfile {
    /**
     * "accountId": 1,
    "firstName": "高（真）",
    "lastName": "杰（真）",
    "gender": "male",
    "identify": "430421198903140000",
    "birthday": "2024-08-29T09:04:31.035+00:00",
    "email": "okstar@gmail.com",
    "phone": "18910221510",
    "telephone": "18510248810",
    "country": "China",
    "province": null,
    "city": "Beijing",
    "address": "ChaoYang",
    "website": "okstar.org",
    "language": null,
    "description": null
     */
    QString firstName;
    QString lastName;
    QString gender;
    QString email;
    QString phone;
    QString telephone;

    inline QString getName() const { return firstName + lastName; }
};

struct OrgStaff {
    /**
     * "createAt": "2024-08-28T07:47:55.986+00:00",
        "updateAt": null,
        "accountId": 1,
        "username": "SJdVr4Swzf2f",
        "no": null,
        "joinedDate": "2024-08-28T07:47:55.986+00:00",
        "leftDate": null,
        "postStatus": "pending",
        "posts": [],
        "postNames": ""
     */
    quint64 accountId;
    QString no;
    QString username;
    QString nickname;
    QString postNames;
    QDate createAt;
    QDate updateAt;
    SysProfile profile;

    OrgStaff(const QJsonObject& data) {
        no = data.value("no").toString();                               //
        accountId = data.value("accountId").toVariant().toULongLong();  //
        username = data.value("username").toString();                   //
        nickname = data.value("nickname").toString();                   //
        postNames = data.value("postNames").toString();                 //
        createAt = data.value("createAt").toVariant().toDate();         //
        updateAt = data.value("updateAt").toVariant().toDate();         //

        const QJsonObject& profile_ = data.value("profile").toObject();
        profile.email = profile_.value("email").toString();          //
        profile.firstName = profile_.value("firstName").toString();  //
        profile.lastName = profile_.value("lastName").toString();    //
        profile.phone = profile_.value("phone").toString();          //
        profile.telephone = profile_.value("telephone").toString();  //
    }

    QString toContactId(const QString& host) { return QString("%1@%2").arg(username).arg(host); }
};

class UserService : public BaseService {
    Q_OBJECT
public:
    UserService(const QString& base, QObject* parent = nullptr);
    ~UserService();

    void search(const QString& query, ok::base::Fn<void(const QList<OrgStaff*>&)> callback,
                network::HttpErrorFn errFn = nullptr);
};

}  // namespace lib::backend
