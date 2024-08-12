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

#include "notificationedgewidget.h"
#include <QBoxLayout>
#include <QLabel>
#include "src/lib/settings/style.h"

#include <QDebug>

NotificationEdgeWidget::NotificationEdgeWidget(Position position, QWidget* parent)
        : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);  // Show background.
    setStyleSheet(Style::getStylesheet("notificationEdge/notificationEdge.css"));
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addStretch();

    textLabel = new QLabel(this);
    textLabel->setMinimumHeight(textLabel->sizeHint().height());  // Prevent cut-off text.
    layout->addWidget(textLabel);

    QLabel* arrowLabel = new QLabel(this);

    if (position == Top)
        arrowLabel->setPixmap(QPixmap(Style::getImagePath("chatArea/scrollBarUpArrow.svg")));
    else
        arrowLabel->setPixmap(QPixmap(Style::getImagePath("chatArea/scrollBarDownArrow.svg")));

    layout->addWidget(arrowLabel);
    layout->addStretch();

    setCursor(Qt::PointingHandCursor);
}

void NotificationEdgeWidget::updateNotificationCount(int count) {
    textLabel->setText(tr("Unread message(s)", "", count));
}

void NotificationEdgeWidget::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();
    QWidget::mousePressEvent(event);
}
