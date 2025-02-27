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

#include "CallDurationForm.h"
#include "ui_CallDurationForm.h"

#include <QDebug>
#include <QPushButton>
#include <QTimer>
#include "base/times.h"
#include "src/core/coreav.h"
#include "src/lib/storage/settings/style.h"
#include "src/video/netcamview.h"
namespace module::im {

CallDurationForm::CallDurationForm(QWidget* parent)
        : QWidget(parent)
        , ui(new Ui::CallDurationForm)
        , contact{nullptr}
        , muteOut{false}
        , muteIn{false}
        , netcam{nullptr}
        , timeElapsed{nullptr} {
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setWindowFlags(Qt::WindowMinMaxButtonsHint);

    ui->videoButton->hide();

    connect(ui->callButton, &QPushButton::clicked, this, &CallDurationForm::onCallEnd);
    connect(ui->micButton, &QPushButton::clicked, this, &CallDurationForm::onMuteOut);
    connect(ui->volButton, &QPushButton::clicked, this, &CallDurationForm::onMuteIn);

    reloadTheme();

    callDurationTimer = new QTimer(this);
    connect(callDurationTimer, &QTimer::timeout, this, &CallDurationForm::onUpdateTime);
}

CallDurationForm::~CallDurationForm() {
    //    QString dhms = ok::base::secondsToDHMS(timeElapsed.elapsed() / 1000);
    //    QString mess = error ? tr("Call with %1 ended unexpectedly. %2")
    //                         : tr("Call with %1 ended. %2");

    //  addSystemInfoMessage(mess.arg(name, dhms), ChatMessage::INFO,
    //                       QDateTime::currentDateTime());
    callDurationTimer->stop();

    // delete callDurationTimer;
    // callDurationTimer = nullptr;

    if (timeElapsed != nullptr) {
        delete timeElapsed;
        timeElapsed = nullptr;
    }

    delete ui;
}

void CallDurationForm::setContact(const Contact* c) {
    if (!c) return;
    contact = c;
}

void CallDurationForm::reloadTheme() {
    setStyleSheet(lib::settings::Style::getStylesheet(
            QStringLiteral("CallDurationForm/CallDurationForm.css")));
}

void CallDurationForm::closeEvent(QCloseEvent* e) {}

void CallDurationForm::onUpdateTime() {
    if (!timeElapsed) {
        return;
    }
    auto time = tr("Call duration: ") + ok::base::secondsToDHMS(timeElapsed->elapsed() / 1000);
    ui->duration->setText(time);
}

void CallDurationForm::startCounter() {
    // 启动计时
    if (timeElapsed == nullptr) {
        timeElapsed = new QElapsedTimer();
    }
    timeElapsed->start();

    if (!timeElapsed->isValid()) {
        qWarning() << "Unsupported QElapsedTimer!";
        return;
    }

    // 启动计时器
    callDurationTimer->start(1000);
}

void CallDurationForm::stopCounter() {
    if ((timeElapsed == nullptr) || !timeElapsed->isValid()) {
        return;
    }
    timeElapsed->invalidate();

    delete timeElapsed;
    timeElapsed = nullptr;

    callDurationTimer->stop();
}

void CallDurationForm::onCallEnd() {
    emit endCall();
}

void CallDurationForm::onMuteOut() {
    muteOut = !muteOut;
    ui->micButton->setProperty("state", muteOut ? "disabled" : "");
    emit muteMicrophone(muteOut);
    reloadTheme();
}

void CallDurationForm::onMuteIn() {
    muteIn = !muteIn;
    ui->volButton->setProperty("state", muteIn ? "disabled" : "");
    emit muteSpeaker(muteIn);
    reloadTheme();
}

GenericNetCamView* CallDurationForm::createNetcam() {
    qDebug() << __func__ << "...";

    if (!contact) {
        qWarning() << "contact is no existing!";
        return nullptr;
    }

    auto fId = FriendId(contact->getIdAsString());

    auto view = new NetCamView(fId, this);
    CoreAV* av = CoreAV::getInstance();

    lib::video::VideoSource* source = av->getVideoSourceFromCall(fId.getId());
    view->show(source, contact->getDisplayedName());

    //  connect(view, &GenericNetCamView::videoCallEnd, this,
    //          &ChatForm::onVideoCallTriggered);

    //  connect(view, &GenericNetCamView::volMuteToggle, this,
    //          &ChatForm::onVolMuteToggle);
    //  connect(view, &GenericNetCamView::micMuteToggle, this,
    //          &ChatForm::onMicMuteToggle);

    connect(view, &GenericNetCamView::videoPreviewToggle, view, &NetCamView::toggleVideoPreview);
    return view;
}

void CallDurationForm::showNetcam() {
    if (!netcam) {
        netcam = createNetcam();
        ui->viewbar->layout()->addWidget(netcam);
    }
    netcam->show();
}

void CallDurationForm::hideNetcam() {
    if (!netcam) return;

    ui->viewbar->layout()->removeWidget(netcam);

    netcam->close();
    netcam->hide();
    delete netcam;
    netcam = nullptr;
}

void CallDurationForm::showAvatar() {
    if (contact) {
        auto c = new QLabel();
        c->setFixedSize(86, 86);
        c->setPixmap(contact->getAvatar().scaled(c->size()));
        ui->viewbar->layout()->addWidget(c);
    }
}
}  // namespace module::im
