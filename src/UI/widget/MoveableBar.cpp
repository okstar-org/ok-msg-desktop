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
#include "MoveableBar.h"

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>

namespace UI {

MoveableBar::MoveableBar(QWidget* parent) : QWidget(parent), _moveable(true), m_target(nullptr) {}

MoveableBar::~MoveableBar() {}

void MoveableBar::mousePressEvent(QMouseEvent* event) {
    if (_moveable) {
        m_isPressed = true;
        m_startMovePos = event->globalPos();
    }
    return QWidget::mousePressEvent(event);
}

void MoveableBar::mouseMoveEvent(QMouseEvent* event) {
    if (m_isPressed) {
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

void MoveableBar::mouseReleaseEvent(QMouseEvent* event) {
    m_isPressed = false;
    return QWidget::mouseReleaseEvent(event);
}

void MoveableBar::setTarget(QWidget* target) { m_target = target; }

void MoveableBar::setMoveable(bool moveable) { _moveable = moveable; }

}  // namespace UI
