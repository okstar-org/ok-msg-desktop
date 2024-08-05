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

#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QDialog>
#include <QWidget>

class BaseWindow : public QWidget {
    Q_OBJECT

public:
    BaseWindow(QWidget* parent = 0);
    ~BaseWindow();

protected:
    void paintEvent(QPaintEvent* event);

private slots:

protected:
};

#endif  // BASEWINDOW_H
