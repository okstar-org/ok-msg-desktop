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
#include "src/MeetingParticipant.h"

#include <QPainter>
#include <QStyleOption>

namespace module::meet {

MeetingVideoOutput::MeetingVideoOutput(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);
}

void MeetingVideoOutput::bindParticipant(MeetingParticipant* participant) {
    this->participant = participant;
    update();
}

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

void MeetingVideoOutput::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if (!this->participant) {
        return;
    }

    bool isVideo = false;
    if (!isVideo) {
        QString text = this->participant->getNick().right(2);
        if (!text.isEmpty()) {
            QRect rect = this->rect();
            int d = std::min(rect.width(), rect.height()) * 0.5;
            d = std::min(d, 150);
            QRect circle(0, 0, d, d);
            circle.moveCenter(rect.center());

            QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
            gradient.setColorAt(0, QColor(0xFF9F2E));
            gradient.setColorAt(1, QColor(0xF08101));

            painter.setPen(Qt::NoPen);
            painter.setBrush(gradient);
            painter.drawEllipse(circle);

            QFont font = this->font();
            font.setPixelSize(d * 0.33);
            painter.setFont(font);
            painter.setPen(QPen(Qt::white));
            painter.drawText(circle, Qt::AlignCenter, text);
        }
    } else {
    }
}

}  // namespace module::meet