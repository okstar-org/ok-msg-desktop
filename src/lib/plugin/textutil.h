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

#ifndef TEXTUTIL_H
#define TEXTUTIL_H

#include <QtGlobal>

class QString;
class QStringRef;

namespace TextUtil {

QString escape(const QString& plain);
QString unescape(const QString& escaped);

QString quote(const QString&, int width = 60, bool quoteEmpty = false);
QString plain2rich(const QString&);
QString rich2plain(const QString&, bool collapseSpaces = true);
QString resolveEntities(const QStringRef&);
QString linkify(const QString&);
QString legacyFormat(const QString&);
QString emoticonify(const QString& in);
QString img2title(const QString& in);

QString prepareMessageText(const QString& text, bool isEmote = false, bool isHtml = false);

QString sizeUnit(qlonglong n, qlonglong* div = nullptr);
QString roundedNumber(qlonglong n, qlonglong div);

}  // namespace TextUtil

#endif  // TEXTUTIL_H
