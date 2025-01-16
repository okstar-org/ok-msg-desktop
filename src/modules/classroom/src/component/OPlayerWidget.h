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
#ifndef OPLAYER_WIDGET_H
#define OPLAYER_WIDGET_H

#include <QString>
#include <QWidget>
#include "OVideoPlayer.h"

namespace Ui {
class OPlayerWidget;
}

class OPlayerWidget : public QWidget {
    Q_OBJECT

public:
    explicit OPlayerWidget(QWidget* parent = nullptr);
    ~OPlayerWidget();

    void setSource(const QUrl& url);
    void play();

protected:
    virtual void paintEvent(QPaintEvent* event) override;

protected slots:
    void on_btnMin_clicked();
    void on_btnMax_clicked();
    void on_btnClose_clicked();
    void on_bthFull_clicked();

private:
    Ui::OPlayerWidget* ui;

    OVideoPlayer* videoPlayer_;
};

#endif  // MAINWINDOW_H
