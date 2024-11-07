#ifndef SHADOWBACKGROUND_H
#define SHADOWBACKGROUND_H

#include <QPointer>
#include <QWidget>

class ShadowBackground : public QObject {
  public:
    ShadowBackground(QWidget *parent = nullptr);

    // set shadow color, radius
    void setShadowColor(const QColor & color);
    void setShadowRadius(int radius);

    // set border width
    void setRoudedRadius(qreal radius);

    // set center background
    void setBackground(const QColor & color);

  protected:
    void drawShadow();
    void clearCache();

    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    QPointer<QWidget> target;
    int _shadowRadius = 10;
    QColor _shadowColor = 0x858585;
    QColor _centerColor = Qt::white;
    qreal _borderRadius = 6.0;
    QImage shadowPix;
};

#endif // !SHADOWBACKGROUND_H
