#ifndef ROUNDEDMASKCONTAINER_H
#define ROUNDEDMASKCONTAINER_H

#include <QFrame>

class RoundMaskGraphicsEffect;

class RoundedMaskContainer : public QFrame {
    Q_OBJECT
    Q_PROPERTY(int radius READ roundRadius WRITE setRoundRadius)
  public:
    RoundedMaskContainer(QWidget *parent);
    Q_INVOKABLE void setRoundRadius(int radius);
    Q_INVOKABLE int roundRadius() const {
        return _radius;
    }

  private:
    int _radius = 0;
    RoundMaskGraphicsEffect *effect = nullptr;
};
#endif // !ROUNDEDMASKCONTAINER_H
