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
#include "OPlayerController.h"
#include "ui_OPlayerController.h"

OPlayerController::OPlayerController(QWidget* parent)
        : QWidget(parent), ui(new Ui::OPlayerController) {
    ui->setupUi(this);
}

OPlayerController::~OPlayerController() {
    delete ui;
}

#ifdef QtAV
void OPlayerController::init(QtAV::AVPlayer* m_player_) {
    connect(m_player_, SIGNAL(error(QtAV::AVError)), this, SLOT(handleError(QtAV::AVError)));
    connect(m_player_, SIGNAL(started()), this, SLOT(onStartPlay()));
    connect(m_player_, SIGNAL(stopped()), this, SLOT(onStopPlay()));
    connect(m_player_, SIGNAL(paused(bool)), this, SLOT(onPaused(bool)));
    //    connect(m_player_, SIGNAL(speedChanged(qreal)), this, SLOT(onSpeedChange(qreal)));
    connect(m_player_, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChange(qint64)));

    connect(ui->m_playSlider, SIGNAL(sliderMoved(int)), SLOT(seekBySlider(int)));
    connect(ui->m_playSlider, SIGNAL(sliderPressed()), SLOT(seekBySlider()));
    connect(ui->m_volumeSlider, SIGNAL(sliderPressed()), SLOT(setVolume()));
    connect(ui->m_volumeSlider, SIGNAL(valueChanged(int)), SLOT(setVolume()));
    connect(ui->m_playBtn, SIGNAL(clicked()), SLOT(playPause()));
    //    connect(ui->m_stopBtn, SIGNAL(clicked()), m_player_, SLOT(stop()));
    connect(ui->m_muteBtn, SIGNAL(clicked()), SLOT(muteVolume()));
}
#endif

QSlider* OPlayerController::playSlider() {
    return ui->m_playSlider;
}

QSlider* OPlayerController::volumeSlider() {
    return ui->m_volumeSlider;
}

QToolButton* OPlayerController::playBtn() {
    return ui->m_playBtn;
}

QToolButton* OPlayerController::muteBtn() {
    return ui->m_muteBtn;
}

QLabel* OPlayerController::currentTime() {
    return ui->m_time_current;
}

QLabel* OPlayerController::totalTime() {
    return ui->m_time_total;
}

void OPlayerController::on_m_playBtn_clicked(bool checked) {}
