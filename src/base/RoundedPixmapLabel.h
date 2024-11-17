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
        MinEdgeCircle,
        MaxEdgeCircle,
        PercentRadius,
        AbsoluteRadius
    };

public:
    RoundedPixmapLabel(QWidget* parent = nullptr);    
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void setContentsSize(const QSize & size);
    void setPixmap(const QPixmap & pixmap);
    void setMaskOnPixmap(bool pixmap);
    void setPixmapAlign(Qt::Alignment alignment);
    void setScaleMode(PixmapScaleMode mode);
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
