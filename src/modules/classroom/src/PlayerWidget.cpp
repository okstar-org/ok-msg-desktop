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
#include "PlayerWidget.h"
#include "Base.h"
#include "base/jid.h"
#include "base/logs.h"
#include "src/lib/ui/ui.h"

namespace module::classroom {

PlayerWidgetShadow::PlayerWidgetShadow(PlayerWidget* player, QWidget* parent)
        : QFrame(parent), _player(player) {}

PlayerWidget::PlayerWidget(VideoWidgetConfig config, QWidget* parent)
        : QWidget(parent), _config(config), _shadow(nullptr) {
    // 设置大小策略
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setViewSize(config.size);

    _shadow = new PlayerWidgetShadow(this, this);
    _shadow->setGeometry(geometry());
    _shadow->setFixedSize(geometry().size());
    _shadow->setAttribute(Qt::WA_StyledBackground, true);
    _shadow->setStyleSheet("background-color: rgb(255,255, 255)");

    //    _videoWidget = new VideoWidget(_config, peerJID().node(), this);
    //    _overlay = new VideoOverlay(_config, this);
    //    _pageManager = (UI::WindowManager::Instance());
}

PlayerWidget::~PlayerWidget() {}

void PlayerWidget::setViewSize(VIDEO_SIZE size) {
    switch (size) {
        case VIDEO_SIZE::SMALL:
            this->setFixedSize(QSize(75, 75));
            break;
        case VIDEO_SIZE::MIDDLE:
            this->setFixedSize(QSize(156, 156));
            break;
        default:
            break;
    }
}

void PlayerWidget::start() {
    //    _videoWidget->start();
}

void PlayerWidget::stop() {
    //    _videoWidget->stop();
}

void PlayerWidget::setBorderRed() {
    QString color("#d8564b");

    _overlay->setStyleSheet(qsl("#video_overlay { border: 2px solid %1; }").arg(color));

    auto effect = _overlay->shadowEffect();
    effect->setColor(color);
    effect->setEnabled(true);

    /*auto _painter = _overlay->painter();
            _painter->setRenderHint(QPainter::Antialiasing, true);
            _painter->setPen(QPen(QColor(color), 2));
            _painter->setBrush(QColor(color));
            _painter->drawEllipse(QPointF(this->width() - 10, 10), 10, 10);*/
}

void PlayerWidget::setBorderBlue() {
    QString color("#61c554");

    _overlay->setStyleSheet(qsl("#video_overlay { border: 2px solid %1; }").arg(color));
    auto effect = _overlay->shadowEffect();
    effect->setColor(color);
    effect->setEnabled(true);
}

void PlayerWidget::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    //    MoveableBar::mousePressEvent(event);
}

void PlayerWidget::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    //    if (!_moveable) {
    //        setChecked(!_checked);
    //    }
    //    MoveableBar::mouseReleaseEvent(event);
}

void PlayerWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    setChecked(false);
}

void PlayerWidget::resizeEvent(QResizeEvent* event) {
    //    _videoWidget->resize(event->size().width(), event->size().height());
}

void PlayerWidget::setChecked(bool checked) {
    if (!_gLayout) {
        return;
    }

    if (checked && _checked) {
        return;
    }

    _checked = checked;

    if (checked) {
        //        setMoveable(true);

        _overlay->setStyleSheet(qsl("#video_overlay { border: 2px solid #61c554; }"));

        if (0 < _gLayout->indexOf(this)) {
            setWindowFlags(Qt::FramelessWindowHint);
            show();
        }

        _gLayout->replaceWidget(this, _shadow);

        //    const QFrame *page = _pageManager->getPage(PageMenu::classing);
        //    if (page) {
        //      this->setParent(const_cast<QFrame *>(page));
        //    }

        QRect geo = this->geometry();
        this->move(geo.left(), geo.top());
        this->show();
    } else {
        //        setMoveable(false);
        setWindowFlags(Qt::Widget);
        if (_gLayout) _gLayout->replaceWidget(_shadow, this);
        // 关闭边框
        _overlay->setStyleSheet(qsl("#video_overlay { border: none }"));
    }
}

void PlayerWidget::setMute(bool checked) {
    // TODO
    //   auto im = lib::IM::Messenger::getInstance()->im();
    //  if (!im) {
    //    return;
    //  }
    //
    //  if (checked) {
    //    im->setRole(_jid, gloox::MUCRoomRole::RoleVisitor);
    //  } else {
    //    im->setRole(_jid, gloox::MUCRoomRole::RoleParticipant);
    //  }

    _overlay->bar()->setMuteIcon(checked);
}

void PlayerWidget::setPeerStatus(const PeerStatus& status) {
    if (_overlay) {
        _overlay->bar()->setMuteIcon(status.audioMuted);
    }
    setChecked(status.raisedHand);
}

void PlayerWidget::setPeerName(const QString& name) {
    if (_overlay) {
        _overlay->bar()->setName(name);
    }
}

void PlayerWidget::setDisabled() {
    if (_videoWidget) {
        _videoWidget->setDisabled(true);
    }
}

void PlayerWidget::showEvent(QShowEvent* e) {
    Q_UNUSED(e);
    _showTimes++;
    if (_showTimes == 1) {
        //        painter::Painter *painter = painter::Painter::Get();
        //        if(!painter){
        //            DEBUG_LOG_S(L_ERROR) <<"painter is null";
        //            return;
        //        }

        //        const painter::WhiteboardController *_controller =
        //        painter->controller(); connect(_controller,
        //        SIGNAL(checked(WB_CTRL, bool)),
        //                SLOT(onCtrollerChecked(WB_CTRL, bool)));
    }
}

//void PlayerWidget::onCtrollerChecked(WB_CTRL ctrl, bool isChecked) {
//    switch (ctrl) {
//        case WB_CTRL::MOVE:
//            break;
//        case WB_CTRL::MUTE:
//            setMute(isChecked);
//            break;
//        case WB_CTRL::WB:
//            break;
//        case WB_CTRL::RE:
//            setChecked(false);
//            break;
//        case WB_CTRL::OK:
//            setChecked(isChecked);
//            break;
//        case WB_CTRL::GIFT:
//            addSilverCoin(1);
//            break;
//        case WB_CTRL::ALL:
//            break;
//    }
//}

void PlayerWidget::goBack() {}

void PlayerWidget::addSilverCoin(int coin) {
    if (!_checked) {
        return;
    }
    if (_overlay) {
        _overlay->setCoin(_overlay->coin() + coin);
    }
}

}  // namespace module::classroom
