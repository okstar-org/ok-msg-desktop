#pragma once

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

#include <QPainterPath>
#include <QWidget>
class RoundedPixmapLabel : public QWidget {
    Q_OBJECT

public:
    enum PixmapScaleMode{
        NoScale,
        IgnoreAspectRatio = 0x01,
        KeepAspectRatio,
        KeepAspectRatioByExpanding
    };

    enum RoundedType
    {
        NoRound,
        MinEdgeCircle,  // 以短边圆形遮罩
        MaxEdgeCircle,  // 以长边圆形遮罩
        PercentRadius,  // 百分比圆角 0-100
        AbsoluteRadius  // 像素绝对值
    };

public:
    RoundedPixmapLabel(QWidget* parent = nullptr);    
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    // 头像图片显示区域尺寸
    void setContentsSize(const QSize & size);
    void setPixmap(const QPixmap & pixmap);

    // 在图片上进行圆角遮罩还是QWidget
    void setMaskOnPixmap(bool pixmap);
    // 对齐方式
    void setPixmapAlign(Qt::Alignment alignment);
    // 缩放模式
    void setScaleMode(PixmapScaleMode mode);
    // 圆角类型
    void setRoundedType(RoundedType type);
    void setRoundRadius(int xRadius, int yRadius);

protected:
    void paintEvent(QPaintEvent *event);
    QRect paintRect();
    QPainterPath roundMaskPath(const QRect & rect);

private:
    QPixmap _pixmap;
    QPixmap _cachePixmap;
    QSize _contentsSize;
    Qt::Alignment _align = Qt::AlignCenter;
    PixmapScaleMode _scaleMode = KeepAspectRatio;
    RoundedType _roundType = MinEdgeCircle;
    bool maskPixmap = true;
    QPoint roundRadius= {0, 0};
};
