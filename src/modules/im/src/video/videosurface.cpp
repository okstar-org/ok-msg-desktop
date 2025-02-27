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

#include "videosurface.h"
#include "base/SvgUtils.h"

#include "src/lib/storage/settings/style.h"

#include "src/model/friendlist.h"

#include "lib/video/videoframe.h"
#include "src/widget/friendwidget.h"

#include <QDebug>
#include <QLabel>
#include <QPainter>

/**
 * @var std::atomic_bool VideoSurface::frameLock
 * @brief Fast lock for lastFrame.
 */
namespace module::im {

float getSizeRatio(const QSize size) {
    return size.width() / static_cast<float>(size.height());
}

VideoSurface::VideoSurface(const QPixmap& avatar, QWidget* parent, bool expanding)
        : QWidget{parent}
        , source{nullptr}
        , frameLock{false}
        , avatar{avatar}
        , ratio{1.0f}
        , expanding{expanding} {
    recalulateBounds();
}

VideoSurface::VideoSurface(const QPixmap& avatar, lib::video::VideoSource* source, QWidget* parent)
        : VideoSurface(avatar, parent) {
    setSource(source);
}

VideoSurface::~VideoSurface() {
    unsubscribe();
}

bool VideoSurface::isExpanding() const {
    return expanding;
}

/**
 * @brief Update source.
 * @note nullptr is a valid option.
 * @param src source to set.
 *
 * Unsubscribe from old source and subscribe to new.
 */
void VideoSurface::setSource(const lib::video::VideoSource* src) {
    if (source == src) return;

            // unsubscribe();
    source = src;
    subscribe();
}

QRect VideoSurface::getBoundingRect() const {
    QRect bRect = boundingRect;
    bRect.setBottomRight(QPoint(boundingRect.bottom() + 1, boundingRect.right() + 1));
    return boundingRect;
}

float VideoSurface::getRatio() const {
    return ratio;
}

void VideoSurface::setAvatar(const QPixmap& pixmap) {
    avatar = pixmap;
    update();
}

QPixmap VideoSurface::getAvatar() const {
    return avatar;
}

void VideoSurface::subscribe() {
    if (!source) {
        return;
    }

    connect(source, &lib::video::VideoSource::frameAvailable, this,
            &VideoSurface::onNewFrameAvailable);
    connect(source, &lib::video::VideoSource::sourceStopped, this,
            &VideoSurface::onSourceStopped);

}

void VideoSurface::unsubscribe() {
    if (!source) return;

    lock();
    lastFrame.reset();
    unlock();

    ratio = 1.0f;
    recalulateBounds();
    emit ratioChanged();
    emit boundaryChanged();

    disconnect(source, &lib::video::VideoSource::frameAvailable, this,
               &VideoSurface::onNewFrameAvailable);
    disconnect(source, &lib::video::VideoSource::sourceStopped, this,
               &VideoSurface::onSourceStopped);

}

void VideoSurface::onNewFrameAvailable(const std::shared_ptr<lib::video::OVideoFrame>& newFrame) {

    lock();
    lastFrame = newFrame;
    QSize newSize = lastFrame->getSize();
    unlock();

    float newRatio = getSizeRatio(newSize);

    if (!qFuzzyCompare(newRatio, ratio) && isVisible()) {
        ratio = newRatio;
        recalulateBounds();
        emit ratioChanged();
        emit boundaryChanged();
    }

    update();
}

void VideoSurface::onSourceStopped() {
    // If the source's stream is on hold, just revert back to the avatar view
    lastFrame.reset();
    update();
}

void VideoSurface::paintEvent(QPaintEvent*) {
    lock();

    QPainter painter(this);
    painter.fillRect(painter.viewport(), Qt::black);
    if (lastFrame) {
        // QImage frame = lastFrame->toQImage(rect().size());
        // if (frame.isNull()) lastFrame.reset();
        // auto& img = lastFrame->image;
        // auto rec = rect();
        painter.drawImage(boundingRect, lastFrame->getImage(), lastFrame->getImage().rect(), Qt::NoFormatConversion);
    } else {
        painter.fillRect(boundingRect, Qt::white);
        QPixmap drawnAvatar = avatar;

        if (drawnAvatar.isNull())
            drawnAvatar = ok::base::SvgUtils::scaleSvgImage(
                    ":/img/contact_dark.svg", boundingRect.width(), boundingRect.height());

        painter.drawPixmap(boundingRect, drawnAvatar, drawnAvatar.rect());
    }

    unlock();
}

void VideoSurface::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    recalulateBounds();
    emit boundaryChanged();
}

void VideoSurface::showEvent(QShowEvent* e) {
    Q_UNUSED(e);
    emit ratioChanged();
}

void VideoSurface::recalulateBounds() {
    if (expanding) {
        boundingRect = contentsRect();
    } else {
        QPoint pos;
        QSize size;
        QSize usableSize = contentsRect().size();
        int possibleWidth = usableSize.height() * ratio;

        if (possibleWidth > usableSize.width())
            size = (QSize(usableSize.width(), usableSize.width() / ratio));
        else
            size = (QSize(possibleWidth, usableSize.height()));

        pos.setX(width() / 2 - size.width() / 2);
        pos.setY(height() / 2 - size.height() / 2);
        boundingRect.setRect(pos.x(), pos.y(), size.width(), size.height());
    }

    update();
}

void VideoSurface::lock() {
    // Fast lock
    bool expected = false;
    while (!frameLock.compare_exchange_weak(expected, true)) expected = false;
}

void VideoSurface::unlock() {
    frameLock = false;
}
}  // namespace module::im
