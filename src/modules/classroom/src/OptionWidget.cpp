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

#include "OptionWidget.h"
#include "base/RoundedPixmapLabel.h"
#include "base/images.h"
#include "lib/ui/widget/tools/PopupMenuComboBox.h"
#include "src/Bus.h"
#include "src/application.h"

#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

namespace module::classroom {

OptionWidget::OptionWidget(QWidget* parent)
        : QWidget(parent), ctrlState{true, true, true} {
    auto profile = ok::Application::Instance()->getProfile();

    avatarLabel = new RoundedPixmapLabel(this);
    avatarLabel->setObjectName("avatarLabel");
    avatarLabel->setAttribute(Qt::WA_StyledBackground);
    avatarLabel->setContentsSize(QSize(120, 120));
    avatarLabel->setImage(profile->getAvatar());

    micSpeakSetting = new lib::ui::PopupMenuComboBox(this);
    // micSpeakSetting->iconButton()->setIcon(QIcon(":/meet/image/micphone.svg"));
    micSpeakSetting->setLabel(tr("Micphone"));
    micSpeakSetting->setCursor(Qt::PointingHandCursor);
    connect(micSpeakSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableMic = !ctrlState.enableMic;
        updateAudioVideoIcon(true, false, false);
    });

    cameraSetting = new lib::ui::PopupMenuComboBox(this);
    // cameraSetting->iconButton()->setIcon(QIcon(":/meet/image/videocam.svg"));
    cameraSetting->setLabel(tr("Camera"));
    cameraSetting->setCursor(Qt::PointingHandCursor);

    connect(cameraSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableCam = !ctrlState.enableCam;
        updateAudioVideoIcon(false, true, false);
    });

    volumeSetting = new lib::ui::PopupMenuComboBox(this);
    // volumnSetting->iconButton()->setIcon(QIcon(":/meet/image/speaker.svg"));
    volumeSetting->iconButton()->setCursor(Qt::PointingHandCursor);
    connect(volumeSetting->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableSpk = !ctrlState.enableSpk;
        updateAudioVideoIcon(false, false, true);
    });

    updateAudioVideoIcon(true, true, true);

    volumeSlider = new QSlider(Qt::Horizontal, volumeSetting);
    volumeSlider->setRange(0, 100);
    volumeSetting->setWidget(volumeSlider);

    buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(10);

    QHBoxLayout* footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(10);
    footerLayout->addWidget(micSpeakSetting);
    footerLayout->addWidget(cameraSetting);
    footerLayout->addWidget(volumeSetting);
    footerLayout->addStretch(1);
    footerLayout->addLayout(buttonLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(avatarLabel, 1);
    mainLayout->addLayout(footerLayout);

    //    auto profile = ok::Application::Instance()->getProfile();
    //    connect(profile, &lib::session::Profile::selfAvatarChanged, avatarLabel,
    //    &RoundedPixmapLabel::setImage);
}

OptionWidget::~OptionWidget() {}

void OptionWidget::addFooterButton(QPushButton* button) {
    buttonLayout->addWidget(button);
}

void OptionWidget::retranslateUi() {
    micSpeakSetting->setLabel(tr("Micphone"));
    cameraSetting->setLabel(tr("Camera"));
}

void OptionWidget::showEvent(QShowEvent* event) {}

void OptionWidget::updateAudioVideoIcon(bool audio, bool video, bool speaker) {
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

    if (speaker) {
        if (ctrlState.enableSpk) {
            volumeSetting->iconButton()->setIcon(QIcon(":/meet/image/speaker.svg"));
        } else {
            volumeSetting->iconButton()->setIcon(QIcon(":/meet/image/speaker_stop.svg"));
        }
    }
}

}  // namespace module::classroom
