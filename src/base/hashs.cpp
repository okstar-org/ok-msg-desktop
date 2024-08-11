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
// Created by gaojie on 23-9-12.
//

#include "hashs.h"
#include <QCryptographicHash>
#include <QString>
namespace ok::base {

QByteArray Hashs::hash(const QByteArray& buf, QCryptographicHash::Algorithm algorithm) {
    QCryptographicHash hash(algorithm);
    hash.addData(buf);
    return hash.result();
}

QByteArray Hashs::sha1(const QByteArray& buf) {
    return hash(buf, QCryptographicHash::Algorithm::Md5);
}

QString Hashs::sha1String(const QByteArray& buf) { return QString{sha1(buf).toHex()}; }

}  // namespace ok::base
