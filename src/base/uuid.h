/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

//
// Created by gaojie on 24-7-30.
//

#ifndef BASE_UUID_H
#define BASE_UUID_H

#include <QString>
#include <QUuid>
namespace ok::base {
class UUID {
public:
    inline static QString make() {
        return QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    }
};

}  // namespace ok::base
#endif  // MD5_H
