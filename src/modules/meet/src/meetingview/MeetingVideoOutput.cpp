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
    // showAvatar();
    update();
}

void MeetingVideoOutput::showVideo() {
    if (avatarLabel) {
        avatarLabel->deleteLater();
        avatarLabel = nullptr;
    }
    update();
}

void MeetingVideoOutput::showAvatar() {
    // todo for lipeixu: 应检查图片是否存在，并构造控件，设置图片
    if (!avatarLabel) {
        avatarLabel = new RoundedPixmapLabel(this);
    }
    avatarLabel->setGeometry(calcAvatarRect().toRect());
    avatarLabel->raise();
}

bool MeetingVideoOutput::hasVideoOutput() {
    return false;
}

QRectF MeetingVideoOutput::calcAvatarRect() {
    QRectF rect = this->rect();
    int d = std::min(rect.width(), rect.height()) * 0.5;
    d = std::min(d, 150);
    QRectF circle(0, 0, d, d);
    circle.moveCenter(rect.center());
    return circle;
}

void MeetingVideoOutput::resizeEvent(QResizeEvent* event) {
    if (avatarLabel) {
        avatarLabel->setGeometry(calcAvatarRect().toRect());
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

    if (!hasVideoOutput() && !avatarLabel) {
        QString text = this->participant->getNick().right(2);
        if (!text.isEmpty()) {
            QRectF circle = calcAvatarRect();
            QLinearGradient gradient(circle.topLeft(), circle.bottomLeft());
            gradient.setColorAt(0, QColor(0xFF9F2E));
            gradient.setColorAt(1, QColor(0xF08101));

            painter.setPen(Qt::NoPen);
            painter.setBrush(gradient);
            painter.drawEllipse(circle);

            QFont font = this->font();
            font.setPixelSize(circle.height() * 0.33);
            painter.setFont(font);
            painter.setPen(QPen(Qt::white));
            painter.drawText(circle, Qt::AlignCenter, text);
        }
    } else {
    }
}

}  // namespace module::meet