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

#include "flyoutoverlaywidget.h"

#include <QBitmap>
#include <QHBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>

FlyoutOverlayWidget::FlyoutOverlayWidget(QWidget* parent) : QWidget(parent) {
    setContentsMargins(0, 0, 0, 0);

    animation = new QPropertyAnimation(this, QByteArrayLiteral("flyoutPercent"), this);
    animation->setKeyValueAt(0, 0.0f);
    animation->setKeyValueAt(1, 1.0f);
    animation->setDuration(200);

    connect(animation, &QAbstractAnimation::finished, this,
            &FlyoutOverlayWidget::finishedAnimation);
    setFlyoutPercent(0);
    hide();
}

FlyoutOverlayWidget::~FlyoutOverlayWidget() {}

int FlyoutOverlayWidget::animationDuration() const { return animation->duration(); }

void FlyoutOverlayWidget::setAnimationDuration(int timeMs) { animation->setDuration(timeMs); }

qreal FlyoutOverlayWidget::flyoutPercent() const { return percent; }

void FlyoutOverlayWidget::setFlyoutPercent(qreal progress) {
    percent = progress;

    QSize self = size();
    setMask(QRegion(0, 0, self.width() * progress + 1, self.height()));
    move(startPos.x() + self.width() - self.width() * percent, startPos.y());
    setVisible(progress != 0);
}

bool FlyoutOverlayWidget::isShown() const { return (percent == 1); }

bool FlyoutOverlayWidget::isBeingAnimated() const {
    return (animation->state() == QAbstractAnimation::Running);
}

bool FlyoutOverlayWidget::isBeingShown() const {
    return (isBeingAnimated() && animation->direction() == QAbstractAnimation::Forward);
}

void FlyoutOverlayWidget::animateShow() {
    if (percent == 1.0f) return;

    if (animation->state() != QAbstractAnimation::Running) this->startPos = pos();

    startAnimation(true);
}

void FlyoutOverlayWidget::animateHide() {
    if (animation->state() != QAbstractAnimation::Running) this->startPos = pos();

    startAnimation(false);
}

void FlyoutOverlayWidget::finishedAnimation() {
    bool hide = (animation->direction() == QAbstractAnimation::Backward);

    // Delay it by a few frames to let the system catch up on rendering
    if (hide) QTimer::singleShot(50, this, SIGNAL(hidden()));
}

void FlyoutOverlayWidget::startAnimation(bool forward) {
    setAttribute(Qt::WA_TransparentForMouseEvents, !forward);
    animation->setDirection(forward ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    animation->start();
    animation->setCurrentTime(animation->duration() * percent);
}
