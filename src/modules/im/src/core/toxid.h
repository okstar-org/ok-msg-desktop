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

#ifndef TOXID_H
#define TOXID_H

#include "FriendId.h"

#include <QByteArray>
#include <QString>
#include <cstdint>

class ToxId {
public:
    ToxId();
    ToxId(const ToxId& other);
    explicit ToxId(const QString& id);
    explicit ToxId(const QByteArray& rawId);
    explicit ToxId(const uint8_t* rawId, int len);
    ToxId& operator=(const ToxId& other) = default;
    ToxId& operator=(ToxId&& other) = default;

    bool operator==(const ToxId& other) const;
    bool operator!=(const ToxId& other) const;
    QString toString() const;
    void clear();
    bool isValid() const;

    static bool isValidToxId(const QString& id);
    static bool isToxId(const QString& id);
    const uint8_t* getBytes() const;
    QByteArray getToxId() const;
    FriendId getPublicKey() const;
    QString getNoSpamString() const;

    QString getToxIdAsStr() const;

private:
    void constructToxId(const QByteArray& rawId);

public:
    static const QRegularExpression ToxIdRegEx;

private:
    QByteArray toxId;
};

#endif  // TOXID_H
