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

#include "toxpk.h"

//#include <tox/tox.h>

#include <QByteArray>
#include <QString>

#include <cassert>

/**
 * @class ToxPk
 * @brief This class represents a Tox Public Key, which is a part of Ok ID.
 */

/**
 * @brief The default constructor. Creates an empty Tox key.
 */
ToxPk::ToxPk() : ContactId()
{
}

/**
 * @brief The copy constructor.
 * @param other ToxPk to copy
 */
ToxPk::ToxPk(const ToxPk& other)
    : ContactId(other.toString().toUtf8())
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from. The lenght must be exactly
 *              TOX_PUBLIC_KEY_SIZE, else the ToxPk will be empty.
 */
ToxPk::ToxPk(const QByteArray& rawId)
    : ContactId(rawId)
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from, will read exactly
 * TOX_PUBLIC_KEY_SIZE from the specified buffer.
 */
ToxPk::ToxPk(const lib::messenger::FriendId& rawId)
    : ContactId( rawId )
{
}

bool ToxPk::operator==(const ToxPk &other) const {
    return other.username.toLower() == username.toLower() ;//&& other.server == server;
}

bool ToxPk::operator<(const ToxPk &other) const {
  return username < other.username ;// && server < other.server;
}

/**
 * @brief Get size of public key in bytes.
 * @return Size of public key in bytes.
 */
int ToxPk::getSize() const
{
    return toString().size();
}
