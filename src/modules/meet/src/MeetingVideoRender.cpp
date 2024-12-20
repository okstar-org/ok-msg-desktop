#include "MeetingVideoRender.h"
#include <QWidget>
#include "src/lib/ortc/utils/image_convert.h"

EmptyVideoRender* EmptyVideoRender::instance() {
    static EmptyVideoRender r;
    return &r;
}

MeetingVideoWidgetRender::MeetingVideoWidgetRender(QWidget* widget) : w(widget) {}

void MeetingVideoWidgetRender::setRenderEnable(bool enable) {
    _enable.store(enable);
}

bool MeetingVideoWidgetRender::renderEnable() const {
    return _enable;
}

void MeetingVideoWidgetRender::begin() {
    this->_mutex.lock();
}

const QImage& MeetingVideoWidgetRender::image() {
    return swapFlag ? _image : _image2;
}

void MeetingVideoWidgetRender::end() {
    this->_mutex.unlock();
}

void MeetingVideoWidgetRender::renderImage(const lib::ortc::RendererImage& rawimage) {
    if (!_enable) {
        return;
    }

    // 与image接口相反
    QImage* image = swapFlag ? &_image2 : &_image;
    if (image->width() != rawimage.width_ && image->height() != rawimage.height_) {
        *image = QImage(rawimage.width_, rawimage.height_, QImage::Format_ARGB32);
        image->fill(0);
    }

    // 使用 libyuv 的函数进行转换
    lib::ortc::image_convert::yuv420_to_argb(rawimage.y, rawimage.ystride, rawimage.u,
                                             rawimage.ustride, rawimage.v, rawimage.vstride,
                                             image->bits(), rawimage.width_ * 4, rawimage.width_,
                                             rawimage.height_);

    if (_mutex.try_lock()) {
        swapFlag = !swapFlag;
        _mutex.unlock();
    }
    if (w) {
        w->update();
    }
}
