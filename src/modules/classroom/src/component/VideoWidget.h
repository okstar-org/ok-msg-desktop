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
#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <memory>
#include <mutex>
#include <queue>

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QWidget>
#include "OVideoWidget.h"
#include "src/modules/classroom/src/ui.h"

namespace module::classroom {

class PlayerWidget;
class VideoOverlay;

class VideoWidget : public OVideoWidget  //, public ortc::Renderer
{
    Q_OBJECT
public:
    enum class PB_MODE { FIXED_SIZE, FIX_SIZE_CENTRED, AUTO_ZOOM, AUTO_SIZE };

    explicit VideoWidget(const VideoWidgetConfig& config, QWidget* parent = nullptr);

    ~VideoWidget() override;

    void start();

    void stop();

    virtual void setDisabled();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

private:
    VideoWidgetConfig _config;
    QString _jid;

    QPixmap m_pixmap;
    double m_scale;
    PB_MODE m_mode;
    QBrush m_brush;

    std::unique_ptr<QLabel> _label;

    std::mutex _render_mutex;
    std::mutex _disabled_mutex;

    int _buf_frames;

    bool _destoryed;
};

class VideoOverlayBar : public QWidget {
public:
    VideoOverlayBar(VideoWidgetConfig& config, QWidget* parent);
    virtual ~VideoOverlayBar() override;

    virtual void setMuteIcon(bool checked);

    virtual void setName(const QString& name);

    inline QLabel* goldCount() {
        return _goldCount.get();
    }

    void setVideoSize(VIDEO_SIZE size);

    void setVideoFor(VIDEO_FOR videoFor);

protected:
    virtual void showEvent(QShowEvent* e) override;

private:
    VideoWidgetConfig _config;

    std::unique_ptr<QGridLayout> _hLayout;

    std::shared_ptr<QLabel> _voice;
    std::shared_ptr<QLabel> _name;
    std::shared_ptr<QLabel> _time;

    std::shared_ptr<QLabel> _goldCount;
    std::shared_ptr<QLabel> _silverIcon;
};

/*视频覆盖层*/
class VideoOverlay : public QFrame {
    Q_OBJECT
public:
    explicit VideoOverlay(const VideoWidgetConfig& config, QWidget* parent = nullptr);
    ~VideoOverlay();

    void paintEvent(QPaintEvent* event);

    PlayerWidget* wrap() {
        return _wrap;
    }

    QGraphicsDropShadowEffect* shadowEffect() {
        return _shadowEffect.get();
    }

    QPainter* painter() {
        return _painter.get();
    }

    inline int coin() {
        return _coin;
    }

    inline VideoOverlayBar* bar() {
        return _bar;
    }

    void setCoin(int coin);

    void setVideoSize(VIDEO_SIZE size);

private:
    VideoWidgetConfig _config;

    int _coin;

    PlayerWidget* _wrap;

    VideoOverlayBar* _bar;

    std::shared_ptr<QLabel> _zoomIn;

    std::shared_ptr<QGraphicsDropShadowEffect> _shadowEffect;

    std::shared_ptr<QPainter> _painter;
};

// class PlayerWidget : public MoveableBar, public VideoBase
//{
//     Q_OBJECT
// public:
//     PlayerWidget(
//             QWidget* parent,
//             QLayout* layout,
//             QGridLayout* gLayout,
//             DelayedCallTimer* _delayCaller,
//             VIDEO_MODE mode = VIDEO_MODE::NONE,
//             VIDEO_SIZE size = VIDEO_SIZE::NONE,
//             VIDEO_FOR videoFor = VIDEO_FOR::NONE);

//    virtual ~PlayerWidget() override;

//    void setViewSize(VIDEO_SIZE size);

//    void start();

//    void stop();

//    void setBorderRed();
//    void setBorderBlue();

//    void setChecked(bool checked);
//    void setMute(bool checked);

//    inline virtual base::DelayedCallTimer* delayCaller() {
//        return _delayCaller;
//    }

//    virtual const UserJID& peerJID(){
//        return _jid;
//    }
//    virtual void setPeerJID(const UserJID& jid){
//        _jid = jid;
//    }

//    QFrame* shadow();

// protected:

//    virtual void showEvent(QShowEvent* e) override;
//    virtual void mousePressEvent(QMouseEvent *event) override;
//    virtual void mouseReleaseEvent(QMouseEvent *event) override;
//    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

// private:

//    bool _checked = false;

//    bool _mute = false;

//    QWidget* _parent = nullptr;

//    QLayout* _frame = nullptr;

//    QGridLayout* _gLayout = nullptr;

//    base::DelayedCallTimer* const _delayCaller;

//    painter::WhiteboardController* _controller = nullptr;

//    QFrame* _shadow = nullptr;

//    UI::WindowManager *_pageManager = nullptr;

//    std::unique_ptr<VideoWidget> _videoWidget;

//    std::unique_ptr<VideoOverlay> _overlay;

//    UserJID _jid;

// signals:

// public slots:
//     void onCtrollerChecked(context::WB::WB_CTRL, bool);

//    void goBack();

//    void addSilverCoin(int coin);

//};
}  // namespace module::classroom

#endif
