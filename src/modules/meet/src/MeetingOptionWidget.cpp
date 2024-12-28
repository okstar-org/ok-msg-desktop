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

#include "MeetingOptionWidget.h"
#include "base/RoundedPixmapLabel.h"
#include "src/Bus.h"
#include "src/application.h"
#include "tools/PopupMenuComboBox.h"

#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

namespace module::meet {

MeetingOptionWidget::MeetingOptionWidget(QWidget* parent) : QWidget(parent) {
    avatarLabel = new RoundedPixmapLabel(this);
    avatarLabel->setObjectName("avatarLabel");
    avatarLabel->setAttribute(Qt::WA_StyledBackground);
    avatarLabel->setContentsSize(QSize(120, 120));

    micSpeakSetting = new PopupMenuComboBox(this);
    // micSpeakSetting->iconButton()->setIcon(QIcon(":/meet/image/micphone.svg"));
    micSpeakSetting->setLabel(tr("Micphone"));
    micSpeakSetting->setCursor(Qt::PointingHandCursor);
    connect(micSpeakSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableMic = !ctrlState.enableMic;
        updateAudioVideoIcon(true, false);
    });
    ctrlState.enableMic = true;

    cameraSetting = new PopupMenuComboBox(this);
    // cameraSetting->iconButton()->setIcon(QIcon(":/meet/image/videocam.svg"));
    cameraSetting->setLabel(tr("Camera"));
    connect(cameraSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableCam = !ctrlState.enableCam;
        updateAudioVideoIcon(false, true);
    });
    ctrlState.enableCam = true;
    updateAudioVideoIcon(true, true);

    volumnSetting = new PopupMenuComboBox(this);
    volumnSetting->iconButton()->setIcon(QIcon(":/meet/image/volumn_2.svg"));
    volumnSlider = new QSlider(Qt::Horizontal, volumnSetting);
    volumnSlider->setRange(0, 100);
    volumnSetting->setWidget(volumnSlider);

    buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(10);

    QHBoxLayout* footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(10);
    footerLayout->addWidget(micSpeakSetting);
    footerLayout->addWidget(cameraSetting);
    footerLayout->addWidget(volumnSetting);
    footerLayout->addStretch(1);
    footerLayout->addLayout(buttonLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(avatarLabel, 1);
    mainLayout->addLayout(footerLayout);

    auto bus = ok::Application::Instance()->bus();
    connect(bus, &ok::Bus::avatarChanged, avatarLabel, &RoundedPixmapLabel::setPixmap);
}

void MeetingOptionWidget::addFooterButton(QPushButton* button) {
    buttonLayout->addWidget(button);
}

void MeetingOptionWidget::retranslateUi() {
    micSpeakSetting->setLabel(tr("Micphone"));
    cameraSetting->setLabel(tr("Camera"));
}

void MeetingOptionWidget::showEvent(QShowEvent* event) {
    auto bus = ok::Application::Instance()->bus();
    emit bus->getAvatar();
}
void MeetingOptionWidget::updateAudioVideoIcon(bool audio, bool video) {
    if (audio) {
        if (ctrlState.enableMic) {
            micSpeakSetting->iconButton()->setIcon(QIcon(":/meet/image/micphone.svg"));
        } else {
            micSpeakSetting->iconButton()->setIcon(QIcon(":/meet/image/micphone_mute.svg"));
        }
    }
    if (video) {
        if (ctrlState.enableCam) {
            cameraSetting->iconButton()->setIcon(QIcon(":/meet/image/videocam.svg"));
        } else {
            cameraSetting->iconButton()->setIcon(QIcon(":/meet/image/videocam_stop.svg"));
        }
    }
}
}  // namespace module::meet
