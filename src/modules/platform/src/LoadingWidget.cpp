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

#include "LoadingWidget.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimeLine>
#include <QVariantAnimation>

static constexpr qreal anima_delay = 0.2;
LoadingWidget::LoadingWidget(QWidget* target) : QWidget(target), anchorWidget(target) {
    Q_ASSERT(target);

    setVisible(false);
    target->installEventFilter(this);

    progressAnima = new QVariantAnimation(this);
    progressAnima->setStartValue(0.0);
    progressAnima->setEndValue(1 + anima_delay);
    progressAnima->setDuration(1400);
    progressAnima->setLoopCount(-1);
    connect(progressAnima, &QVariantAnimation::valueChanged, this, [this](QVariant value) {
        progress = value.toReal();
        update();
    });

    timeLine = new QTimeLine(progressAnima->duration(), this);
    timeLine->setEasingCurve(QEasingCurve::InOutQuart);
}

QSize LoadingWidget::sizeHint() const { return minimumSizeHint(); }

QSize LoadingWidget::minimumSizeHint() const { return contentSizeHint.grownBy(layoutMargins); }

void LoadingWidget::setMarginInTarget(const QMargins& margins) {
    if (this->layoutMargins != margins) {
        this->layoutMargins = margins;
    }
    updateWidgetGeo();
}

void LoadingWidget::setSizeHint(const QSize& hint) { contentSizeHint = hint; }

bool LoadingWidget::event(QEvent* e) {
    switch (e->type()) {
        case QEvent::Hide: {
            if (progressAnima) {
                progressAnima->stop();
            }
        } break;
        case QEvent::Show: {
            if (progressAnima) {
                progressAnima->start();
            }
        } break;
        default:
            break;
    }
    return QWidget::event(e);
}

void LoadingWidget::paintEvent(QPaintEvent* e) {
    QRectF r = QRectF(this->rect().marginsRemoved(layoutMargins)).adjusted(0.5, 0.5, -1, -1);
    qreal size = std::min(contentSizeHint.width(), contentSizeHint.height());

    // 开始绘制
    qreal pen_size = size / 60.0 * 8;
    QRectF paint_rect = QRectF(QPointF(0, 0), QSize(size - pen_size - 1, size - pen_size - 1));
    paint_rect.moveCenter(r.center());

    int slow = std::max(progress - anima_delay, 0.0) * timeLine->duration();
    int quick =  std::min(progress, 1.0) * timeLine->duration();
    qreal start_angle = -360 * timeLine->valueForTime(slow) + 90;
    qreal end_angle = -360 * timeLine->valueForTime(quick) + 90;
    QPainterPath path;
    path.arcMoveTo(paint_rect, start_angle);
    path.arcTo(paint_rect, start_angle, end_angle - start_angle);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(this->palette().windowText(), pen_size, Qt::SolidLine, Qt::RoundCap));
    painter.drawPath(path);
}

bool LoadingWidget::eventFilter(QObject* watched, QEvent* e) {
    switch (e->type()) {
        case QEvent::LayoutRequest:
        case QEvent::Resize:
            updateWidgetGeo();
            break;
        default:
            break;
    }
    return false;
}

void LoadingWidget::updateWidgetGeo() {
    if (anchorWidget) {
        QRect rect = anchorWidget->rect() - layoutMargins;
        this->setGeometry(rect);
        this->raise();
    }
}
