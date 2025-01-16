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
#ifndef OPLAYERCONTROLLER_H
#define OPLAYERCONTROLLER_H

#include <QLabel>
#include <QSlider>
#include <QToolButton>
#include <QWidget>

#ifdef QtAV
#include <QtAV>
#endif

namespace Ui {
class OPlayerController;
}

class OPlayerController : public QWidget {
    Q_OBJECT

public:
    explicit OPlayerController(QWidget* parent = nullptr);
    ~OPlayerController();

#ifdef QtAV
    void init(QtAV::AVPlayer* player);
#endif

    QSlider* playSlider();
    QSlider* volumeSlider();
    QToolButton* playBtn();
    QToolButton* muteBtn();
    QLabel* currentTime();
    QLabel* totalTime();

private slots:
    void on_m_playBtn_clicked(bool checked);

private:
    Ui::OPlayerController* ui;
};

#endif  // OPLAYERCONTROLLER_H
