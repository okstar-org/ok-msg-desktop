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

#include "MeetingVideoOutput.h"
#include "base/RoundedPixmapLabel.h"
namespace module::meet {

MeetingVideoOutput::MeetingVideoOutput(QWidget* parent) : QWidget(parent) {}

void MeetingVideoOutput::bindParticipant(MeetingParticipant* participant) {}

void MeetingVideoOutput::showVideo() {
    if (avatarLabel) {
        avatarLabel->deleteLater();
        avatarLabel = nullptr;
    }
}

void MeetingVideoOutput::showAvatar() {
    if (!avatarLabel) {
        avatarLabel = new RoundedPixmapLabel(this);
        avatarLabel->setContentsSize(QSize(120, 120));
    }
    avatarLabel->setGeometry(this->rect());
    avatarLabel->raise();
}

void MeetingVideoOutput::resizeEvent(QResizeEvent* event) {
    if (avatarLabel) {
        avatarLabel->setGeometry(this->rect());
        avatarLabel->raise();
    }
}
}  // namespace module::meet