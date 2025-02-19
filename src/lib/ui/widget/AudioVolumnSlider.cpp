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

#include "AudioVolumnSlider.h"

#include <QStyle>
#include <QStyleOptionSlider>
#include <QPainter>
#include <QMouseEvent>

AudioVolumnSlider::AudioVolumnSlider(QWidget* parent) : QSlider(Qt::Horizontal, parent) {
    setRange(0, 100);
    real_volume = 0;
}

void AudioVolumnSlider::setRealVolume(int val) {
    val = qBound(this->minimum(), val, this->maximum());
    if (real_volume != val) {
        real_volume = val;
        update();
    }
}

void AudioVolumnSlider::paintEvent(QPaintEvent* ev) {
    if (_custom_style) {
        QPainter p(this);
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        opt.tickPosition = QSlider::NoTicks;
        opt.activeSubControls = QStyle::SC_SliderGroove;
        opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
        QRect hr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

        // draw groove
        {
            opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;

            const int prime_pos = hr.center().x();
            int start = opt.rect.left();
            QStyle::State state = opt.state;
            if (real_volume > this->minimum()) {
                // 绘制动态值
                const int second_pos = positionFromValue(real_volume);
                if (start < second_pos)
                {
                    QRect clip = opt.rect;
                    clip.setRight(second_pos);
                    opt.state = state | QStyle::State_On;
                    p.setClipRect(clip);
                    style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
                    start = second_pos + 1;
                }
            }

            if (start < prime_pos) {
                // 绘制静态值
                QRect clip = opt.rect;
                clip.setLeft(start);
                clip.setRight(prime_pos);
                opt.state = state;
                p.setClipRect(clip);
                style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
                start = prime_pos + 1;
            }

            if (start < opt.rect.right()) {
                // 绘制剩余
                QRect clip = opt.rect;
                clip.setLeft(start);
                opt.state = state | QStyle::State_Off;
                p.setClipRect(clip);
                style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
            }
            opt.state = state | QStyle::State_Enabled;
        }
    } else {
        QSlider::paintEvent(ev);
    }
}

void AudioVolumnSlider::mousePressEvent(QMouseEvent* me) {
    if (me->buttons() == Qt::LeftButton) {
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        QStyle::SubControl pressed =
                style()->hitTestComplexControl(QStyle::CC_Slider, &opt, me->pos(), this);
        if (pressed == QStyle::SC_SliderGroove) {
            this->blockSignals(true);
            int value = valueFromPosition(me->pos().x());
            setSliderPosition(value);
            this->blockSignals(false);
        }
    }
    QSlider::mousePressEvent(me);
}

int AudioVolumnSlider::valueFromPosition(int pos_x) {
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    const QPoint center = sr.center() - sr.topLeft();
    int sliderMin, sliderMax, sliderLength;
    sliderLength = sr.width();
    sliderMin = gr.x();
    sliderMax = gr.right() - sliderLength + 1;
    return QStyle::sliderValueFromPosition(minimum(), maximum(), pos_x - center.x() - sliderMin,
                                           sliderMax - sliderMin, opt.upsideDown);
}

int AudioVolumnSlider::positionFromValue(int value) {
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    int sliderMin, sliderMax, sliderLength;
    sliderLength = sr.width();
    sliderMin = gr.x();
    sliderMax = gr.right() - sliderLength + 1;
    int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), value, sliderMax - sliderMin,
                                              opt.upsideDown);
    const QPoint center = sr.center() - sr.topLeft();
    return pos + sliderMin + center.x();
}
