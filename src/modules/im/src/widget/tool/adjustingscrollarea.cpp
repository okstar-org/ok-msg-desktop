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

#include "adjustingscrollarea.h"

#include <QDebug>
#include <QEvent>
#include <QLayout>
#include <QScrollBar>

AdjustingScrollArea::AdjustingScrollArea(QWidget* parent) : QScrollArea(parent) {}

void AdjustingScrollArea::resizeEvent(QResizeEvent* ev) {
    int scrollBarWidth =
            verticalScrollBar()->isVisible() ? verticalScrollBar()->sizeHint().width() : 0;

    if (layoutDirection() == Qt::RightToLeft) setViewportMargins(-scrollBarWidth, 0, 0, 0);

    updateGeometry();
    QScrollArea::resizeEvent(ev);
}

QSize AdjustingScrollArea::sizeHint() const {
    if (widget()) {
        int scrollbarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
        return widget()->sizeHint() + QSize(scrollbarWidth, 0);
    }

    return QScrollArea::sizeHint();
}
