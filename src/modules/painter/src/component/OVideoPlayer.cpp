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
#include "OVideoPlayer.h"
#include "ui_OVideoPlayer.h"

#include "base/logs.h"

#include <QMessageBox>

const qreal kVolumeInterval = 0.1;

OVideoPlayer::OVideoPlayer(QWidget* parent)
        : QWidget(parent), ui(new Ui::OVideoPlayer), m_unit(1000) {
    ui->setupUi(this);
    controller_ = ui->playerController;
}

OVideoPlayer::~OVideoPlayer() {
    delete ui;
}

void OVideoPlayer::setRendererWidget(QWidget* rw) {
    QWidget* welcome = ui->player_renderer;

    layout()->replaceWidget(ui->player_renderer, rw);

    delete welcome;
}

void OVideoPlayer::start() {
    m_player_ = initPlayer();
    if (!m_player_) {
        QMessageBox::warning(nullptr, QString::fromLatin1("QtAV"), tr("Player error!"));
        return;
    }
}

void* OVideoPlayer::initPlayer() {
#ifdef QtAV
    QtAV::AVPlayer* m_player_ = new QtAV::AVPlayer(this);
    QtAV::VideoOutput* m_vo = new QtAV::VideoOutput(this);
    if (!m_vo->widget()) {
        DEBUG_LOG_S(L_ERROR) << "Can not create video renderer";
        return nullptr;
    }

    m_player_->setRenderer(m_vo);

    // 本地界面
    setRendererWidget(m_vo->widget());

    connect(m_player_, SIGNAL(error(QtAV::AVError)), this, SLOT(handleError(QtAV::AVError)));
    connect(m_player_, SIGNAL(started()), this, SLOT(onStartPlay()));
    connect(m_player_, SIGNAL(stopped()), this, SLOT(onStopPlay()));
    connect(m_player_, SIGNAL(paused(bool)), this, SLOT(onPaused(bool)));
    //    connect(m_player_, SIGNAL(speedChanged(qreal)), this, SLOT(onSpeedChange(qreal)));
    connect(m_player_, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChange(qint64)));

    connect(controller_->playSlider(), SIGNAL(sliderMoved(int)), SLOT(seekBySlider(int)));
    connect(controller_->playSlider(), SIGNAL(sliderPressed()), SLOT(seekBySlider()));
    connect(controller_->volumeSlider(), SIGNAL(sliderPressed()), SLOT(setVolume()));
    connect(controller_->volumeSlider(), SIGNAL(valueChanged(int)), SLOT(setVolume()));
    connect(controller_->playBtn(), SIGNAL(clicked()), SLOT(playPause()));
    //    connect(ui->m_stopBtn, SIGNAL(clicked()), m_player_, SLOT(stop()));
    connect(controller_->muteBtn(), SIGNAL(clicked()), SLOT(muteVolume()));

#endif
    return m_player_;
}

void* OVideoPlayer::audioOutput() {
    return nullptr;
    // return m_player_ ? m_player_->audio() : nullptr;
}

void OVideoPlayer::play() {
#ifdef QtAV
    if (m_player_) m_player_->play();
#endif
}

void OVideoPlayer::stop() {
#ifdef QtAV
    if (m_player_) m_player_->deleteLater();
#endif
}

const QString OVideoPlayer::fileName() {
    return m_source.fileName();
}

void OVideoPlayer::setSource(const QUrl& url) {
#ifdef QtAV
    if (m_source == url) return;
    if (!m_player_) return;
    m_source = url;
    if (url.isLocalFile() || url.scheme().isEmpty() || url.scheme().startsWith("qrc") ||
        url.scheme().startsWith("avdevice")
        // TODO: what about custom io?
    )
        m_player_->setFile(QUrl::fromPercentEncoding(url.toEncoded()));
    else
        m_player_->setFile(url.toEncoded());
#endif
}

void OVideoPlayer::setVolume() {
#ifdef QtAV
    QSlider* volumeSlider = controller_->volumeSlider();
    QToolButton* muteBtn = controller_->muteBtn();

    QtAV::AudioOutput* ao = audioOutput();
    qreal v = qreal(volumeSlider->value()) * kVolumeInterval;
    if (ao) {
        if (qAbs(int(ao->volume() / kVolumeInterval) - volumeSlider->value()) >=
            int(0.1 / kVolumeInterval)) {
            ao->setVolume(v);
        }
    }
    volumeSlider->setToolTip(QString::number(v));
    controller_->muteBtn()->setToolTip(QString::number(v));
    if (v <= 0) {
        muteBtn->setChecked(true);
    } else
        muteBtn->setChecked(false);
#endif
}

void OVideoPlayer::muteVolume() {
#ifdef QtAV
    QSlider* volumeSlider = controller_->volumeSlider();
    qreal v;
    QtAV::AudioOutput* ao = audioOutput();
    if (ao->volume() <= 0) {
        v = qreal(40) * kVolumeInterval;
        volumeSlider->setValue(40);
    } else {
        v = 0;
        volumeSlider->setValue(0);
    }
    ao->setVolume(v);
#endif
}

void OVideoPlayer::syncVolumeUi(qreal value) {
    QSlider* volumeSlider = controller_->volumeSlider();
    int v = 0;
    if (0 < value) {
        v = (value / kVolumeInterval);
    }
    volumeSlider->setValue(v);
}

void OVideoPlayer::seekBySlider(int value) {
#ifdef QtAV
    if (!m_player_->isPlaying()) return;
    m_player_->seek(qint64(value * m_unit));
#endif
}

void OVideoPlayer::seekBySlider() {
    QSlider* playSlider = controller_->playSlider();
    seekBySlider(playSlider->value());
}

void OVideoPlayer::playPause() {
#ifdef QtAV
    if (!m_player_->isPlaying()) {
        m_player_->play();
        return;
    }
    m_player_->pause(!m_player_->isPaused());
#endif
}

void OVideoPlayer::onPositionChange(qint64 pos) {
#ifdef QtAV
    QSlider* playSlider = controller_->playSlider();
    QLabel* currentTime = controller_->currentTime();

    playSlider->setRange(0, int(m_player_->duration() / m_unit));
    playSlider->setValue(int(pos / m_unit));

    currentTime->setText(QTime(0, 0, 0).addMSecs(pos).toString(QString::fromLatin1("HH:mm:ss")));
#endif
}

void OVideoPlayer::onStartPlay() {
#ifdef QtAV
    QToolButton* playBtn = controller_->playBtn();
    QSlider* playSlider = controller_->playSlider();
    QSlider* volumeSlider = controller_->volumeSlider();
    QLabel* totalTime = controller_->totalTime();

    totalTime->setText(QTime(0, 0, 0)
                               .addMSecs(m_player_->mediaStopPosition())
                               .toString(QString::fromLatin1("HH:mm:ss")));

    playBtn->setIcon(QIcon(QString::fromLatin1(":/resources/icon/player_pause.png")));

    playSlider->setValue(0);
    playSlider->setMinimum(m_player_->mediaStartPosition());
    playSlider->setMaximum(m_player_->mediaStopPosition());
    playSlider->setEnabled(m_player_->isSeekable());

    QtAV::AudioOutput* ao = audioOutput();
    if (ao) {
        volumeSlider->setValue((ao->volume() / kVolumeInterval));
    }

    onPositionChange(m_player_->position());
#endif
}

void OVideoPlayer::onPaused(bool p) {}

// void OVideoPlayer::handleError(QtAV::AVError error)
// {
//     DEBUG_LOG_S(L_ERROR) << error;
// }

void OVideoPlayer::onStopPlay() {
    QToolButton* playBtn = controller_->playBtn();
    QSlider* playSlider = controller_->playSlider();
    QLabel* totalTime = controller_->totalTime();
    QLabel* currentTime = controller_->currentTime();

    //    onPositionChange(m_player_->position());
    totalTime->setText(QString::fromLatin1("00:00:00"));
    currentTime->setText(QString::fromLatin1("00:00:00"));

    playSlider->setValue(0);
    playSlider->setMinimum(0);
    playSlider->setMaximum(0);
    playSlider->setDisabled(true);

    playBtn->setIcon(QIcon(QString::fromLatin1(":/resources/icon/player_play.png")));
}

void OVideoPlayer::updateSliderUnit() {}
