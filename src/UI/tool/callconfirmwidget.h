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

#ifndef CALLCONFIRMWIDGET_H
#define CALLCONFIRMWIDGET_H

#include <QBrush>
#include <QPolygon>
#include <QRect>
#include <QWidget>

class QPaintEvent;
class QShowEvent;

class CallConfirmWidget final : public QWidget {
    Q_OBJECT
public:
    explicit CallConfirmWidget(const QWidget* anchor);

signals:
    void accepted();
    void rejected();

public slots:
    void reposition();

protected:
    void paintEvent(QPaintEvent* event) final;
    void showEvent(QShowEvent* event) final;
    void hideEvent(QHideEvent* event) final;
    bool eventFilter(QObject*, QEvent* event) final;

private:
    const QWidget* anchor;

    QRect mainRect;
    QPolygon spikePoly;
    QBrush brush;

    const int rectW, rectH;
    const int spikeW, spikeH;
    const int roundedFactor;
    const qreal rectRatio;
};

#endif  // CALLCONFIRMWIDGET_H
