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
#ifndef OVIDEOPLAYER_H
#define OVIDEOPLAYER_H

#include <QUrl>
#include <QWidget>
#include "OPlayerController.h"

namespace Ui {
class OVideoPlayer;
}

class OVideoPlayer : public QWidget {
    Q_OBJECT

public:
    explicit OVideoPlayer(QWidget* parent = nullptr);
    ~OVideoPlayer();

    void setRendererWidget(QWidget* rw);

    OPlayerController* controller();

    void start();
    void setSource(const QUrl& url);
    void play();
    void stop();

    const QString fileName();

public slots:
    void seekBySlider(int value);
    void seekBySlider();
    void playPause();

private slots:
    void setVolume();
    void onPositionChange(qint64 value);
    void onStartPlay();
    void onStopPlay();
    void updateSliderUnit();

    void muteVolume();

    void syncVolumeUi(qreal value);
    void onPaused(bool);
    // void handleError(QtAV::AVError);

private:
    Ui::OVideoPlayer* ui;

    QUrl m_source;

    void* initPlayer();
    void* audioOutput();
    void* m_player_;

    QWidget* welcome_;
    int m_unit;

    OPlayerController* controller_;
};

#endif  // OVIDEOPLAYER_H
