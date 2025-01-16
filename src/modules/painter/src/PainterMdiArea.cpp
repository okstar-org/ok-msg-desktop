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

#include "PainterMdiArea.h"
#include "SharedPainterScene.h"

#include <QApplication>
#include <QGraphicsSceneDragDropEvent>
#include <QMdiSubWindow>
#include <QMouseEvent>

#include "src/base/logs.h"

namespace module::painter {

PainterMdiArea::PainterMdiArea(std::shared_ptr<CSharedPainterScene> scene, QWidget* parent)
        : QMdiArea(parent), scene_(scene) {
    this->setAcceptDrops(true);

    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    setBackground(QBrush(Qt::transparent));

    if (parent) setFixedSize(parent->size());

    connect(scene_.get(), SIGNAL(dropEvent()), this, SLOT(onDropEvent()));

    connect(scene_.get(), SIGNAL(dragLeaveEvent()), this, SLOT(onDragLeaveEvent()));

    this->installEventFilter(this);
}

PainterMdiArea::~PainterMdiArea() {}

void PainterMdiArea::mouseReleaseEvent(QMouseEvent* e) {
    forwardEvent(e);
}

void PainterMdiArea::mouseMoveEvent(QMouseEvent* e) {
    forwardEvent(e);
}

void PainterMdiArea::mousePressEvent(QMouseEvent* e) {
    setFocus();
    forwardEvent(e);
}

void PainterMdiArea::keyPressEvent(QKeyEvent* e) {
    forwardEvent(e);
}

void PainterMdiArea::forwardEvent(QMouseEvent* e) {
    //    qDebug() << e;

    if (!parentWidget()) {
        return;
    }

    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QPoint pt = this->mapTo(this->parentWidget(), e->pos());
    QWidget* w = this->parentWidget()->childAt(pt);
    if (w) {
        pt = w->mapFrom(this->parentWidget(), pt);
        QMouseEvent* event = new QMouseEvent(*e);
        QApplication::postEvent(w, event);
    }

    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void PainterMdiArea::forwardEvent(QKeyEvent* e) {
    if (!parentWidget()) {
        return;
    }

    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QKeyEvent* event = new QKeyEvent(*e);

    QApplication::postEvent(scene_.get(), event);

    this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void PainterMdiArea::dragEnterEvent(QDragEnterEvent* e) {
    forwardEvent(e);
}

void PainterMdiArea::dragLeaveEvent(QDragLeaveEvent* e) {
    forwardEvent(e);
}

void PainterMdiArea::dragMoveEvent(QDragMoveEvent* e) {
    forwardEvent(e);
}

void PainterMdiArea::dropEvent(QDropEvent* e) {
    forwardEvent(e);
}

void PainterMdiArea::forwardEvent(QDragEnterEvent* e) {
    qDebug() << e;
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void PainterMdiArea::forwardEvent(QDragLeaveEvent* e) {
    qDebug() << e;
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void PainterMdiArea::forwardEvent(QDragMoveEvent* e) {
    qDebug() << e;
}

void PainterMdiArea::forwardEvent(QDropEvent* e) {
    qDebug() << e;
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void PainterMdiArea::onDropEvent() {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void PainterMdiArea::onDragLeaveEvent() {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

bool PainterMdiArea::eventFilter(QObject* _watcher, QEvent* _event) {
    if (_watcher != this) return false;

    if (QEvent::InputMethod == _event->type())  // 输入法事件
    {
        QApplication::postEvent(scene_.get(),
                                new QInputMethodEvent(*static_cast<QInputMethodEvent*>(_event)));
        return true;
    }

    return QMdiArea::eventFilter(_watcher, _event);
}

}  // namespace module::painter
