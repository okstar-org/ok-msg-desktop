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

#include "src/core/contactid.h"
#include <QByteArray>
#include <QHash>
#include <QString>
#include <QRegularExpression>
#include <cstdint>
#include <QDebug>
#include "base/StringUtils.h"
#include "lib/messenger/IMMessage.h"


/**
 * @brief The default constructor. Creates an empty id.
 */
ContactId::ContactId()  {}


ContactId::ContactId(const QByteArray &rawId)
    : ContactId(QString::fromUtf8(rawId)) {
}

ContactId::ContactId(const QString &strId) {
    // 正则表达式模式，这里假设username不包含@，server不包含/
      QRegularExpression re("([^@]+)@([^/]+)(/[^/]+)?");
      // 匹配输入字符串
      QRegularExpressionMatch match = re.match(strId);
      // 检查是否匹配成功
      if (!match.hasMatch()) {
          qWarning() << "Unable to parse contactId:"<<strId;
          return;
      }
          // 提取各个部分
          username = match.captured(1);
          server = match.captured(2);

}

ContactId::ContactId(const ContactId &contactId):
    username{contactId.username}, server{contactId.server}
{

}

/**
 * @brief Compares the equality of the ContactId.
 * @param other ContactId to compare.
 * @return True if both ContactId are equal, false otherwise.
 */
bool ContactId::operator==(const ContactId &other) const {
  return username == other.username && server == other.server;
}

/**
 * @brief Compares the inequality of the ContactId.
 * @param other ContactId to compare.
 * @return True if both ContactIds are not equal, false otherwise.
 */
bool ContactId::operator!=(const ContactId &other) const {
  return !(ContactId::operator==(other));
}

/**
 * @brief Compares two ContactIds
 * @param other ContactId to compare.
 * @return True if this ContactIds is less than the other ContactId, false
 * otherwise.
 */
bool ContactId::operator<(const ContactId &other) const {
  return username < other.username && server < other.server;
}

/**
 * @brief Get a copy of the id
 * @return Copied id bytes
 */
QByteArray ContactId::getByteArray() const {
  return toString().toUtf8();
}

/**
 * @brief Checks if the ContactId contains a id.
 * @return True if there is a id, False otherwise.
 */
bool ContactId::isEmpty() const { return username.isEmpty(); }

int ContactId::getSize()
{
    return toString().size();
}

