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
#include "OPainterToolBox.h"
#include "ui_OPainterToolBox.h"

#include "base/logs.h"
#include "base/widgets.h"


static QPoint GetCurPos() {
    auto curPos = QCursor().pos();
    curPos.setX(curPos.x() - 220);
    return curPos;
}

namespace module::classroom {

OPainterToolBox::OPainterToolBox(QWidget* parent)
        : lib::ui::OWidget(parent)
        , ui(new Ui::OPainterToolBox)
        , _delayCaller(std::make_unique<base::DelayedCallTimer>()) {
    ui->setupUi(this);

    //
    //  connect(ui->color_panel_text, &OPainterColorPanel::colorChange, this,
    //          &OPainterToolBox::onTextColorChange);
    //
    //  connect(ui->color_panel_text, &OPainterColorPanel::weightChange, this,
    //          &OPainterToolBox::onTextWeightChange);

    //  ui->toolbox_pen->installEventFilter(this);
    //  ui->toolbox_text->installEventFilter(this);

    setMouseTracking(true);

    // 设置样式
    //  QString qss = base::Files::readStringAll(":/res/qss/toolbox.qss");
    //  setStyleSheet(qss);

    //  ui->tool_panel
    //    base::Widgets::SetShadowEffect(ui->tool_panel, {0, 0}, QColor(qsl("#535353")), 20);

    m_textColorPanel = std::make_unique<OPainterColorPanel>(this);
    connect(m_textColorPanel.get(), &OPainterColorPanel::colorChange, this,
            &OPainterToolBox::onTextColorChange);
    connect(m_textColorPanel.get(), &OPainterColorPanel::weightChange, this,
            &OPainterToolBox::onTextWeightChange);

    m_penColorPanel = std::make_unique<OPainterColorPanel>(this);
    connect(m_penColorPanel.get(), &OPainterColorPanel::colorChange, this,
            &OPainterToolBox::onPenColorChange);
    connect(m_penColorPanel.get(), &OPainterColorPanel::weightChange, this,
            &OPainterToolBox::onPenWeightChange);

    ui->layout->insertWidget(0, new lib::ui::MoveableBar(this));
    //module--classroom--OPainterToolBox{
    setStyleSheet("#OPainterToolBox{background-color: #535353; border-radius: 8px;}");
}

OPainterToolBox::~OPainterToolBox() {
    delete ui;
}

void OPainterToolBox::on_toolbox_move_clicked(bool checked) {
    if (!checked) return;
    ui->toolbox_move->setChecked(checked);
    ui->toolbox_text->setChecked(false);
    ui->toolbox_pen->setChecked(false);
    ui->toolbox_delete->setChecked(false);
    ui->toolbox_cutter->setCheckable(false);
    emit toolChange(ToolboxType::P_MOVE);
}

void OPainterToolBox::on_toolbox_text_clicked(bool checked) {
    qDebug() << __func__ << checked;
    m_textColorPanel->move(GetCurPos());
    m_textColorPanel->show();
    m_penColorPanel->hide(true);

    ui->toolbox_text->setChecked(true);
    ui->toolbox_move->setChecked(false);
    ui->toolbox_pen->setChecked(false);
    ui->toolbox_delete->setChecked(false);
    ui->toolbox_cutter->setCheckable(false);

    emit toolChange(ToolboxType::P_TEXT);
}

void OPainterToolBox::on_toolbox_pen_clicked(bool checked) {
    qDebug() << "on_toolbox_pen_clicked" << checked;

    m_penColorPanel->move(GetCurPos());
    m_penColorPanel->show();
    m_textColorPanel->hide(true);

    ui->toolbox_pen->setChecked(true);
    ui->toolbox_move->setChecked(false);
    ui->toolbox_text->setChecked(false);
    ui->toolbox_delete->setChecked(false);
    ui->toolbox_cutter->setCheckable(false);
    emit toolChange(ToolboxType::P_PEN);
}

void OPainterToolBox::on_toolbox_delete_clicked(bool checked) {
    if (!checked) return;

    ui->toolbox_delete->setChecked(checked);
    ui->toolbox_pen->setChecked(false);
    ui->toolbox_move->setChecked(false);
    ui->toolbox_text->setChecked(false);
    ui->toolbox_cutter->setCheckable(false);
    emit toolChange(ToolboxType::P_REMOVE);
}

void OPainterToolBox::on_toolbox_cutter_clicked(bool checked) {
    emit toolChange(ToolboxType::P_MOVE);
}

void OPainterToolBox::on_toolbox_cloud_clicked(bool checked) {
    emit toolChange(ToolboxType::P_CLOUD);
}

void OPainterToolBox::onTextColorChange(QColor color) {
    emit textColorChange(color);
}

void OPainterToolBox::onTextWeightChange(int weight) {
    emit textWeightChange(weight);
}

void OPainterToolBox::onPenColorChange(QColor color) {
    emit penColorChange(color);
}

void OPainterToolBox::onPenWeightChange(int weight) {
    emit penWeightChange(weight);
}

// bool OPainterToolBox::eventFilter(QObject *target, QEvent *event) {
//  获取鼠标位置
//   auto curPos = QCursor().pos();
//   curPos.setX(curPos.x() - 220);
//
//   QToolButton *button = qobject_cast<QToolButton *>(target);
//   if (button) {
//     if (button == ui->toolbox_text) {
//       if (event->type() == QEvent::Type::HoverEnter) {
//         m_textColorPanel->move(curPos);
//         m_textColorPanel->show();
//       }
//     } else if (button == ui->toolbox_pen) {
//       if (event->type() == QEvent::Type::HoverEnter) {
//         m_penColorPanel->move(curPos);
//         m_penColorPanel->show();
//       }
//     }
//   }
// QToolButton:hover, QToolButton:!hover:checked {
//   background-color: #28292c;
// border:1px solid #7d7d7d;
// }
//   return QWidget::eventFilter(target, event);
// }

void OPainterToolBox::enterEvent(QEvent* event) {}

void OPainterToolBox::leaveEvent(QEvent* event) {}

void OPainterToolBox::mouseMoveEvent(QMouseEvent* e) {
    if (this->parentWidget()) {
        this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        QPoint pt = this->mapTo(this->parentWidget(), e->pos());
        QWidget* w = this->parentWidget()->childAt(pt);
        if (w) {
            pt = w->mapFrom(this->parentWidget(), pt);
            QMouseEvent* event =
                    new QMouseEvent(e->type(), pt, e->button(), e->buttons(), e->modifiers());
            QApplication::postEvent(w, event);
        }
        this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
}
}  // namespace module::classroom
