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
// Created by gaojie on 23-9-22.
//

#include <QDomElement>

namespace base {
class Xmls {
public:
  inline static QDomElement parse(const QString &xmlData) {
    QDomDocument document;
    if (!document.setContent(xmlData, true)) {
      return QDomElement{};
    }
    return document.documentElement();
  }

  inline static QString format(QDomElement& element){
    if(element.isNull())
      return QString{};

    QTextStream stream;
    stream.setString(new QString());
    element.save(stream, 0);
    return *stream.string();
  }
};

} // namespace base
