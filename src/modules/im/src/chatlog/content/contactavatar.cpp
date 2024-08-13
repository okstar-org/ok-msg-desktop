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

#include "contactavatar.h"
#include "../pixmapcache.h"

#include <QPainter>

static constexpr float avatar_size = 40;
ContactAvatar::ContactAvatar(const QPixmap& avatar) : avatar(avatar) {}

QRectF ContactAvatar::boundingRect() const { return QRectF(0, 0, avatar_size, avatar_size); }

qreal ContactAvatar::getAscent() const { return 0.0; }

QIcon ContactAvatar::invalidAvatar() {
    static QIcon icon;
    if (icon.isNull()) {
        icon.addFile(":/img/contact_dark.svg");
    }
    return icon;
}

void ContactAvatar::paint(QPainter* painter,
                          const QStyleOptionGraphicsItem* option,
                          QWidget* widget) {
    QRectF r = boundingRect();
    QPainterPath path;
    path.addRoundedRect(r.adjusted(0, 0, -1, -1), 4.0, 4.0);

    if (!avatar.isNull()) {
        QSizeF s = QSizeF(avatar.size()) / painter->device()->devicePixelRatioF();
        s.scale(r.size(), Qt::KeepAspectRatioByExpanding);
        QRectF paint_r(QPointF(0, 0), s);
        paint_r.moveCenter(r.center());
        painter->save();
        painter->setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
        painter->setClipPath(path);
        painter->drawPixmap(paint_r.toRect(), avatar);
        painter->restore();
    } else {
        painter->save();
        painter->setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
        painter->setClipPath(path);
        invalidAvatar().paint(painter, r.toRect(), Qt::AlignCenter);
        painter->restore();
    }

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void ContactAvatar::setWidth(qreal width) { Q_UNUSED(width) }
