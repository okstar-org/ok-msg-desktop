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
#include "lib/ui/widget/tools/RoundedPixmapLabel.h"
#include "base/widgets.h"
#include "src/lib/storage/settings/style.h"
#include "src/model/friendlist.h"
#include "src/widget/widget.h"

namespace module::im {

CallConfirmWidget::CallConfirmWidget(const PeerId& from, bool video, QWidget* parent)
        : QWidget(parent)
        , from(from)
        , isVideo(video)
        , rectW{320}
        , rectH{182}
        , spikeW{30}
        , spikeH{15}
        , roundedFactor{20}
        , rectRatio(static_cast<qreal>(rectH) / static_cast<qreal>(rectW)) {
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);

    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    // 头像
    auto frd = Core::getInstance()->getFriendList().findFriend(from);
    auto avt = new lib::ui::RoundedPixmapLabel(this);
    // avt->setText(tr("Avatar"));
    avt->setPixmap(frd->getAvatar());
    avt->setContentsSize(QSize(120, 120));
    avt->setPixmapAlign(Qt::AlignCenter);
    // avt->setAlignment(Qt::AlignCenter);
    avt->setStyleSheet("QWidget{border:1px solid red;}");
    layout->addWidget(avt);

    callLabel = new QLabel(QObject::tr("Incoming call..."), this);
    callLabel->setAlignment(Qt::AlignCenter);
    // callLabel->setToolTip(callLabel->text());

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

    buttonBox = new QDialogButtonBox(Qt::Horizontal, this);

    auto* reject = new QPushButton(this);
    reject->setFlat(true);
    reject->setStyleSheet("QPushButton{border:none;}");
    reject->setIcon(QIcon(lib::settings::Style::getImagePath("rejectCall/rejectCall.svg")));
    reject->setIconSize(reject->size());

    buttonBox->addButton(reject, QDialogButtonBox::RejectRole);
    for (auto* b : buttonBox->buttons()) b->setCursor(Qt::PointingHandCursor);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CallConfirmWidget::accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CallConfirmWidget::rejected);

    layout->setMargin(marginSize);
    layout->addSpacing(spikeH);
    layout->addWidget(callLabel);

    auto ctrlLayout = new QHBoxLayout(this);
    ctrlLayout->addStretch(1);
    ctrlLayout->addWidget(buttonBox);
    ctrlLayout->addStretch(1);

    layout->addLayout(ctrlLayout);

    ok::base::Widgets::moveToScreenCenter(this);
}

void CallConfirmWidget::setCallLabel(const QString& label) {
    callLabel->setText(label);
}

void CallConfirmWidget::reposition() {
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
    auto call = CoreAV::getInstance()->getCall(from.toFriendId().toString());
    if (!call) {
        return;
    }

    if (call->getDirection() == lib::messenger::CallDirection::CallOut) {
        // 呼出，不显示接受
    } else {
        // 呼入，显示接受按钮
        auto accept = new QPushButton(this);
        accept->setFlat(true);
        accept->setStyleSheet("QPushButton{border:none;}");
        accept->setIcon(QIcon(lib::settings::Style::getImagePath("acceptCall/acceptCall.svg")));
        accept->setIconSize(accept->size());
        buttonBox->addButton(accept, QDialogButtonBox::AcceptRole);
    }
}

void CallConfirmWidget::hideEvent(QHideEvent*) {
    if (parentWidget()) parentWidget()->removeEventFilter(this);
}

bool CallConfirmWidget::eventFilter(QObject*, QEvent* event) {
    if (event->type() == QEvent::Resize) reposition();

    return false;
}
}  // namespace module::im
