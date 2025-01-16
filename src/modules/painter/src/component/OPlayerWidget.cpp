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
#include "OPlayerWidget.h"
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QtMath>
#include "OTitleBar.h"
#include "base/logs.h"
#include "ui_OPlayerWidget.h"

OPlayerWidget::OPlayerWidget(QWidget* parent) : QWidget(parent), ui(new Ui::OPlayerWidget) {
    ui->setupUi(this);

    //    setResizeableAreaWidth(8);

    //    OTitleBar* bar = ui->titleBar;
    //    setTitleBar(bar);
    //    addIgnoreWidget(bar->getTitle());

    // 初始化播放器
    videoPlayer_ = ui->videoPlayer;
    videoPlayer_->start();
}

OPlayerWidget::~OPlayerWidget() {
    videoPlayer_->stop();
    delete ui;
}

void OPlayerWidget::setSource(const QUrl& url) {
    videoPlayer_->setSource(url);
}

void OPlayerWidget::play() {
    videoPlayer_->play();
}

void OPlayerWidget::paintEvent(QPaintEvent* event) {
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRect(0, 0, this->width(), this->height());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(path, QBrush(Qt::white));

    QColor color(0, 0, 0, 50);
    for (int i = 0; i < 10; i++) {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(10 - i, 10 - i, this->width() - (10 - i) * 2, this->height() - (10 - i) * 2);
        color.setAlpha(150 - qSqrt(i) * 50);
        painter.setPen(color);
        painter.drawPath(path);
    }
}

void OPlayerWidget::on_btnMin_clicked() {
    showMinimized();
}
void OPlayerWidget::on_btnMax_clicked() {
    if (isMaximized())
        showNormal();
    else
        showMaximized();
}
void OPlayerWidget::on_btnClose_clicked() {
    close();
}

void OPlayerWidget::on_bthFull_clicked() {
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}
