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
// Created by gaojie on 24-5-28.
//

#ifndef OKMSG_PROJECT_IMGROUP_H
#define OKMSG_PROJECT_IMGROUP_H

#include <QList>
#include <QString>

namespace lib::messenger {

/**
 * 群聊
 */
struct IMGroup {
    QString name;
    QString description;
    QString subject;
    QString creationdate;
    uint64_t occupants = 0;
};

/**
 * 群聊名称
 */
struct IMGroupOccupant {
    QString jid;
    QString nick;
    QString affiliation;
    QString role;
    int status;
    // https://xmpp.org/registrar/mucstatus.html
    QList<int> codes;
};

}  // namespace lib::messenger

#endif  // OKMSG_PROJECT_IMGROUP_H
