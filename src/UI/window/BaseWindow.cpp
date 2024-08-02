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

#include "BaseWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QStyleOption>

#include <base/utils.h>

BaseWindow::BaseWindow(QWidget* parent) : QWidget(parent) {
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    // setAttribute(Qt::WA_TranslucentBackground);
}

BaseWindow::~BaseWindow() {}

void BaseWindow::paintEvent(QPaintEvent* event) { return QWidget::paintEvent(event); }
