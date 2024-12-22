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

#ifndef MEETINGVIDEORENDER_H
#define MEETINGVIDEORENDER_H

#include <QImage>
#include <atomic>
#include <mutex>
#include "lib/ortc/ok_rtc.h"

namespace module::meet {

class MeetingVideoRender {
public:
    virtual void renderImage(const lib::ortc::RendererImage& image) = 0;
    virtual ~MeetingVideoRender() {}
};

class EmptyVideoRender : public MeetingVideoRender {
public:
    static EmptyVideoRender* instance();

private:
    void renderImage(const lib::ortc::RendererImage& rawimage) override {};
};

class MeetingVideoWidgetRender : public MeetingVideoRender {
public:
    MeetingVideoWidgetRender(QWidget* widget);
    void setRenderEnable(bool enable);
    bool renderEnable() const;

    void begin();
    const QImage& image();
    void end();

private:
    void renderImage(const lib::ortc::RendererImage& rawimage) override;
    QWidget* w = nullptr;
    std::atomic<bool> _enable = false;

    std::mutex _mutex;
    std::atomic<bool> swapFlag = false;
    QImage _image;
    QImage _image2;
};
}  // namespace module::meet
#endif  // !MEETINGVIDEORENDER_H
