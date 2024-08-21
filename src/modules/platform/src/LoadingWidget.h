#pragma once

#include <QPointer>
#include <QWidget>

class QVariantAnimation;
class QTimeLine;

class LoadingWidget : public QWidget {
public:
    LoadingWidget(QWidget* target);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setMarginInTarget(const QMargins& margins);
    void setMarginInTarget(int left, int top, int right, int bottom) {
        setMarginInTarget(QMargins(left, top, right, bottom));
    }

    void setSizeHint(const QSize & hint);

protected:
    bool event(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* e) override;

private:
    void updateWidgetGeo();

private:
    QPointer<QWidget> anchorWidget;
    QMargins layoutMargins = {0, 0, 0, 0};
    QVariantAnimation* progressAnima = nullptr;
    QTimeLine * timeLine = nullptr;
    qreal progress = 0.0;
    QSize contentSizeHint = {60, 60};
};
