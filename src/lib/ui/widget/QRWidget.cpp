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

#include "QRWidget.h"
#include <qrencode.h>
#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QVBoxLayout>
namespace lib::ui {
/**
 * @file qrwidget.cpp
 * @link https://stackoverflow.com/questions/21400254/how-to-draw-a-qr-code-with-qt-in-native-c-c
 */

QRWidget::QRWidget(QSize& size_, QWidget* parent)
        : QWidget(parent)
        , data("0")
        , size{size_}

{
    image = new QImage(size, QImage::Format_RGB32);
    image->fill(QColor(0, 0, 0, 0));
    setFixedSize(size);
}

QRWidget::~QRWidget() { delete image; }

QSize QRWidget::sizeHint() const { return minimumSizeHint(); }

QSize QRWidget::minimumSizeHint() const { return size.grownBy(this->contentsMargins()); }

void QRWidget::setQRData(const QString& data) {
    this->data = data;
    paintImage();
    update();
}

const QImage* QRWidget::getImage() { return image; }

/**
 * @brief QRWidget::saveImage
 * @param path Full path to the file with extension.
 * @return indicate if saving was successful.
 */
bool QRWidget::saveImage(QString path) {
    // 0 - image format same as file extension, 75-quality, png file is ~6.3kb
    return image->save(path, nullptr, 75);
}

void QRWidget::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    QRect rect = this->contentsRect();
    painter.drawImage(rect, *image);
}

// http://stackoverflow.com/questions/21400254/how-to-draw-a-qr-code-with-qt-in-native-c-c
void QRWidget::paintImage() {
    QPainter painter(image);
    // NOTE: I have hardcoded some parameters here that would make more sense as variables.
    // ECLEVEL_M is much faster recognizable by barcodescanner any any other type
    // https://fukuchi.org/works/qrencode/manual/qrencode_8h.html#a4cebc3c670efe1b8866b14c42737fc8f
    // any mode other than QR_MODE_8 or QR_MODE_KANJI results in EINVAL. First 1 is version, second
    // is case sensitivity
    const std::string dataString = data.toStdString();
    QRcode* qr = QRcode_encodeString(dataString.c_str(), 1, QR_ECLEVEL_M, QR_MODE_8, 1);

    if (qr != nullptr) {
        QColor fg("black");
        QColor bg("white");
        painter.setBrush(bg);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, size.width(), size.height());
        painter.setBrush(fg);
        painter.scale(0.96, 0.96);
        painter.translate(size.width() * 0.02, size.height() * 0.02);
        const int s = qr->width > 0 ? qr->width : 1;
        const double w = width();
        const double h = height();
        const double aspect = w / h;
        const double scale = ((aspect > 1.0) ? h : w) / s;

        for (int y = 0; y < s; ++y) {
            const int yy = y * s;
            for (int x = 0; x < s; ++x) {
                const int xx = yy + x;
                const unsigned char b = qr->data[xx];
                if (b & 0x01) {
                    const double rx1 = x * scale, ry1 = y * scale;
                    QRectF r(rx1, ry1, scale, scale);
                    painter.drawRects(&r, 1);
                }
            }
        }
        QRcode_free(qr);
    } else {
        QColor error("red");
        painter.setBrush(error);
        painter.drawRect(0, 0, width(), height());
        qDebug() << "QR FAIL: " << strerror(errno);
    }

    qr = nullptr;
}
}  // namespace lib::ui
