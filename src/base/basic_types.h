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

#include <QStringList>
#include <functional>
#include <string>
#include <utility>
namespace ok::base {
template <typename Signature> using Fn = std::function<Signature>;

template <typename Type> inline Type take(Type& value) { return std::exchange(value, Type{}); }

template <typename Type> inline Type duplicate(const Type& value) { return value; }

template <typename Type, size_t Size> inline constexpr size_t array_size(const Type (&)[Size]) {
    return Size;
}

#define qsl(s) QStringLiteral(s)
#define qstr(s) QLatin1String(s, sizeof(s) - 1)
#define qstring(s) QString::fromStdString(s)
#define stdstring(s) s.toStdString()

inline QStringList qstringlist(std::list<std::string> sl) {
    QStringList qsl;
    for (auto s : sl) {
        qsl.append(qstring(s));
    }
    return qsl;
}

#define ARRAY_LENGTH_OF(array) (sizeof(array) / sizeof(array[0]))
}
