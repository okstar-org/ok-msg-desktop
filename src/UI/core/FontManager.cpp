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

FontManager::FontManager(QObject* parent) : QObject(parent) {}

FontManager::~FontManager() {}

void FontManager::loadFonts() {
    // QFontDatabase::addApplicationFont(":/font/resources/fonts/PingFang/PingFang_SC_Regular.ttf");
    // QFontDatabase::addApplicationFont(":/font/resources/fonts/PingFang/PingFang_SC_Light.ttf");
    /*QFontDatabase::addApplicationFont(":/font/resources/fonts/PingFang/PingFang_SC_Medium.ttf");
    QFontDatabase::addApplicationFont(":/font/resources/fonts/PingFang/PingFang_SC_Thin.ttf");
    QFontDatabase::addApplicationFont(":/font/resources/fonts/PingFang/PingFang_SC_Semibold.ttf");
    QFontDatabase::addApplicationFont(":/font/resources/fonts/PingFang/PingFang_SC_UltraLight.ttf");*/

    //	QFont f("Microsoft Yahei", 10, QFont::Normal);
    //
    //    qApp->setFont(f);
}
