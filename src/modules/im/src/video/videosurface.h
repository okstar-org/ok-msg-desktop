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
#include "lib/video/videosource.h"
namespace module::im {

class VideoSurface : public QWidget {
    Q_OBJECT

public:
    VideoSurface(const QPixmap& avatar, QWidget* parent = nullptr, bool expanding = false);
    VideoSurface(const QPixmap& avatar, lib::video::VideoSource* source, QWidget* parent = nullptr);
    ~VideoSurface();

    bool isExpanding() const;
    void setSource(const lib::video::VideoSource* src);
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

    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onNewFrameAvailable(const std::shared_ptr<lib::video::OVideoFrame>& newFrame);
    void onSourceStopped();

private:
    void recalulateBounds();
    void lock();
    void unlock();

    QRect boundingRect;
    const lib::video::VideoSource* source;
    std::shared_ptr<lib::video::OVideoFrame> lastFrame;
    std::atomic_bool frameLock;
    QPixmap avatar;
    volatile float ratio;
    bool expanding;
};
}  // namespace module::im
#endif  // SELFCAMVIEW_H
