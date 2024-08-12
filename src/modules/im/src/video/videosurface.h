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

#ifndef SELFCAMVIEW_H
#define SELFCAMVIEW_H

#include <QWidget>
#include <atomic>
#include <memory>
#include "src/video/videosource.h"

class VideoSurface : public QWidget {
    Q_OBJECT

public:
    VideoSurface(const QPixmap& avatar, QWidget* parent = nullptr, bool expanding = false);
    VideoSurface(const QPixmap& avatar, VideoSource* source, QWidget* parent = nullptr);
    ~VideoSurface();

    bool isExpanding() const;
    void setSource(VideoSource* src);
    QRect getBoundingRect() const;
    float getRatio() const;
    void setAvatar(const QPixmap& pixmap);
    QPixmap getAvatar() const;

signals:
    void ratioChanged();
    void boundaryChanged();

protected:
    void subscribe();
    void unsubscribe();

    virtual void paintEvent(QPaintEvent* event) final override;
    virtual void resizeEvent(QResizeEvent* event) final override;
    virtual void showEvent(QShowEvent* event) final override;

private slots:
    void onNewFrameAvailable(const std::shared_ptr<VideoFrame>& newFrame);
    void onSourceStopped();

private:
    void recalulateBounds();
    void lock();
    void unlock();

    QRect boundingRect;
    VideoSource* source;
    std::shared_ptr<VideoFrame> lastFrame;
    std::atomic_bool frameLock;
    uint8_t hasSubscribed;
    QPixmap avatar;
    volatile float ratio;
    bool expanding;
};

#endif  // SELFCAMVIEW_H
