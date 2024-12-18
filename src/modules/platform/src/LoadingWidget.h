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

#pragma once

#include <QPointer>
#include <QWidget>

class QVariantAnimation;
class QTimeLine;

/**
 * 加载中界面
 */
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
