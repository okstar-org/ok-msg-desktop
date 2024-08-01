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

#ifndef GROUPID_H
#define GROUPID_H

#include "src/core/contactid.h"

class GroupId : public ContactId {
public:
    GroupId();
    GroupId(const GroupId& other);
    explicit GroupId(const QByteArray& rawId);
    explicit GroupId(const QString& rawId);
    explicit GroupId(const ContactId& contactId);
    int getSize() const;

    bool operator==(const GroupId& other) const;
    bool operator<(const GroupId& other) const;

    QString name;
    QString nick;
};

#endif  // GROUPID_H
