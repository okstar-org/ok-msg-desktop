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
// Created by gaojie on 25-1-17.
//

#include "Widget.h"

#include <QAbstractButton>
#include <QGridLayout>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTabBar>
#include "Bus.h"
#include "JoinRoomWidget.h"
#include "StartRoomWidget.h"
#include "application.h"
#include "lib/storage/settings/OkSettings.h"
#include "lib/storage/settings/style.h"
#include "lib/storage/settings/translator.h"

#include <QContextMenuEvent>
#include <QMenu>

namespace module::classroom {

Widget::Widget(QWidget* parent) : lib::ui::OPage(parent), view{nullptr}
        , state{RoomState::None}
{
    OK_RESOURCE_INIT(Classroom);
    OK_RESOURCE_INIT(ClassroomRes);

    qRegisterMetaType<ok::base::Jid>("ok::base::Jid");
    qRegisterMetaType<lib::messenger::Participant>("lib::messenger::Participant");

    auto layout = new QGridLayout(this);

    tabWidget = new QTabWidget(this);
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tabWidget->setObjectName("mainTab");
    tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);

    startRoomWidget = new StartRoomWidget(this);
    tabWidget->addTab(startRoomWidget, tr("我是老师"));

    joinRoomWidget = new JoinRoomWidget(this);
    tabWidget->addTab(joinRoomWidget, tr("我是学生"));

    layout->addWidget(tabWidget);
    setLayout(layout);

    reloadTheme();
    initTranslate();

    connect(startRoomWidget, &StartRoomWidget::requstStartMeeting, this, &Widget::createMeeting);
    connect(startRoomWidget, &StartRoomWidget::requstDisbandMeeting, this,&Widget::destroyMeeting);
    connect(startRoomWidget, &StartRoomWidget::requstShareMeeting, this, &Widget::shareMeeting);
//    connect(joinRoomWidget, &JoinRoomWidget::requstJoinMeeting, this, &Widget::joinMeeting);
}

Widget::~Widget() {

}

void Widget::start() {}

void Widget::reloadTheme() {
    auto style = lib::settings::Style::getStylesheet("general.css");
    setStyleSheet(style);

    auto baseStyle = lib::settings::Style::getStylesheet("Base.css");
    startRoomWidget->setStyleSheet(baseStyle);
//    joinRoomWidget->setStyleSheet(style);
}

void Widget::doStart() {}

void Widget::initTranslate() {
    QString locale = lib::settings::OkSettings::getInstance().getTranslation();
    settings::Translator::translate(OK_Classroom_MODULE, locale);

    retranslateUi();
    connect(ok::Application::Instance()->bus(), &ok::Bus::languageChanged,
            [](QString locale0) { settings::Translator::translate(OK_Classroom_MODULE, locale0); });
}

void Widget::retranslateUi() {

}

void Widget::joinMeeting(const QString& no) {
    //    TODO 加入课堂
    qDebug() << __func__ << no;
}

/**
 * 创建会议
 * @param name
 */
void Widget::createMeeting(const QString& name, const lib::ortc::CtrlState& ctrlState) {
    qDebug() << __func__;

    QMutexLocker locker(&mutex);
    if (!currentMeetingName.isEmpty()) {
        qWarning() << "Existing meeting:" << this->currentMeetingName;
        return;
    }
    if (!view) {
        setState(RoomState::Creating);
        view = new RoomWindow(name, ctrlState);

        // TODO 暂时关闭即退出
        connect(view.data(), &RoomWindow::destroyed, this, [this]() {
            currentMeetingName.clear();
            setState(RoomState::None);
        });

        // 退出会议
        connect(view.data(), &RoomWindow::roomLeft, this, [this]() {
            currentMeetingName.clear();
            setState(RoomState::None);
            view->close();
        });

        connect(view.data(), &RoomWindow::roomCreated, this,
                [this]() { setState(RoomState::Created); });

        connect(view.data(), &RoomWindow::participantJoined, this,
                [this](const lib::messenger::Participant& part) {
                    setState(RoomState::Meeting);
                });

        connect(view.data(), &RoomWindow::roomDestroyed, this, [this]() {
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
    setState(RoomState::None);
}

void Widget::setState(const RoomState& state_) {
    if (view) {
        switch (state_) {
            case RoomState::None: {
                if (state == RoomState::Meeting) {
                    view.data()->stopCounter();
                }
            } break;
            case RoomState::Meeting: {
                if (state != RoomState::Meeting) {
                    view.data()->startCounter();
                }
            } break;
            default:
                break;
        }
    }

    state = state_;
    startRoomWidget->setMeetingState(state);
}

void Widget::activate() {
    QWidget* curr = tabWidget->currentWidget();
    if (curr == startRoomWidget) {
        startRoomWidget->focusInput();
    } else if (curr == joinRoomWidget) {
        joinRoomWidget->focusInput();
    }
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

}  // namespace module::classroom
