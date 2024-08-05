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

#ifndef FLYOUTOVERLAYWIDGET_HPP
#define FLYOUTOVERLAYWIDGET_HPP

#include <QWidget>

class QPropertyAnimation;

class FlyoutOverlayWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal flyoutPercent READ flyoutPercent WRITE setFlyoutPercent)
public:
    explicit FlyoutOverlayWidget(QWidget* parent = nullptr);
    ~FlyoutOverlayWidget();

    int animationDuration() const;
    void setAnimationDuration(int timeMs);

    qreal flyoutPercent() const;
    void setFlyoutPercent(qreal progress);

    bool isShown() const;
    bool isBeingAnimated() const;
    bool isBeingShown() const;

    void animateShow();
    void animateHide();

signals:

    void hidden();

private:
    void finishedAnimation();
    void startAnimation(bool forward);

    QWidget* container;
    QPropertyAnimation* animation;
    qreal percent = 1.0f;
    QPoint startPos;
};

#endif  // FLYOUTOVERLAYWIDGET_HPP
