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
#include "OPainterColorPanel.h"
#include "ui_OPainterColorPanel.h"

#include "base/logs.h"
#include "base/widgets.h"

namespace module::painter {

OPainterColorPanel::OPainterColorPanel(QWidget* parent)
        : QWidget(parent), ui(new Ui::OPainterColorPanel) {
    ui->setupUi(this);

    installEventFilter(this);
    setHidden(true);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);  //
    setAttribute(Qt::WA_TranslucentBackground, true);    //
    //  setAttribute(Qt::WA_NoMousePropagation, true);

    // 实例阴影shadow
    //    base::Widgets::SetShadowEffect(ui->color_frame, {0, 0},  //
    //                                   QColor("#535353"), 20);
}

OPainterColorPanel::~OPainterColorPanel() {
    delete ui;
}

void OPainterColorPanel::init() {
    ui->weight_1->click();
    ui->color_1->click();
}

void OPainterColorPanel::on_color_1_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_2_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_3_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_4_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_5_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_6_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_7_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_8_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_9_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_10_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_11_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::on_color_12_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkColor(static_cast<QToolButton*>(QObject::sender()));
}

void OPainterColorPanel::checkColor(QToolButton* color) {
    // qDebug() << "color:" << color;

    for (int i = 0; i < ui->layout_color->count(); i++) {
        QToolButton* clr = static_cast<QToolButton*>(ui->layout_color->itemAt(i)->widget());
        if (clr && clr != color) clr->setChecked(false);
    }

    const QColor clr = color->palette().background().color();

    // qDebug() << clr.name();

    emit colorChange(clr);
}

void OPainterColorPanel::on_weight_1_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkWeight(static_cast<QToolButton*>(QObject::sender()));
    emit weightChange(1);
}

void OPainterColorPanel::on_weight_2_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkWeight(static_cast<QToolButton*>(QObject::sender()));
    emit weightChange(2);
}

void OPainterColorPanel::on_weight_3_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkWeight(static_cast<QToolButton*>(QObject::sender()));
    emit weightChange(3);
}

void OPainterColorPanel::on_weight_4_clicked(bool isChecked) {
    Q_UNUSED(isChecked);
    checkWeight(static_cast<QToolButton*>(QObject::sender()));
    emit weightChange(4);
}

void OPainterColorPanel::checkWeight(QToolButton* weight) {
    // qDebug() << "weight:" << weight;

    for (int i = 0; i < ui->layout_weight->count(); i++) {
        QToolButton* clr = static_cast<QToolButton*>(ui->layout_weight->itemAt(i)->widget());
        if (clr && clr != weight) clr->setChecked(false);
    }
}

void OPainterColorPanel::hide(bool force) {
    if (!force && _hovered) {
        return;
    }
    QWidget::hide();
}

bool OPainterColorPanel::eventFilter(QObject* target, QEvent* event) {
    if (event->type() == QEvent::Type::Show) {
        _hovered = true;
    } else if (event->type() == QEvent::Type::Leave) {
        _hovered = false;
        hide(true);
    }

    return QWidget::eventFilter(target, event);
}

}  // namespace module::painter