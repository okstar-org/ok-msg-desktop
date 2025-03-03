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
#include "lib/storage/settings/OkSettings.h"
#include "lib/storage/settings/style.h"
#include "lib/storage/settings/translator.h"
#include "meetingview/MeetingVideoFrame.h"

#include <QAbstractButton>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

#include <QContextMenuEvent>
#include <QMenu>

namespace module::meet {

Widget::Widget(QWidget* parent)
        : lib::ui::OFrame(parent)
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

    startMeetWidget = new StartMeetingWidget(this);
    ui->tabWidget->addTab(startMeetWidget, tr("Start Meeting"));
    connect(startMeetWidget, &StartMeetingWidget::requstStartMeeting,   //
            this, &Widget::createMeeting);
    connect(startMeetWidget, &StartMeetingWidget::requstDisbandMeeting, //
            this, &Widget::destroyMeeting);
    connect(startMeetWidget, &StartMeetingWidget::requstShareMeeting, this, &Widget::shareMeeting);

    // 预约会议
    // BookMeetingWidget* bookMeet = new BookMeetingWidget(this);
    // ui->tabWidget->addTab(bookMeet, tr("Book Meeting"));

    // 加入会议
    // joinMeetWidget = new JoinMeetingWidget(this);
    // ui->tabWidget->addTab(joinMeetWidget, tr("Join Meeting"));
    // connect(joinMeetWidget, &JoinMeetingWidget::requstJoinMeeting, this, &Widget::joinMeeting);

    // 设置
    // MeetingSettingWidget* setting = new MeetingSettingWidget(this);
    // ui->tabWidget->addTab(setting, tr("Setting"));



    initTranslate();
    reloadTheme();
}

Widget::~Widget() {
    delete ui;
}

void Widget::start() {}

void Widget::reloadTheme() {
    QString style = lib::settings::Style::getStylesheet("general.css");
    setStyleSheet(style);

    auto mStyle = lib::settings::Style::getStylesheet("MeetingBase.css");
    startMeetWidget->setStyleSheet(mStyle);

    // joinMeetWidget->setStyleSheet(mStyle);
}

void Widget::doStart() {}

void Widget::initTranslate() {
    retranslateUi();
    connect(ok::Application::Instance()->bus(), &ok::Bus::languageChanged,
            [&](const QString& locale0) {
                retranslateUi();
                settings::Translator::translate(OK_Meet_MODULE, locale0);
            });
}

void Widget::retranslateUi() {
    QString locale = lib::settings::OkSettings::getInstance().getTranslation();
    settings::Translator::translate(OK_Meet_MODULE, locale);

    ui->retranslateUi(this);

    if (ui->tabWidget->count() >= 4) {
        ui->tabWidget->setTabText(0, tr("Start Meeting"));
        if (StartMeetingWidget* widget =
                    qobject_cast<StartMeetingWidget*>(ui->tabWidget->widget(0))) {
            widget->retranslateUi();
        }

        ui->tabWidget->setTabText(1, tr("Join Meeting"));
        if (JoinMeetingWidget* widget =
                    qobject_cast<JoinMeetingWidget*>(ui->tabWidget->widget(1))) {
            widget->retranslateUi();
        }

        ui->tabWidget->setTabText(2, tr("Book Meeting"));

        ui->tabWidget->setTabText(3, tr("Setting"));
        if (MeetingSettingWidget* widget =
                    qobject_cast<MeetingSettingWidget*>(ui->tabWidget->widget(3))) {
            widget->retranslateUi();
        }
    }
}

void Widget::joinMeeting(const QString& no) {
    //    TODO 加入会议
}

/**
 * 创建会议
 * @param name
 */
void Widget::createMeeting(const QString& name,
                           const QStringList& aDeviceList,
                           const QVector<lib::video::VideoDevice>& vDeviceList,
                           const lib::ortc::DeviceConfig& conf,
                           const lib::ortc::CtrlState& ctrlState) {
    qDebug() << __func__;
    QMutexLocker locker(&mutex);
    if (!currentMeetingName.isEmpty()) {
        qWarning() << "Existing meeting:" << this->currentMeetingName;
        return;
    }
    if (!view) {
        setState(MeetingState::CreatingMeeting);
        view = new MeetingVideoFrame(name,aDeviceList, vDeviceList, conf, ctrlState);

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
                [this](const QString& name) {
                    setState(MeetingState::Created);
                });

        connect(view.data(), &MeetingVideoFrame::participantJoined, this,
                [this](const QString& name, const lib::messenger::Participant& part) {
                    setState(MeetingState::OnMeeting);
                });

        connect(view.data(), &MeetingVideoFrame::meetDestroyed, this, [this]() {
            view->deleteLater();
            view = nullptr;
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
        // 离开会议
        view->doLeaveMeet();
    }
    setState(MeetingState::NoMeeting);
}

void Widget::setState(const MeetingState& state_) {
    if (view) {
        switch (state_) {
            case MeetingState::NoMeeting: {
                if (state == MeetingState::OnMeeting) {
                    view.data()->stopCounter();
                }
            } break;
            case MeetingState::OnMeeting: {
                if (state != MeetingState::OnMeeting) {
                    view.data()->startCounter();
                }
            } break;
            default:
                break;
        }
    }

    state = state_;
    startMeetWidget->setMeetingState(state);
}

void Widget::activate() {
    // 暂时先这么做吧
    // 使用showevent事件感觉有些不合适
    QWidget* curr = ui->tabWidget->currentWidget();
    if (curr == startMeetWidget) {
        startMeetWidget->focusInput();
    }
    // if (curr == joinMeetWidget) {
    //  joinMeetWidget->focusInput();
    // }
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
