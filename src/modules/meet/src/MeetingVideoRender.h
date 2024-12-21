#ifndef MEETINGVIDEORENDER_H
#define MEETINGVIDEORENDER_H

#include "lib/ortc/ok_rtc_defs.h"
#include <atomic>
#include <mutex>

#include <QImage>

class MeetingVideoRender {
public:
    virtual void renderImage(const lib::ortc::RendererImage& image) = 0;
    virtual ~MeetingVideoRender(){}
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

#endif  // !MEETINGVIDEORENDER_H
