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
#include "RoundedAvatarLabel.h"
#include "src/Bus.h"
#include "src/application.h"

#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

MeetingOptionWidget::MeetingOptionWidget(QWidget* parent) : QWidget(parent) {
    avatarLabel = new RoundedAvatarLabel(this);
    avatarLabel->setObjectName("avatarLabel");
    avatarLabel->setAttribute(Qt::WA_StyledBackground);
    avatarLabel->setContentsSize(QSize(120, 120));

    micSpeakSetting = new VideoDeviceSettingWidget(this);
    micSpeakSetting->iconButton()->setIcon(QIcon(":/meet/image/micphone.svg"));
    micSpeakSetting->setLabel(tr("Micphone"));
    cameraSetting = new VideoDeviceSettingWidget(this);
    cameraSetting->iconButton()->setIcon(QIcon(":/meet/image/videocam.svg"));
    cameraSetting->setLabel(tr("Camera"));
    volumnSetting = new VideoDeviceSettingWidget(this);
    volumnSetting->iconButton()->setIcon(QIcon(":/meet/image/volumn_2.svg"));
    volumnSlider = new QSlider(Qt::Horizontal, volumnSetting);
    volumnSlider->setRange(0, 100);
    volumnSetting->setWidget(volumnSlider);

    confirmButton = new QPushButton(this);
    confirmButton->setObjectName("confirm");
    confirmButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    QHBoxLayout* footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(10);
    footerLayout->addWidget(micSpeakSetting);
    footerLayout->addWidget(cameraSetting);
    footerLayout->addWidget(volumnSetting);
    footerLayout->addStretch(1);
    footerLayout->addWidget(confirmButton);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(avatarLabel, 1);
    mainLayout->addLayout(footerLayout);

    ok::Bus* bus = ok::Application::Instance()->bus();
    connect(bus, &ok::Bus::avatarChanged, avatarLabel, &RoundedAvatarLabel::setPixmap);
}

void MeetingOptionWidget::setConfirmButtonText(const QString& text) {
    confirmButton->setText(text);
}

VideoDeviceSettingWidget::VideoDeviceSettingWidget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    _iconButton = new QToolButton(this);
    menuButton = new QToolButton(this);
    menuButton->setIcon(QIcon(":/meet/image/up_arrow.svg"));
    mainLayout->addWidget(_iconButton);
    mainLayout->addWidget(menuButton);
}

void VideoDeviceSettingWidget::setLabel(const QString& text) { setWidget(new QLabel(text, this)); }

void VideoDeviceSettingWidget::setWidget(QWidget* widget) {
    if (content.isNull()) {
        if (widget) {
            mainLayout->insertWidget(1, widget, 1);
        }
    } else {
        if (widget) {
            mainLayout->replaceWidget(content, widget);
        } else {
            content->deleteLater();
        }
    }
    content = widget;
}

QAbstractButton* VideoDeviceSettingWidget::iconButton() { return _iconButton; }

void VideoDeviceSettingWidget::showEvent(QShowEvent* e) {
    ok::Bus* bus = ok::Application::Instance()->bus();
    emit bus->getAvatar();
}
