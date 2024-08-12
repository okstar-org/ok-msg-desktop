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

#include "maskablepixmapwidget.h"
#include <QPainter>
#include <QStyle>

/**
 * @var QPixmap* MaskablePixmapWidget::renderTarget
 * @brief pointer to dynamically call the constructor.
 */

MaskablePixmapWidget::MaskablePixmapWidget(QWidget* parent, QSize size, QString maskName)
        : QLabel("", parent), renderTarget(nullptr), maskName(maskName), clickable(false) {
    setSize(size);
}

MaskablePixmapWidget::~MaskablePixmapWidget() { delete renderTarget; }

void MaskablePixmapWidget::setClickable(bool clickable) {
    this->clickable = clickable;

    if (clickable) {
        setCursor(Qt::PointingHandCursor);
    } else {
        unsetCursor();
    }
}

void MaskablePixmapWidget::setPixmap(const QPixmap& pmap) {
    if (pmap.isNull()) {
        return;
    }

    unscaled = pmap;
    pixmap = pmap.scaled(
            size() * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    updatePixmap();
    update();
}

QPixmap MaskablePixmapWidget::getPixmap() const { return *renderTarget; }

void MaskablePixmapWidget::setSize(QSize size) {
    setFixedSize(size);
    delete renderTarget;

    QSize targetSize = size * devicePixelRatioF();
    renderTarget = new QPixmap(targetSize);

    QPixmap pmapMask = QPixmap(maskName);
    if (!pmapMask.isNull()) {
        mask = pmapMask.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if (!unscaled.isNull()) {
        pixmap = unscaled.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        updatePixmap();
        update();
    }
}

void MaskablePixmapWidget::mousePressEvent(QMouseEvent*) {
    if (clickable) {
        emit clicked();
    }
}

void MaskablePixmapWidget::updatePixmap() {
    renderTarget->fill(Qt::transparent);
    renderTarget->setDevicePixelRatio(1.0);

    QSize actualSize = size() * devicePixelRatioF();
    QPoint offset((actualSize.width() - pixmap.size().width()) / 2,
                  (actualSize.height() - pixmap.size().height()) / 2);  // centering the pixmap

    QPainter painter(renderTarget);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawPixmap(offset, pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0, 0, mask);
    painter.end();

    renderTarget->setDevicePixelRatio(this->devicePixelRatioF());
    QLabel::setPixmap(*renderTarget);
}
