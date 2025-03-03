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

#include <QApplication>
#include <QVBoxLayout>
#include <QAction>
#include "lib/storage/settings/style.h"
#include "lib/ui/widget/MoveableBar.h"

static QPoint GetCurPos() {
    auto curPos = QCursor().pos();
    curPos.setX(curPos.x() - 220);
    return curPos;
}

namespace module::classroom {

OPainterToolBox::OPainterToolBox(QWidget* parent)
        : lib::ui::OFrame(parent)
        , _delayCaller(std::make_unique<base::DelayedCallTimer>()) {

    setMouseTracking(true);
    setObjectName("OPainterToolBox");
    setContentsMargins(0,0,0,0);

    setFixedSize(QSize(40, 220));

    //工具栏宽尺寸40*220
    //

    auto* vBox = new QVBoxLayout(this);
    vBox->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    vBox->setSpacing(5);

    vBox->addWidget(new lib::ui::MoveableBar(this));

    toolbox_pen = new QToolButton(this);
    toolbox_pen->setIcon(QIcon(":/res/icon/toolbox/toolbox_pen.png"));
    connect(toolbox_pen, &QToolButton::clicked, this,
            [&](){
                emit toolChange(ToolboxType::P_PEN);
            });

    vBox->addWidget(toolbox_pen, Qt::AlignHCenter);

    toolbox_text = new QToolButton(this);
    toolbox_text->setIcon(QIcon(":/res/icon/toolbox/toolbox_text.png"));
    connect(toolbox_text, &QToolButton::clicked, this,
            [&](){
                emit toolChange(ToolboxType::P_TEXT);
            });
    vBox->addWidget(toolbox_text, Qt::AlignHCenter);

     //toolbox_move.png
    toolbox_move = new QToolButton(this);
    toolbox_move->setIcon(QIcon(":/res/icon/toolbox/toolbox_move.png"));
    connect(toolbox_move, &QToolButton::clicked, this,
            &OPainterToolBox::on_toolbox_move_clicked);
    vBox->addWidget(toolbox_move, Qt::AlignHCenter);


    toolbox_delete = new QToolButton(this);
    toolbox_delete->setIcon(QIcon(":/res/icon/toolbox/toolbox_delete.png"));
    connect(toolbox_delete, &QToolButton::clicked, this,
            [&](){
                emit toolChange(ToolboxType::P_REMOVE);
            });
    vBox->addWidget(toolbox_delete);

    toolbox_cutter = new QToolButton(this);
    toolbox_cutter->setIcon(QIcon(":/res/icon/toolbox/toolbox_cutter.png"));
    connect(toolbox_cutter, &QToolButton::clicked, this,
            [&](){
                emit toolChange(ToolboxType::P_CUTTER);
            });
    vBox->addWidget(toolbox_cutter);

    toolbox_cloud = new QToolButton(this);
    toolbox_cloud->setIcon(QIcon(":/res/icon/toolbox/toolbox_cloud.png"));
    connect(toolbox_cutter, &QToolButton::clicked, this,
            [&](){
                emit toolChange(ToolboxType::P_CLOUD);
            });
    vBox->addWidget(toolbox_cloud);

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


    setLayout(vBox);

    QString qss = lib::settings::Style::getStylesheet("toolbox.css");
    setStyleSheet(qss);
}

OPainterToolBox::~OPainterToolBox() {
    // delete ui;
}

void OPainterToolBox::on_toolbox_move_clicked(bool checked) {
    if (!checked) return;
    toolbox_move->setChecked(checked);
    toolbox_text->setChecked(false);
    toolbox_pen->setChecked(false);
    toolbox_delete->setChecked(false);
    toolbox_cutter->setCheckable(false);
    emit toolChange(ToolboxType::P_MOVE);
}

void OPainterToolBox::on_toolbox_text_clicked(bool checked) {
    qDebug() << __func__ << checked;
    m_textColorPanel->move(GetCurPos());
    m_textColorPanel->show();
    m_penColorPanel->hide(true);

    toolbox_text->setChecked(true);
    toolbox_move->setChecked(false);
    toolbox_pen->setChecked(false);
    toolbox_delete->setChecked(false);
    toolbox_cutter->setCheckable(false);

    emit toolChange(ToolboxType::P_TEXT);
}

void OPainterToolBox::on_toolbox_pen_clicked(bool checked) {
    qDebug() << __func__ << checked;

    m_penColorPanel->move(GetCurPos());
    m_penColorPanel->show();
    m_textColorPanel->hide(true);

    toolbox_pen->setChecked(true);
    toolbox_move->setChecked(false);
    toolbox_text->setChecked(false);
    toolbox_delete->setChecked(false);
    toolbox_cutter->setCheckable(false);
    emit toolChange(ToolboxType::P_PEN);
}

void OPainterToolBox::on_toolbox_delete_clicked(bool checked) {
    if (!checked) return;

    toolbox_delete->setChecked(checked);
    toolbox_pen->setChecked(false);
    toolbox_move->setChecked(false);
    toolbox_text->setChecked(false);
    toolbox_cutter->setCheckable(false);
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

void OPainterToolBox::reloadTheme()
{

}
}  // namespace module::classroom
