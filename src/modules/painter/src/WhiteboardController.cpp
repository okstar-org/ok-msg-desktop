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

#include "WhiteboardController.h"

#include <QBrush>
#include <QPixmap>

#include "src/base/basic_types.h"
#include "src/base/utils.h"

namespace module::painter {

static WB_CTRL_BTN btns[] = {
        {WB_CTRL::MOVE, 32, 38, "://res/icon/painter/btn_ctrl_plus.png",
         "://res/icon/painter/btn_ctrl_plus_active.png", true, true},
        {WB_CTRL::MUTE, 64, 20, "://res/icon/painter/btn_ctrl_mute.png",
         "://res/icon/painter/btn_ctrl_mute_active.png", false, true},
        {WB_CTRL::GIFT, 32, 1, "://res/icon/painter/btn_ctrl_gift.png",
         "://res/icon/painter/btn_ctrl_gift_active.png", false, false},
        {WB_CTRL::RE, 64, 56, "://res/icon/painter/btn_ctrl_refresh.png",
         "://res/icon/painter/btn_ctrl_refresh_active.png", false, false},
        {WB_CTRL::WB, 0, 56, "://res/icon/painter/btn_ctrl_wb.png",
         "://res/icon/painter/btn_ctrl_wb_active.png", false, true},
        {WB_CTRL::OK, 0, 20, "://res/icon/painter/btn_ctrl_ok.png",
         "://res/icon/painter/btn_ctrl_ok_active.png", false, true},
        {WB_CTRL::ALL, 32, 75, "://res/icon/painter/btn_ctrl_all.png",
         "://res/icon/painter/btn_ctrl_all_active.png", false, true},
};

WhiteboardControllerButton::WhiteboardControllerButton(QWidget* parent) : QLabel(parent) {
    setFixedSize(QSize(45, 35));
    setAlignment(Qt::AlignCenter);
    // Utils::SetPalette(this, QPalette::Background,
    // QBrush(QPixmap("://res/icon/painter/btn_ctrl_bg.png")));

    setStyleSheet("background: url(://res/icon/painter/btn_ctrl_bg.png) no-repeat center;");
}

WhiteboardControllerButton::~WhiteboardControllerButton() {}

void WhiteboardControllerButton::setType(WB_CTRL type) {
    _type = type;
}

void WhiteboardControllerButton::setIcon(QString& icon) {
    _icon = icon;
}

void WhiteboardControllerButton::setActiveIcon(QString& activeIcon) {
    _activeIcon = activeIcon;
}

void WhiteboardControllerButton::showEvent(QShowEvent* event) {}

void WhiteboardControllerButton::mousePressEvent(QMouseEvent* event) {
    if (_moveable) {
        m_isMoved = false;
        m_isPressed = true;
        m_startMovePos = event->globalPos();
    }
}

void WhiteboardControllerButton::mouseMoveEvent(QMouseEvent* event) {
    if (m_isPressed) {
        m_isMoved = true;

        QWidget* p = m_target;

        if (!p) {
            p = this->parentWidget();
        }

        if (p) {
            QPoint movePoint = event->globalPos() - m_startMovePos;
            QPoint widgetPos = p->pos() + movePoint;
            m_startMovePos = event->globalPos();
            p->move(widgetPos.x(), widgetPos.y());
        }
    }
    return QWidget::mouseMoveEvent(event);
}

void WhiteboardControllerButton::mouseReleaseEvent(QMouseEvent* event) {
    m_isPressed = false;
    // 设置状态
    if ((_moveable && !m_isMoved) || !_moveable) {
        setChecked(!_isChecked);

        emit clicked();
    }
}

void WhiteboardControllerButton::setTarget(QWidget* target) {
    m_target = target;
}

void WhiteboardControllerButton::setMoveable(bool moveable) {
    _moveable = moveable;
}

void WhiteboardControllerButton::leaveEvent(QEvent* event) {
    if (_isChecked) {
        return;
    }
    setPixmap(QPixmap(_icon));
}

void WhiteboardControllerButton::enterEvent(QEvent* event) {
    if (_isChecked) {
        return;
    }
    setPixmap(QPixmap(_activeIcon));
}

void WhiteboardControllerButton::setChecked(bool checked) {
    if (!_checkable) {
        return;
    }

    _isChecked = checked;
    if (_isChecked) {
        setPixmap(QPixmap(_activeIcon));
    } else {
        setPixmap(QPixmap(_icon));
    }
}

WhiteboardController::WhiteboardController(QWidget* parent) : QWidget(parent) {
    setFixedSize(QSize(110, 140));
    int count = ARRAY_LENGTH_OF(btns);
    for (int i = 0; i < count; i++) {
        WB_CTRL_BTN _b = btns[i];

        WhiteboardControllerButton* btn = new WhiteboardControllerButton(this);
        btn->move(_b.x, _b.y);
        btn->setType(_b.type);
        btn->setIcon(_b.icon);
        btn->setActiveIcon(_b.activeIcon);
        btn->setMoveable(_b.moveable);
        btn->setCheckable(_b.checkable);
        btn->setPixmap(QPixmap(btn->icon()));
        btn->hide();

        if (_b.type == WB_CTRL::MOVE) {
            setCursor(Qt::PointingHandCursor);
            btn->setTarget(this);
            btn->show();
        }

        buttons.push_back(btn);

        connect(btn, &WhiteboardControllerButton::clicked, this, [=] {
            WhiteboardControllerButton* sender =
                    qobject_cast<WhiteboardControllerButton*>(QObject::sender());

            switch (sender->type()) {
                case WB_CTRL::MOVE: {
                    for (WhiteboardControllerButton* n : buttons) {
                        if (sender->isChecked()) {
                            if (n != sender) {
                                n->show();
                            }
                        } else {
                            if (n != sender) {
                                n->hide();
                            }
                        }
                    }
                    break;
                }

                case WB_CTRL::RE: {
                    emit checked(WB_CTRL::RE, sender->isChecked());
                    break;
                }

                case WB_CTRL::OK: {
                    emit checked(WB_CTRL::OK, sender->isChecked());
                    break;
                }

                case WB_CTRL::GIFT: {
                    emit checked(WB_CTRL::GIFT, sender->isChecked());
                    break;
                }

                case WB_CTRL::MUTE: {
                    emit checked(WB_CTRL::MUTE, sender->isChecked());
                    break;
                }

                case WB_CTRL::WB: {
                    emit checked(WB_CTRL::WB, sender->isChecked());
                    break;
                }
                case WB_CTRL::ALL:
                    emit checked(WB_CTRL::ALL, sender->isChecked());
                    break;
            }
        });
    }
}

WhiteboardController::~WhiteboardController() {}

void WhiteboardController::ctrl(int i) {}

}  // namespace module::painter
