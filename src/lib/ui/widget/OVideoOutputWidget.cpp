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

#include "OVideoOutputWidget.h"

#include <QPainter>
namespace lib::ui {

OVideoOutputWidget::OVideoOutputWidget(QWidget* parent) : QWidget{parent} {}

void OVideoOutputWidget::render(const lib::video::VideoDevice& device) {
    qDebug() << __func__;

    lastFrame.reset();
    _camera = lib::video::CameraSource::CreateInstance(device);

    connect(_camera.get(), &lib::video::CameraSource::frameAvailable, this, [this](auto frame) {
        // qDebug() << "received frame:" << frame->sourceID;
        // std::lock_guard locker(mutex);
        lastFrame = std::move(frame);
        update();
    });

    connect(_camera.get(), &lib::video::CameraSource::sourceStopped, this, [&]() {
        qDebug() << "stopped";
        // Avoid dirty data affecting the next frame rendering
        lastFrame.reset();
    });

    modes = _camera->getVideoModes();
    if (modes.empty()) {
        qWarning() << "The video device have not valid modes!";
        return;
    }
    //Default to set first one video mode
    _camera->setup(modes.front());
    _camera->openDevice();
}

void OVideoOutputWidget::stopRender() {
    qDebug() << __func__;

    std::lock_guard locker(mutex);

    if (!_camera) {
        return;
    }

    disconnect(_camera.get());

    _camera->closeDevice();
    _camera.reset();

}

void OVideoOutputWidget::paintEvent(QPaintEvent* event) {
    // qDebug() << __func__;
    // std::lock_guard locker(mutex);

    QPainter painter(this);
    painter.fillRect(painter.viewport(), Qt::black);

    if (!lastFrame) {
        return;
    }

    if (lastFrame->image.isNull()) {
        return;
    }

            // qDebug() << "render:" << lastFrame->sourceID << lastFrame->image;
    painter.drawImage(rect(), lastFrame->image);
}


}
