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

#include "contentlayout.h"
#include <QFrame>
#include <QStyleFactory>
#include "src/lib/settings/style.h"
#include "src/persistence/settings.h"

ContentLayout::ContentLayout() : QStackedLayout() { init(); }

ContentLayout::ContentLayout(QWidget* parent) : QStackedLayout(parent) {
    init();

    //    QPalette palette = parent->palette();
    //    palette.setBrush(QPalette::WindowText, QColor(0, 0, 0));
    //    palette.setBrush(QPalette::Button, QColor(255, 255, 255));
    //    palette.setBrush(QPalette::Light, QColor(255, 255, 255));
    //    palette.setBrush(QPalette::Midlight, QColor(255, 255, 255));
    //    palette.setBrush(QPalette::Dark, QColor(127, 127, 127));
    //    palette.setBrush(QPalette::Mid, QColor(170, 170, 170));
    //    palette.setBrush(QPalette::Text, QColor(0, 0, 0));
    //    palette.setBrush(QPalette::BrightText, QColor(255, 255, 255));
    //    palette.setBrush(QPalette::ButtonText, QColor(0, 0, 0));
    //    palette.setBrush(QPalette::Base, QColor(255, 255, 255));
    //    palette.setBrush(QPalette::Window, QColor(255, 255, 255));
    //    palette.setBrush(QPalette::Shadow, QColor(0, 0, 0));
    //    palette.setBrush(QPalette::AlternateBase, QColor(255, 255, 255));
    //    palette.setBrush(QPalette::ToolTipBase, QColor(255, 255, 220));
    //    palette.setBrush(QPalette::ToolTipText, QColor(0, 0, 0));

    //    palette.setBrush(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    //    palette.setBrush(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    //    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));

    ////    parent->setPalette(palette);
}

ContentLayout::~ContentLayout() {
    clear();

    //    mainHead->deleteLater();
    //    mainContent->deleteLater();
}

void ContentLayout::reloadTheme() {
#ifndef Q_OS_MAC
//    mainHead->setStyleSheet(Style::getStylesheet("settings/mainHead.css"));
//    mainContent->setStyleSheet(Style::getStylesheet("window/general.css"));
#endif
}

void ContentLayout::clear() {
    //    QLayoutItem* item;
    //    while ((item = mainHead->layout()->takeAt(0)) != nullptr) {
    //        item->widget()->hide();
    //        item->widget()->setParent(nullptr);
    //        delete item;
    //    }
    //
    //    while ((item = mainContent->layout()->takeAt(0)) != nullptr) {
    //        item->widget()->hide();
    //        item->widget()->setParent(nullptr);
    //        delete item;
    //    }
}

void ContentLayout::init() {}
