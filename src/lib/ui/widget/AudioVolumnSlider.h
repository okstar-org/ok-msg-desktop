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