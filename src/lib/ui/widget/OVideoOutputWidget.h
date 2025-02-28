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

#ifndef OVIDEOOUTPUTWIDGET_H
#define OVIDEOOUTPUTWIDGET_H

#include <QWidget>

#include "lib/video/camerasource.h"
#include "lib/video/videoframe.h"

namespace lib::ui {

class OVideoOutputWidget : public QWidget {
    Q_OBJECT
public:
    explicit OVideoOutputWidget(QWidget* parent = nullptr);

    void render(const lib::video::VideoDevice& device);
    void stopRender();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::unique_ptr<lib::video::CameraSource> _camera;
    std::shared_ptr<lib::video::OVideoFrame> lastFrame;
    QVector<lib::video::VideoMode> modes;
    std::mutex mutex;
};
}
#endif  // OVIDEOOUTPUTWIDGET_H
