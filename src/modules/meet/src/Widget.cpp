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
#include "StartMeetingWidget.h"
#include "JoinMeetingWidget.h"
#include "BookMeetingWidget.h"
#include "MeetingSettingWidget.h"

#include <QAbstractButton>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

namespace module::meet {

Widget::Widget(QWidget* parent) : UI::OMenuWidget(parent), ui(new Ui::WorkPlatform) {
    OK_RESOURCE_INIT(Meet);

    ui->setupUi(this);
    ui->tabWidget->setObjectName("mainTab");
    ui->tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);

    initTranslate();

    StartMeetingWidget* startMeet = new StartMeetingWidget(this);
    ui->tabWidget->addTab(startMeet, tr("Start Metting"));

    JoinMeetingWidget* joinMeet = new JoinMeetingWidget(this);
    ui->tabWidget->addTab(joinMeet, tr("Join Metting"));

    BookMeetingWidget* bookMeet = new BookMeetingWidget(this);
    ui->tabWidget->addTab(bookMeet, tr("Book Metting"));

    MeetingSettingWidget* setting = new MeetingSettingWidget(this);
    ui->tabWidget->addTab(setting, tr("Setting"));

    reloadTheme();
}

Widget::~Widget() { delete ui; }

void Widget::start() {}

void Widget::reloadTheme() {
    auto& style = Style::getStylesheet("general.css");
    setStyleSheet(style);
}

void Widget::doStart() {}

void Widget::initTranslate() {
    QString locale = ok::base::OkSettings::getInstance().getTranslation();
    settings::Translator::translate(OK_Meet_MODULE, locale);
    settings::Translator::registerHandler([this] { retranslateUi(); }, this);
    retranslateUi();
    connect(ok::Application::Instance()->bus(), &ok::Bus::languageChanged,
            [](QString locale0) { settings::Translator::translate(OK_Meet_MODULE, locale0); });
}

void Widget::retranslateUi() { ui->retranslateUi(this); }

}  // namespace module::meet