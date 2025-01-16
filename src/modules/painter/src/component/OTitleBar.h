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
#ifndef OTITLEBAR_H
#define OTITLEBAR_H

#include <QWidget>

namespace Ui {
class OTitleBar;
}

class OTitleBar : public QWidget {
    Q_OBJECT

public:
    explicit OTitleBar(QWidget* parent = nullptr);
    ~OTitleBar();
    void setTitle(const QString& title);

    QWidget* getTitle();

private:
    Ui::OTitleBar* ui;
};

#endif  // OTITLEBAR_H
