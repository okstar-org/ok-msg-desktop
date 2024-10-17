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
#include "FontManager.h"
#include <QObject>
#include <QtGui/QFontDatabase>
#include <QtWidgets/QtWidgets>

FontManager::FontManager(QObject* parent) : QObject(parent) {
    //    loadFonts("PingFangSC", QStringList{"Light", "Medium", "Regular", "Semibold", "Thin",
    //    "Ultralight"});
}

FontManager::~FontManager() {}

void FontManager::loadFonts(const QString& font, const QStringList& styles) {
    for (const auto& item : styles) {
        QFontDatabase::addApplicationFont(
                QString(":/resources/fonts/%1-%2.ttf").arg(font).arg(item));
    }
}

void FontManager::useFont(const QString& font, const QString& style) {
    QFont f(font + "-" + style, 10, 16);
    useFont(f);
}

void FontManager::useFont(const QFont& f) { qApp->setFont(f); }

void FontManager::useFontSize(int size) {
    auto f = qApp->font();
    f.setWeight(size);
    qApp->setFont(f);
}
