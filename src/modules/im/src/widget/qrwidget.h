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

#ifndef QRWIDGET_H
#define QRWIDGET_H

#include <QWidget>

class QRWidget : public QWidget {
    Q_OBJECT

public:
    explicit QRWidget(QWidget* parent = nullptr);
    ~QRWidget();
    void setQRData(const QString& data);
    QImage* getImage();
    bool saveImage(QString path);

private:
    QString data;
    void paintImage();
    QImage* image;
    QSize size;
};

#endif  // QRWIDGET_H
