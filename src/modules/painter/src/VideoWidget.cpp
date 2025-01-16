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
#include "VideoWidget.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QMetaObject>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QStyleOption>
#include <QVariant>
#include <QWidget>

#include <base/logs.h>
#include <base/timer.h>
#include <base/utils.h>

#include "base/r.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

namespace module::painter {

VideoOverlayBar::VideoOverlayBar(VideoWidgetConfig& config, QWidget* parent)
        : QWidget(parent), _config(config) {
    setFixedWidth(parent->width());

    _hLayout = std::make_unique<QGridLayout>(this);
    _hLayout->setContentsMargins(0, 0, 0, 0);

    switch (config.size) {
        case VIDEO_SIZE::BIG:
            setFixedHeight(60);
            break;
        case VIDEO_SIZE::MIDDLE:
            setFixedHeight(32);
            break;
        case VIDEO_SIZE::SMALL:
            setFixedHeight(18);
            break;
    }

    // 语音信号灯
    _voice = std::make_shared<QLabel>(this);
    _voice->setFixedWidth(20);
    _voice->setPixmap(QPixmap("://resources/icon/video_voice.png"));
    _hLayout->setColumnStretch(0, 1);
    _hLayout->addWidget(_voice.get(), 0, 0, Qt::AlignCenter);

    if (config.size != VIDEO_SIZE::SMALL) {
        // 名字
        _name = std::make_shared<QLabel>(this);
        _name->setText(".");
        _hLayout->setColumnStretch(1, 5);
        _hLayout->addWidget(_name.get(), 0, 1);
    }

    // 弹簧
    //     _hLayout->addStretch();
    setStyleSheet(
            "		\
                  QWidget {border: none; background-color:rgba(0,0,0, 60 ); }		\
                  QWidget QLabel {color: white; background-color: transparent }	\
                  ");
}

VideoOverlayBar::~VideoOverlayBar() {}

void VideoOverlayBar::setMuteIcon(bool checked) {
    if (checked) {
        _voice->setPixmap(QPixmap("://resources/icon/video_voice.png"));
    } else {
        _voice->setPixmap(QPixmap("://resources/icon/video_voice_active.png"));
    }
}

void VideoOverlayBar::setName(const QString& name) {
    if (_name.get()) {
        _name->setText(name);
    }
}

void VideoOverlayBar::setVideoSize(VIDEO_SIZE size) {}

void VideoOverlayBar::setVideoFor(VIDEO_FOR videoFor) {
    switch (videoFor) {
        case VIDEO_FOR::STUDENT: {
            _hLayout->setContentsMargins(2, 0, 2, 0);

            // 金牌
            _goldCount = std::make_shared<QLabel>(this);
            _goldCount->setText("X0");
            _goldCount->setStyleSheet("font-size: 10px");

            _hLayout->addWidget(_goldCount.get());

            _silverIcon = std::make_shared<QLabel>(this);
            _silverIcon->setPixmap(QPixmap("://resources/icon/video_silver.png"));
            _hLayout->addWidget(_silverIcon.get());

            break;
        }
        case VIDEO_FOR::TEACHER:
            _hLayout->setContentsMargins(5, 0, 5, 0);

            // 时间
            _time = std::make_shared<QLabel>(this);
            _time->setText("00:00:00");
            _hLayout->addWidget(_time.get());

            break;
    }
}

void VideoOverlayBar::showEvent(QShowEvent* e) {
    Q_UNUSED(e);
}

/*视频覆盖层*/
VideoOverlay::VideoOverlay(const VideoWidgetConfig& config, QWidget* parent)
        : QFrame(parent), _config(config), _coin(0) {
    if (parent) {
        setFixedSize(parent->size());
    }

    // QPalette palette(this->palette());
    // palette.setColor(QPalette::Background, Qt::transparent);
    // this->setPalette(palette);

    // base::Widgets::SetPalette(this, QPalette::Background, Qt::green);
    // setAutoFillBackground(true);

    /*QRegion region(menu->x(),
                menu->y(),
                menu->sizeHint().width(),
                menu->sizeHint().height(),
                QRegion::Ellipse);*/

    // menu->setMask(region);

    setObjectName("video_overlay");

    _bar = new VideoOverlayBar(_config, this);

    QRect geo = this->geometry();
    _bar->move(0, geo.bottom() - _bar->height());

    _shadowEffect = std::make_shared<QGraphicsDropShadowEffect>(this);
    _shadowEffect->setOffset(0);
    _shadowEffect->setColor(Qt::white);
    _shadowEffect->setBlurRadius(10);
    _shadowEffect->setEnabled(false);

    this->setGraphicsEffect(_shadowEffect.get());

    //_painter = std::make_shared<QPainter>(this);
    // move(10, -10);
}

void VideoOverlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

void VideoOverlay::setCoin(int coin) {
    _coin = coin;
    QLabel* c = _bar->goldCount();
    if (c) c->setText(qsl("X%1").arg(_coin));
}

void VideoOverlay::setVideoSize(VIDEO_SIZE size) {
    if (size == VIDEO_SIZE::MIDDLE) {
        _zoomIn = std::make_shared<QLabel>(this);
        _zoomIn->setPixmap(QPixmap("://resources/icon/video_zoom_in.png"));
        _zoomIn->move(10, 10);
    }
}

VideoOverlay::~VideoOverlay() {}

VideoWidget::VideoWidget() {}

VideoWidget::VideoWidget(const VideoWidgetConfig& config, const QString& jid, QWidget* parent)
        : OVideoWidget(parent), _config(config), _jid(jid), _destoryed(false) {
    m_scale = 1;
    m_mode = PB_MODE::AUTO_ZOOM;
    m_brush = QBrush(QColor("#28292c"));

    _label = std::make_unique<QLabel>(this);
}

VideoWidget::~VideoWidget() {}

void VideoWidget::start() {
    //  DEBUG_LOG(("start for JID:%1").arg(qstring(peerJID())));
    switch (_config.mode) {
        case VIDEO_MODE::PLAYER: {
            //    TODO
            //    network::webrtc::OkRtcManager *rtcManager
            //    =network::NetworkManager::Get()->InitGetOkRtcManager();
            //    if (rtcManager) {
            //      rtcManager->setRemoteRenderer(std::shared_ptr<ortc::Renderer>(this));
            //    }
            break;
        }
        case VIDEO_MODE::CAMERA:
            //    network::webrtc::OkRtcManager *rtcManager =
            //        network::NetworkManager::Get()->InitGetOkRtcManager();
            //    if (rtcManager) {
            //      rtcManager->setLocalRenderer(std::shared_ptr<ortc::Renderer>(this));
            //    }
            break;
    }
}

void VideoWidget::stop() {
    //    switch (_wrap->videoMode())
    //    {
    //    case  VIDEO_MODE::PLAYER:
    //    {
    //        //        _player->stop();
    //        break;
    //    }
    //    case VIDEO_MODE::CAMERA:
    //        //        _camera->stop();
    //        break;
    //    }
}

void VideoWidget::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
}

void VideoWidget::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
}

void VideoWidget::setDisabled() {
    //  disableRender();
}

// void VideoWidget::disableRender() {
//     std::lock_guard<std::mutex> lock(_disabled_mutex);
//  if (!disabled()) {
//    QWidget::setDisabled(true);
//  }
//}

// bool VideoWidget::disabled() {
//     std::lock_guard<std::mutex> lock(_disabled_mutex);
//  return !QWidget::isEnabled();
//}

// void VideoWidget::setDestory(){
//     _destoryed = true;
// }

// bool VideoWidget::destory()
//{
//     return _destoryed;
// }

// void VideoWidget::onRender(ortc::RendererImage image) {
//   std::lock_guard<std::mutex> lock(_render_mutex);
//
//   if (disabled()) {
//     return;
//   }
//
//   if (0 < _buf_frames) {
//     DEBUG_LOG(("_buf_frames:%1").arg(_buf_frames));
//   }
//   _buf_frames++;
//   //  DEBUG_LOG(("RendererImage w:%1
//   //  h:%2").arg(image.bmi_.bmiHeader.biWidth).arg(image.bmi_.bmiHeader.biHeight));
//
//   // auto biw = image.bmi_.bmiHeader.biWidth,
//   // auto bih = -image.bmi_.bmiHeader.biHeight,
//
//   QImage qImage(image.image_.get(), image.width_, image.height_,
//                 QImage::Format_ARGB32);
//
//   m_pixmap = QPixmap::fromImage(qImage);
//
//   //    update();
//   QMetaObject::invokeMethod(this, "update");
// }

void VideoWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    std::lock_guard<std::mutex> lock(_render_mutex);
    _buf_frames--;

    //    if(_imageQueue.empty()){
    //        return;
    //    }

    //    QPixmap m_pixmap = _imageQueue.front();

    QPainter painter(this);
    painter.setBackground(m_brush);
    painter.eraseRect(rect());

    double window_width, window_height;
    double image_width, image_height;
    double r1, r2, r;
    int offset_x, offset_y;
    switch (m_mode) {
        case PB_MODE::FIXED_SIZE:
        case PB_MODE::AUTO_SIZE:
            painter.scale(m_scale, m_scale);
            painter.drawPixmap(0, 0, m_pixmap);
            break;
        case PB_MODE::FIX_SIZE_CENTRED:
            window_width = width();
            window_height = height();
            image_width = m_pixmap.width();
            image_height = m_pixmap.height();
            offset_x = (window_width - m_scale * image_width) / 2;
            offset_y = (window_height - m_scale * image_height) / 2;
            painter.translate(offset_x, offset_y);
            painter.scale(m_scale, m_scale);
            painter.drawPixmap(0, 0, m_pixmap);
            break;
        case PB_MODE::AUTO_ZOOM:

            window_width = width();
            window_height = height();
            image_width = m_pixmap.width();
            image_height = m_pixmap.height();
            r1 = window_width / image_width;
            r2 = window_height / image_height;
            r = qMin(r1, r2);
            offset_x = (window_width - r * image_width) / 2;
            offset_y = (window_height - r * image_height) / 2;
            painter.translate(offset_x, offset_y);
            painter.scale(r, r);
            painter.drawPixmap(0, 0, m_pixmap);

            break;
    }
}

// void VideoWidget::setDisabled(bool disabled)
//{
////    this->setDisabled();
//}

};  // namespace module::painter
