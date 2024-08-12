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

#ifndef NOTIFICATIONSCROLLAREA_H
#define NOTIFICATIONSCROLLAREA_H

#include <QHash>
#include "tool/adjustingscrollarea.h"

class GenericChatroomWidget;
class NotificationEdgeWidget;

class NotificationScrollArea final : public AdjustingScrollArea {
public:
    explicit NotificationScrollArea(QWidget* parent = nullptr);

public slots:
    void trackWidget(GenericChatroomWidget* widget);
    void updateVisualTracking();
    void updateTracking(GenericChatroomWidget* widget);

protected:
    void resizeEvent(QResizeEvent* event) final override;

private slots:
    void findNextWidget();
    void findPreviousWidget();

private:
    enum Visibility : uint8_t { Visible, Above, Below };
    Visibility widgetVisible(QWidget* widget) const;
    void recalculateTopEdge();
    void recalculateBottomEdge();

    QHash<GenericChatroomWidget*, Visibility> trackedWidgets;
    NotificationEdgeWidget* topEdge = nullptr;
    NotificationEdgeWidget* bottomEdge = nullptr;
    size_t referencesAbove = 0;
    size_t referencesBelow = 0;
};

#endif  // NOTIFICATIONSCROLLAREA_H
