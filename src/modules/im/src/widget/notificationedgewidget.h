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

#ifndef NOTIFICATIONEDGEWIDGET_H
#define NOTIFICATIONEDGEWIDGET_H

#include <QWidget>

class QLabel;

class NotificationEdgeWidget final : public QWidget {
    Q_OBJECT
public:
    enum Position : uint8_t { Top, Bottom };

    explicit NotificationEdgeWidget(Position position, QWidget* parent = nullptr);
    void updateNotificationCount(int count);

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent* event) final override;

private:
    QLabel* textLabel;
};

#endif  // NOTIFICATIONEDGEWIDGET_H
