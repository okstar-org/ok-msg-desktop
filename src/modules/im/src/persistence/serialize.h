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

#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <QByteArray>
#include <QString>
#include <cstdint>

QString dataToString(QByteArray data);
uint64_t dataToUint64(const QByteArray& data);
int dataToVInt(const QByteArray& data);
size_t dataToVUint(const QByteArray& data);
unsigned getVUint32Size(QByteArray data);
QByteArray vintToData(int num);
QByteArray vuintToData(size_t num);

#endif  // SERIALIZE_H
