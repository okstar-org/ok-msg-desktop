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

#ifndef TOXPK_H
#define TOXPK_H

#include "src/core/contactid.h"
#include <QByteArray>
#include <cstdint>

class ToxPk : public ContactId
{
public:
    ToxPk();
    ToxPk(const ToxPk& other);
    explicit ToxPk(const QByteArray& rawId);
    explicit ToxPk(const lib::messenger::FriendId& rawId);

    bool operator==(const ToxPk& other) const;
    bool operator<(const ToxPk& other) const;

    int getSize() const override;
};


inline uint qHash(const ToxPk& id)
{
    uint hash = qHash(id.getByteArray());
    return hash;
}



#endif // TOXPK_H
