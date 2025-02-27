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
#include "notificationscrollarea.h"
#include <QScrollBar>
#include <cassert>
#include "genericchatroomwidget.h"
#include "notificationedgewidget.h"
namespace module::im {

NotificationScrollArea::NotificationScrollArea(QWidget* parent) : AdjustingScrollArea(parent) {
    connect(verticalScrollBar(), &QAbstractSlider::valueChanged, this,
            &NotificationScrollArea::updateVisualTracking);
    connect(verticalScrollBar(), &QAbstractSlider::rangeChanged, this,
            &NotificationScrollArea::updateVisualTracking);
}

void NotificationScrollArea::trackWidget(GenericChatroomWidget* widget) {
    if (trackedWidgets.find(widget) != trackedWidgets.end()) return;

    Visibility visibility = widgetVisible(widget);
    if (visibility != Visible) {
        if (visibility == Above) {
            if (referencesAbove++ == 0) {
                assert(topEdge == nullptr);
                topEdge = new NotificationEdgeWidget(NotificationEdgeWidget::Top, this);
                connect(topEdge, &NotificationEdgeWidget::clicked, this,
                        &NotificationScrollArea::findPreviousWidget);
                recalculateTopEdge();
                topEdge->show();
            }
            topEdge->updateNotificationCount(referencesAbove);
        } else {
            if (referencesBelow++ == 0) {
                assert(bottomEdge == nullptr);
                bottomEdge = new NotificationEdgeWidget(NotificationEdgeWidget::Bottom, this);
                connect(bottomEdge, &NotificationEdgeWidget::clicked, this,
                        &NotificationScrollArea::findNextWidget);
                recalculateBottomEdge();
                bottomEdge->show();
            }
            bottomEdge->updateNotificationCount(referencesBelow);
        }

        trackedWidgets.insert(widget, visibility);
    }
}

/**
 * @brief Delete notification bar from visible elements on scroll area
 */
void NotificationScrollArea::updateVisualTracking() {
    updateTracking(nullptr);
}

/**
 * @brief Delete notification bar from visible elements and widget on scroll area
 * @param widget Chatroom widget to remove from tracked widgets
 */
void NotificationScrollArea::updateTracking(GenericChatroomWidget* widget) {
    QHash<GenericChatroomWidget*, Visibility>::iterator i = trackedWidgets.begin();
    while (i != trackedWidgets.end()) {
        if (i.key() == widget || widgetVisible(i.key()) == Visible) {
            if (i.value() == Above) {
                if (--referencesAbove == 0) {
                    topEdge->deleteLater();
                    topEdge = nullptr;
                } else {
                    topEdge->updateNotificationCount(referencesAbove);
                }
            } else {
                if (--referencesBelow == 0) {
                    bottomEdge->deleteLater();
                    bottomEdge = nullptr;
                } else {
                    bottomEdge->updateNotificationCount(referencesBelow);
                }
            }
            i = trackedWidgets.erase(i);
            continue;
        }
        ++i;
    }
}

void NotificationScrollArea::resizeEvent(QResizeEvent* event) {
    if (topEdge != nullptr) recalculateTopEdge();
    if (bottomEdge != nullptr) recalculateBottomEdge();

    AdjustingScrollArea::resizeEvent(event);
}

void NotificationScrollArea::findNextWidget() {
    int value = 0;
    GenericChatroomWidget* next = nullptr;
    QHash<GenericChatroomWidget*, Visibility>::iterator i = trackedWidgets.begin();

    // Find the first next, to avoid nullptr.
    for (; i != trackedWidgets.end(); ++i) {
        if (i.value() == Below) {
            next = i.key();
            value = next->mapTo(viewport(), QPoint()).y();
            break;
        }
    }

    // Try finding a closer one.
    for (; i != trackedWidgets.end(); ++i) {
        if (i.value() == Below) {
            int y = i.key()->mapTo(viewport(), QPoint()).y();
            if (y < value) {
                next = i.key();
                value = y;
            }
        }
    }

    if (next != nullptr)
        ensureWidgetVisible(next, 0, referencesBelow != 1 ? bottomEdge->height() : 0);
}

void NotificationScrollArea::findPreviousWidget() {
    int value = 0;
    GenericChatroomWidget* next = nullptr;
    QHash<GenericChatroomWidget*, Visibility>::iterator i = trackedWidgets.begin();

    // Find the first next, to avoid nullptr.
    for (; i != trackedWidgets.end(); ++i) {
        if (i.value() == Above) {
            next = i.key();
            value = next->mapTo(viewport(), QPoint()).y();
            break;
        }
    }

    // Try finding a closer one.
    for (; i != trackedWidgets.end(); ++i) {
        if (i.value() == Above) {
            int y = i.key()->mapTo(viewport(), QPoint()).y();
            if (y > value) {
                next = i.key();
                value = y;
            }
        }
    }

    if (next != nullptr) ensureWidgetVisible(next, 0, referencesAbove != 1 ? topEdge->height() : 0);
}

NotificationScrollArea::Visibility NotificationScrollArea::widgetVisible(QWidget* widget) const {
    int y = widget->mapTo(viewport(), QPoint()).y();

    if (y < 0)
        return Above;
    else if (y + widget->height() - 1 > viewport()->height())
        return Below;

    return Visible;
}

void NotificationScrollArea::recalculateTopEdge() {
    topEdge->move(viewport()->pos());
    topEdge->resize(viewport()->width(), topEdge->height());
}

void NotificationScrollArea::recalculateBottomEdge() {
    QPoint position = viewport()->pos();
    position.setY(position.y() + viewport()->height() - bottomEdge->height());
    bottomEdge->move(position);
    bottomEdge->resize(viewport()->width(), bottomEdge->height());
}
}  // namespace module::im