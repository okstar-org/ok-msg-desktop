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

#ifndef AUDIOVOLUMNSLIDER_H_
#define AUDIOVOLUMNSLIDER_H_

#include <QSlider>

class AudioVolumnSlider : public QSlider {
    Q_OBJECT
    Q_PROPERTY(bool custom_style READ customStyle WRITE setCustomStyle)
public:
    AudioVolumnSlider(QWidget* parent);
    void setCustomStyle(bool set) {
        _custom_style = set;
        update();
    }
    bool customStyle() {
        return _custom_style;
    }
    void setRealVolume(int val);
    void setVolume(int val) {
        setValue(val);
    }

protected:
    void paintEvent(QPaintEvent* ev);
    void mousePressEvent(QMouseEvent* me);

private:
    int valueFromPosition(int pos_x);
    int positionFromValue(int value);

private:
    bool _custom_style = false;
    int real_volume = 0;
};

#endif  // !AUDIOVOLUMNSLIDER_H_
