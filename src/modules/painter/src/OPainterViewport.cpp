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
#include "OPainterViewport.h"
#include "ui_OPainterViewport.h"

namespace module::painter {

OPainterViewport::OPainterViewport(QWidget* parent) : QFrame(parent), ui(new Ui::OPainterViewport) {
    ui->setupUi(this);

    //    QLayout *localLayout = layout();

    // 白板
    //     _wbWidget = std::make_unique<UI::widget::WhiteboardWidget>(this);
    //     layout()->addWidget(_wbWidget.get());

    // 聊天
    //     _imLayout = std::make_unique<UI::layout::IMLayout>(this);
    //     _imLayout->hide();

    // 教材
    //     _webView = std::make_unique<UI::widget::MaterialView>();
    //     _webView->move(200, 160);
    //     _webView->hide();
}

OPainterViewport::~OPainterViewport() {
    delete ui;
}

void OPainterViewport::showEvent(QShowEvent* e) {
    Q_UNUSED(e);

    _inited = true;
}

void OPainterViewport::resizeEvent(QResizeEvent* e) {
    Q_UNUSED(e);
    //  if (ui->oIMViewport)
    //  ui->oIMViewport->setFixedHeight(e->size().height());
}

void OPainterViewport::toggleChat(bool checked) {
    Q_UNUSED(checked);

    // TODO
    //  qDebug() << checked;
    //  if (checked)
    //    ui->oIMViewport->show();
    //  else
    //    ui->oIMViewport->hide();
}
}  // namespace module::painter