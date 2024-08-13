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

#include "callconfirmwidget.h"
#include <assert.h>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QRect>
#include <QVBoxLayout>
#include "src/lib/settings/style.h"
#include "src/widget/widget.h"

/**
 * @class CallConfirmWidget
 * @brief This is a widget with dialog buttons to accept/reject a call
 *
 * It tracks the position of another widget called the anchor
 * and looks like a bubble at the bottom of that widget.
 *
 * @var const QWidget* CallConfirmWidget::anchor
 * @brief The widget we're going to be tracking
 *
 * @var const int CallConfirmWidget::roundedFactor
 * @brief By how much are the corners of the main rect rounded
 *
 * @var const qreal CallConfirmWidget::rectRatio
 * @brief Used to correct the rounding factors on non-square rects
 */

CallConfirmWidget::CallConfirmWidget(const QWidget* anchor)
        : QWidget()
        , anchor(anchor)
        , rectW{120}
        , rectH{85}
        , spikeW{30}
        , spikeH{15}
        , roundedFactor{20}
        , rectRatio(static_cast<qreal>(rectH) / static_cast<qreal>(rectW)) {
    setWindowFlags(Qt::SubWindow);
    setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::white);
    setPalette(palette);

    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* callLabel = new QLabel(QObject::tr("Incoming call..."), this);
    callLabel->setStyleSheet("QLabel{color: white;} QToolTip{color: black;}");
    callLabel->setAlignment(Qt::AlignHCenter);
    callLabel->setToolTip(callLabel->text());

    // Note: At the moment this may not work properly. For languages written
    // from right to left, there is no translation for the phrase "Incoming call...".
    // In this situation, the phrase "Incoming call..." looks as "...oming call..."
    Qt::TextElideMode elideMode = (QGuiApplication::layoutDirection() == Qt::LeftToRight)
                                          ? Qt::ElideRight
                                          : Qt::ElideLeft;
    int marginSize = 12;
    QFontMetrics fontMetrics(callLabel->font());
    QString elidedText =
            fontMetrics.elidedText(callLabel->text(), elideMode, rectW - marginSize * 2 - 4);
    callLabel->setText(elidedText);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *accept = new QPushButton(this), *reject = new QPushButton(this);
    accept->setFlat(true);
    reject->setFlat(true);
    accept->setStyleSheet("QPushButton{border:none;}");
    reject->setStyleSheet("QPushButton{border:none;}");
    accept->setIcon(QIcon(Style::getImagePath("acceptCall/acceptCall.svg")));
    reject->setIcon(QIcon(Style::getImagePath("rejectCall/rejectCall.svg")));
    accept->setIconSize(accept->size());
    reject->setIconSize(reject->size());

    buttonBox->addButton(accept, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(reject, QDialogButtonBox::RejectRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CallConfirmWidget::accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CallConfirmWidget::rejected);

    layout->setMargin(marginSize);
    layout->addSpacing(spikeH);
    layout->addWidget(callLabel);
    layout->addWidget(buttonBox);

    setFixedSize(rectW, rectH + spikeH);
    reposition();
}

/**
 * @brief Recalculate our positions to track the anchor
 */
void CallConfirmWidget::reposition() {
    if (parentWidget()) parentWidget()->removeEventFilter(this);

    setParent(anchor->window());
    parentWidget()->installEventFilter(this);

    QWidget* w = anchor->window();
    QPoint pos = anchor->mapToGlobal({(anchor->width() - rectW) / 2, anchor->height()}) -
                 w->mapToGlobal({0, 0});

    // We don't want the widget to overflow past the right of the screen
    int xOverflow = 0;
    if (pos.x() + rectW > w->width()) xOverflow = pos.x() + rectW - w->width();
    pos.rx() -= xOverflow;

    mainRect = {0, spikeH, rectW, rectH};
    brush = QBrush(QColor(65, 65, 65));
    spikePoly = QPolygon({{(rectW - spikeW) / 2 + xOverflow, spikeH},
                          {rectW / 2 + xOverflow, 0},
                          {(rectW + spikeW) / 2 + xOverflow, spikeH}});

    move(pos);
    update();
}

void CallConfirmWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);

    painter.drawRoundedRect(mainRect, roundedFactor * rectRatio, roundedFactor, Qt::RelativeSize);
    painter.drawPolygon(spikePoly);
}

void CallConfirmWidget::showEvent(QShowEvent*) {
    // Kriby: Legacy comment, is this still true?
    // If someone does show() from Widget or lower, the event will reach us
    // because it's our parent, and we could show up in the wrong form.
    // So check here if our friend's form is actually the active one.

    reposition();
    update();
}

void CallConfirmWidget::hideEvent(QHideEvent*) {
    if (parentWidget()) parentWidget()->removeEventFilter(this);

    setParent(nullptr);
}

bool CallConfirmWidget::eventFilter(QObject*, QEvent* event) {
    if (event->type() == QEvent::Resize) reposition();

    return false;
}
