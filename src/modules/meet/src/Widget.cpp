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

Widget::Widget(QWidget* parent)
        : UI::OMenuWidget(parent)
        , ui(new Ui::WorkPlatform)
        , view{nullptr}
        , state{MeetingState::NoMeeting} {
    OK_RESOURCE_INIT(Meet);
    OK_RESOURCE_INIT(MeetRes);

    qRegisterMetaType<ok::base::Jid>("ok::base::Jid");
    qRegisterMetaType<lib::messenger::Participant>("lib::messenger::Participant");

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
    connect(startMeetWidget, &StartMeetingWidget::requstDisbandMeeting, this,
            &Widget::destroyMeeting);
    connect(startMeetWidget, &StartMeetingWidget::requstShareMeeting, this, &Widget::shareMeeting);
    connect(joinMeetWidget, &JoinMeetingWidget::requstJoinMeeting, this, &Widget::joinMeeting);
}

Widget::~Widget() {
    delete ui;
}

void Widget::start() {}

void Widget::reloadTheme() {
    QString style = Style::getStylesheet("general.css");
    setStyleSheet(style);

    style = Style::getStylesheet("MeetingBase.css");
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

void Widget::joinMeeting(const QString& no) {
    //    TODO 加入会议
}

/**
 * 创建会议
 * @param name
 */
void Widget::createMeeting(const QString& name) {
    qDebug() << __func__;

    QMutexLocker locker(&mutex);
    if (!currentMeetingName.isEmpty()) {
        qWarning() << "Existing meeting:" << this->currentMeetingName;
        return;
    }
    if (!view) {
        setState(MeetingState::CreatingMeeting);
        view = new MeetingVideoFrame(name);
        // TODO 暂时关闭即退出
        connect(view.data(), &MeetingVideoFrame::destroyed, this, [this]() {
            currentMeetingName.clear();
            setState(MeetingState::NoMeeting);
        });

        // 退出会议
        connect(view.data(), &MeetingVideoFrame::meetLeft, this, [this]() {
            currentMeetingName.clear();
            setState(MeetingState::NoMeeting);
            view->close();
        });

        connect(view.data(), &MeetingVideoFrame::meetCreated, this,
                [this](const QString& name) { setState(MeetingState::Created); });

        connect(view.data(), &MeetingVideoFrame::participantJoined, this,
                [this](const QString& name, const lib::messenger::Participant& part) {
                    setState(MeetingState::OnMeeting);
        });
    }

    currentMeetingName = name;
    view->show();
}

/**
 * 解散会议（销毁）
 */
void Widget::destroyMeeting() {
    QMutexLocker locker(&mutex);
    currentMeetingName.clear();
    if (view) {
        view->deleteLater();
        view = nullptr;
    }

    setState(MeetingState::NoMeeting);
}

void Widget::setState(const MeetingState& state_) {
    state = state_;
    startMeetWidget->setMeetingState(state);
}

/**
 * 分享会议
 */
void Widget::shareMeeting() {
    makeShare();
}

Share Widget::makeShare() {
    // TODO 生成加入会议分享信息
    qDebug() << tr("make share link...");
    return Share();
}

}  // namespace module::meet