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

#include <base/jsons.h>
#include <QJsonObject>
#include <QObject>

namespace ok::backend {

class RoomInfo {
public:
    QString getJid() { return jid; }
    QString getName() { return name; }
    QString getPassword() { return password; }
    QString getSn() { return sn; }

    inline void setJid(QString& _jid) { jid = _jid; }

    inline void setName(QString& _name) { name = _name; }

    inline void setPassword(QString& _password) { password = _password; }

private:
    QString jid;
    QString name;
    QString password;
    QString sn;
};

}  // namespace ok::backend
