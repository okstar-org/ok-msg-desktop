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

#include "shadowbackground.h"
#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QtMath>

#include <QDebug>

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QPainter* p, QImage& blurImage, qreal radius, bool quality, bool alphaOnly,
                         int transposed = 0);
QT_END_NAMESPACE

namespace ok::base {

static QImage getRectShadow(const QSize& size, int shadowRadius, const QBrush shadowBrush,
                            const QBrush& backBrush, qreal rectRadius, qreal deviceScale) {
    const int s_r = shadowRadius;
    QSize img_size = QSize(size.width() + s_r * 2, size.height() + s_r * 2) * deviceScale;
    QRectF cr(QPointF(s_r, s_r), size);
    {
        cr.setTopLeft(QPointF((cr.topLeft() * deviceScale).toPoint()) / deviceScale);
        cr.setBottomRight(QPointF((cr.bottomRight() * deviceScale).toPoint()) / deviceScale);
    }

    // 填充原始区域
    QImage source(img_size, QImage::Format_ARGB32_Premultiplied);
    source.setDevicePixelRatio(deviceScale);
    source.fill(0);
    QPainter srcPainter(&source);
    srcPainter.setPen(Qt::NoPen);
    srcPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    srcPainter.setBrush(shadowBrush);
    srcPainter.drawRoundedRect(cr, rectRadius, rectRadius);
    srcPainter.end();
    source.setDevicePixelRatio(1.0);

    // 模糊图像
    QImage target(img_size, QImage::Format_ARGB32_Premultiplied);
    target.fill(0);
    if (s_r > 0) {
        QPainter blur_painter(&target);
        qt_blurImage(&blur_painter, source, s_r * deviceScale, false, true);
    }
    target.setDevicePixelRatio(deviceScale);

    // 上一步操作会使中心区域模糊，重新填充
    QPainter painter(&target);
    painter.setPen(Qt::NoPen);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(target.rect(), shadowBrush);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setBrush(backBrush);
    painter.drawRoundedRect(cr, rectRadius, rectRadius);
    painter.end();
    return target;
}

ShadowBackground::ShadowBackground(QWidget* parent) : QObject(parent) {
    target = parent;
    target->installEventFilter(this);
}

ShadowBackground::~ShadowBackground() {
    qDebug() << __func__;
}

void ShadowBackground::setShadowColor(const QColor& color) {
    if (_shadowColor != color) {
        _shadowColor = color;
        clearCache();
    }
}

void ShadowBackground::setShadowRadius(int radius) {
    if (_shadowRadius != radius) {
        _shadowRadius = std::max(radius, 0);
        clearCache();
    }
}

void ShadowBackground::setRoudedRadius(qreal radius) {
    if (_borderRadius != radius) {
        _borderRadius = std::max(radius, 0.0);
        clearCache();
    }
}

void ShadowBackground::setBackground(const QColor& color) {
    if (_centerColor != color) {
        _centerColor = color;
        clearCache();
    }
}

bool ShadowBackground::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Paint && watched == target.data()) {
        drawShadow();
        return true;
    }
    return false;
}

void ShadowBackground::drawShadow() {
    if (!target) return;
    qreal factor = target->devicePixelRatioF();
    const int content = 10;
    if (shadowPix.isNull()) {
        const int im_w = qFloor(_borderRadius) * 2 + content;
        shadowPix = getRectShadow(QSize(im_w, im_w), _shadowRadius, _shadowColor, _centerColor,
                                  _borderRadius, factor);
    }
    int clip = _shadowRadius + _borderRadius;
    int img_clip = qRound(clip * factor);
    QRectF anchor = QRectF(target->rect()).adjusted(0, 0, -clip, -clip);
    QRectF im_anchor = QRectF(shadowPix.rect()).adjusted(0, 0, -img_clip, -img_clip);

    QPainter painter(target.data());
    // corners
    const QSizeF corner_size(clip, clip);
    const QSizeF im_corner_size(img_clip, img_clip);
    painter.drawImage(QRectF(anchor.topLeft(), corner_size), shadowPix,
                      QRectF(im_anchor.topLeft(), im_corner_size));
    painter.drawImage(QRectF(anchor.topRight(), corner_size), shadowPix,
                      QRectF(im_anchor.topRight(), im_corner_size));
    painter.drawImage(QRectF(anchor.bottomRight(), corner_size), shadowPix,
                      QRectF(im_anchor.bottomRight(), im_corner_size));
    painter.drawImage(QRectF(anchor.bottomLeft(), corner_size), shadowPix,
                      QRectF(im_anchor.bottomLeft(), im_corner_size));

    // edges
    const QSizeF he_size(target->width() - clip - clip, clip);
    const QSizeF ve_size(clip, target->height() - clip - clip);
    anchor.adjust(clip, 0, 0, 0);
    im_anchor.adjust(img_clip, 0, 0, 0);
    painter.drawImage(QRectF(anchor.topLeft(), he_size), shadowPix,
                      QRectF(im_anchor.topLeft(), QSizeF(content, img_clip)));
    painter.drawImage(QRectF(anchor.bottomLeft(), he_size), shadowPix,
                      QRectF(im_anchor.bottomLeft(), QSizeF(content, img_clip)));
    anchor.adjust(-clip, clip, 0, 0);
    im_anchor.adjust(-img_clip, img_clip, 0, 0);
    painter.drawImage(QRectF(anchor.topLeft(), ve_size), shadowPix,
                      QRectF(im_anchor.topLeft(), QSizeF(img_clip, content)));
    painter.drawImage(QRectF(anchor.topRight(), ve_size), shadowPix,
                      QRectF(im_anchor.topRight(), QSizeF(img_clip, content)));

    // center
    anchor.setTopLeft(QPointF(clip, clip));
    im_anchor.setTopLeft(QPointF(img_clip, img_clip));
    painter.drawImage(anchor, shadowPix, im_anchor);
}

void ShadowBackground::clearCache() {
    shadowPix = QImage();
    if (target) target->update();
}
}  // namespace ok::base