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

//
// Created by gaojie on 23-9-17.
//
#pragma once

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
namespace ok::base {
class Images {
public:
    inline static bool putToImage(const QByteArray& data, QImage& image) {
        return image.loadFromData(data);
    }
    inline static bool putToPixmap(const QByteArray& data, QPixmap& pixmap) {
        return pixmap.loadFromData(data);
    }

    static QPixmap roundRectPixmap(const QPixmap& srcPixMap, const QSize& size, int radius) {
        // 不处理空数据或者错误数据
        if (srcPixMap.isNull()) {
            return srcPixMap;
        }

        // 获取图片尺寸
        int imageWidth = size.width();
        int imageHeight = size.height();

        // 处理大尺寸的图片,保证图片显示区域完整
        QPixmap newPixMap =
                srcPixMap.scaled(imageWidth, (imageHeight == 0 ? imageWidth : imageHeight),
                                 Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QPixmap destImage(imageWidth, imageHeight);
        destImage.fill(Qt::transparent);
        QPainter painter(&destImage);
        // 抗锯齿
        painter.setRenderHints(QPainter::Antialiasing, true);
        // 图片平滑处理
        painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
        // 将图片裁剪为圆角
        QPainterPath path;
        QRect rect(0, 0, imageWidth, imageHeight);
        path.addRoundedRect(rect, radius, radius);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, imageWidth, imageHeight, newPixMap);
        return destImage;
    }
};
}
