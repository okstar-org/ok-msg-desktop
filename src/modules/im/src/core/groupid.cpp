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

#include "groupid.h"

#include <QByteArray>
#include <QString>

#include <cassert>

/**
 * @class GroupId
 * @brief This class represents a long term persistent group identifier.
 */

/**
 * @brief The default constructor. Creates an empty Tox group ID.
 */
GroupId::GroupId() : ContactId() {}

/**
 * @brief The copy constructor.
 * @param other GroupId to copy
 */
GroupId::GroupId(const GroupId& other) : ContactId(other.toString().toUtf8()) {}

GroupId::GroupId(const QByteArray& rawId) : ContactId(rawId) {}

GroupId::GroupId(const QString& rawId) : ContactId(rawId) {}
/**
 * @brief Constructs a GroupId from bytes.
 * @param rawId The bytes to construct the GroupId from. The lenght must be exactly
 *              TOX_CONFERENCE_UID_SIZE, else the GroupId will be empty.
 */
GroupId::GroupId(const ContactId& contactId) : ContactId(contactId) {}

/**
 * @brief Get size of public id in bytes.
 * @return Size of public id in bytes.
 */
int GroupId::getSize() const { return toString().size(); }

bool GroupId::operator==(const GroupId& other) const { return ContactId::operator==(other); }

bool GroupId::operator<(const GroupId& other) const { return toString() < other.toString(); }
