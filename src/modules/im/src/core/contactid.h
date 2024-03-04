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

#ifndef CONTACTID_H
#define CONTACTID_H

#include <QByteArray>
#include <QString>
#include <cstdint>
#include <QHash>
#include <memory>

#include "lib/messenger/messenger.h"

class ContactId : public lib::messenger::FriendId
{
public:
    virtual ~ContactId() = default;
    ContactId& operator=(const ContactId& other) = default;
    ContactId& operator=(ContactId&& other) = default;
    bool operator==(const ContactId& other) const;
    bool operator!=(const ContactId& other) const;
    bool operator<(const ContactId& other) const;

    QByteArray getByteArray() const;
    bool isEmpty() const;
    virtual int getSize() const = 0;

    QString getUsername() const;
    QString getServer() const;
//    QString getResource() const;

protected:
    ContactId();
    explicit ContactId(const lib::messenger::FriendId& rawId);
    explicit ContactId(const QByteArray &rawId);

};

inline uint qHash(const ContactId& id)
{
    return qHash(id.getByteArray());
}

using ContactIdPtr = std::shared_ptr<const ContactId>;

#endif // CONTACTID_H
