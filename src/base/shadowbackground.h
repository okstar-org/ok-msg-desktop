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

#ifndef SHADOWBACKGROUND_H
#define SHADOWBACKGROUND_H

#include <QPointer>
#include <QWidget>

namespace ok::base {

class ShadowBackground : public QObject {
public:
    explicit ShadowBackground(QWidget* parent = nullptr);
    ~ShadowBackground() override;
    // set shadow color, radius
    void setShadowColor(const QColor& color);
    void setShadowRadius(int radius);

    // set border width
    void setRoudedRadius(qreal radius);

    // set center background
    void setBackground(const QColor& color);

protected:
    void drawShadow();
    void clearCache();

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QPointer<QWidget> target;
    int _shadowRadius = 10;
    QColor _shadowColor = 0x858585;
    QColor _centerColor = Qt::white;
    qreal _borderRadius = 6.0;
    QImage shadowPix;
};

}  // namespace ok::base
#endif  // !SHADOWBACKGROUND_H
