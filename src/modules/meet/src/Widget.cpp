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
// Created by gaojie on 24-7-31.
//

#include "Widget.h"
#include "ui_Widget.h"

#include <QTabBar>
#include "Bus.h"
#include "application.h"
#include "base/OkSettings.h"
#include "lib/settings/style.h"
#include "lib/settings/translator.h"

#include <QAbstractButton>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

class TabCloseButton : public QAbstractButton {
public:
    explicit TabCloseButton(QWidget* parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

namespace module::meet {

Widget::Widget(QWidget* parent) : UI::OMenuWidget(parent), ui(new Ui::WorkPlatform) {
    OK_RESOURCE_INIT(Platform);
    OK_RESOURCE_INIT(PlatformRes);

    ui->setupUi(this);
    ui->tabWidget->setObjectName("mainTab");
    ui->tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);
    reloadTheme();
}

Widget::~Widget() { delete ui; }

void Widget::start() {}

void Widget::reloadTheme() {
    auto& style = Style::getStylesheet("general.css");
    setStyleSheet(style);
}

PlatformPage* Widget::findPage(const QUrl& url) {}

void Widget::addPage(PlatformPage* page, bool active) {}

bool Widget::removePage(PlatformPage* page) { return false; }

void Widget::activePage(PlatformPage* page) {}

void Widget::doStart() {}

void Widget::retranslateUi() { ui->retranslateUi(this); }

void Widget::requestCloseTab() {}

void Widget::doClose(int index, PlatformPage* page) {}

}  // namespace module::meet

TabCloseButton::TabCloseButton(QWidget* parent) : QAbstractButton(parent) {}

QSize TabCloseButton::sizeHint() const {
    ensurePolished();
    int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, nullptr, this);
    int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, nullptr, this);
    return QSize(width, height);
}

void TabCloseButton::enterEvent(QEvent* event) {
    if (isEnabled()) {
        update();
    }
    QAbstractButton::enterEvent(event);
}

void TabCloseButton::leaveEvent(QEvent* event) {
    if (isEnabled()) {
        update();
    }
    QAbstractButton::leaveEvent(event);
}

void TabCloseButton::paintEvent(QPaintEvent*) {}
