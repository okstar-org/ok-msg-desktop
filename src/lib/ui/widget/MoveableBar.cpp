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
#include <QBoxLayout>
#include <QToolButton>

namespace lib::ui {

MoveableBar::MoveableBar(QWidget* parent) : QWidget(parent), _moveable(true), m_target(nullptr) {
    setFixedSize(QSize(30, 30));
    setAttribute(Qt::WA_StyledBackground, true);

    auto layout = new QGridLayout(this);
    bar = new QLabel(this);

    QPixmap pixmap(":/res/icon/move.svg");
    QPixmap scaledPixmap = pixmap.scaled(size(), Qt::KeepAspectRatio);
    bar->setPixmap(scaledPixmap);
    bar->setObjectName("bar");
    layout->addWidget(bar);
    setLayout(layout);
    // setStyleSheet("QLabel#bar{ background-image: url(:/res/icon/move.svg); }");
}

MoveableBar::~MoveableBar() = default;

void MoveableBar::mousePressEvent(QMouseEvent* event) {
    if (_moveable) {
        m_isPressed = true;
        m_startMovePos = event->globalPos();
    }
    return QWidget::mousePressEvent(event);
}

void MoveableBar::mouseMoveEvent(QMouseEvent* event) {
    if (m_isPressed) {
        auto p = m_target ? m_target : parentWidget();
        if (!p) {
            // qWarning() <<"Have no moveable target!";
            return;
        }

        QPoint movePoint = event->globalPos() - m_startMovePos;
        QPoint widgetPos = p->pos() + movePoint;
        m_startMovePos = event->globalPos();
        p->move(widgetPos.x(), widgetPos.y());

    }
    return QWidget::mouseMoveEvent(event);
}

void MoveableBar::mouseReleaseEvent(QMouseEvent* event) {
    m_isPressed = false;
    return QWidget::mouseReleaseEvent(event);
}

bool MoveableBar::eventFilter(QObject *obj, QEvent *event)
{
    return true;
}

void MoveableBar::setTarget(QWidget* target) {
    m_target = target;
}

void MoveableBar::setMoveable(bool moveable) {
    _moveable = moveable;
}

}  // namespace UI
