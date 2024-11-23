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
#include "BookMeetingWidget.h"
#include "Bus.h"
#include "JoinMeetingWidget.h"
#include "MeetingSettingWidget.h"
#include "StartMeetingWidget.h"
#include "application.h"
#include "base/OkSettings.h"
#include "lib/settings/style.h"
#include "lib/settings/translator.h"
#include "meetingview/MeetingVideoFrame.h"

#include <QAbstractButton>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

#include <QContextMenuEvent>
#include <QMenu>

namespace module::meet {

Widget::Widget(QWidget* parent) : UI::OMenuWidget(parent), ui(new Ui::WorkPlatform), view{nullptr} {
    OK_RESOURCE_INIT(Meet);
    OK_RESOURCE_INIT(MeetRes);

    ui->setupUi(this);
    ui->tabWidget->setObjectName("mainTab");
    ui->tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);

    initTranslate();

    startMeetWidget = new StartMeetingWidget(this);
    ui->tabWidget->addTab(startMeetWidget, tr("Start Metting"));

    joinMeetWidget = new JoinMeetingWidget(this);
    ui->tabWidget->addTab(joinMeetWidget, tr("Join Metting"));

    BookMeetingWidget* bookMeet = new BookMeetingWidget(this);
    ui->tabWidget->addTab(bookMeet, tr("Book Metting"));

    MeetingSettingWidget* setting = new MeetingSettingWidget(this);
    ui->tabWidget->addTab(setting, tr("Setting"));

    reloadTheme();

    connect(startMeetWidget, &StartMeetingWidget::requstStartMeeting, this, &Widget::createMeeting);
    connect(joinMeetWidget, &JoinMeetingWidget::requstJoinMeeting, this, &Widget::joinMeeting);
}

Widget::~Widget() {
    delete ui;
}

void Widget::start() {}

void Widget::reloadTheme() {
    QString style = Style::getStylesheet("general.css");
    setStyleSheet(style);

    style = Style::getStylesheet("MettingBase.css");
    startMeetWidget->setStyleSheet(style);
    joinMeetWidget->setStyleSheet(style);
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

void Widget::retranslateUi() {
    ui->retranslateUi(this);
}

void Widget::joinMeeting() {}

void Widget::createMeeting() {
    qDebug() << __func__;
    view = new MeetingVideoFrame();
    view->show();
}
}  // namespace module::meet