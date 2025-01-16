﻿/*
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
#ifndef OPAINTERTOOLBOX_H
#define OPAINTERTOOLBOX_H

#include <QMouseEvent>
#include <QWidget>

#include "Base.h"
#include "OPainterColorPanel.h"
#include "OPainterToolBox.h"
#include "base/timer.h"

namespace Ui {
class OPainterToolBox;
}

namespace module::painter {

class OPainterToolBox : public QWidget {
    Q_OBJECT
public:
    explicit OPainterToolBox(QWidget* parent = nullptr);
    ~OPainterToolBox();

protected:
    //  bool eventFilter(QObject *target, QEvent *event);

    void leaveEvent(QEvent* event);
    void enterEvent(QEvent* event);
    void mouseMoveEvent(QMouseEvent* e);

private:
    Ui::OPainterToolBox* ui;

    std::unique_ptr<base::DelayedCallTimer> _delayCaller;

    std::unique_ptr<OPainterColorPanel> m_textColorPanel;
    std::unique_ptr<OPainterColorPanel> m_penColorPanel;

signals:

    void toolChange(painter::ToolboxType);

    void textColorChange(QColor color);
    void textWeightChange(int weight);

    void penColorChange(QColor color);
    void penWeightChange(int weight);

public slots:
    void onTextColorChange(QColor color);
    void onTextWeightChange(int weight);

    void onPenColorChange(QColor color);
    void onPenWeightChange(int weight);

    void on_toolbox_move_clicked(bool);
    void on_toolbox_text_clicked(bool);
    void on_toolbox_pen_clicked(bool);
    void on_toolbox_delete_clicked(bool);
    void on_toolbox_cutter_clicked(bool);
    void on_toolbox_cloud_clicked(bool);
};
}  // namespace module::painter
#endif  // OPAINTERTOOLBOX_H
