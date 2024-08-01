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

#ifndef TOXSTRING_H
#define TOXSTRING_H

#include <QByteArray>
#include <QString>

#include <cstdint>

class ToxString {
public:
    explicit ToxString(const QString& text);
    explicit ToxString(const QByteArray& text);
    ToxString(const uint8_t* text, size_t length);

    const uint8_t* data() const;
    size_t size() const;
    QString getQString() const;
    QByteArray getBytes() const;

private:
    QByteArray string;
};
#endif  // TOXSTRING_H
