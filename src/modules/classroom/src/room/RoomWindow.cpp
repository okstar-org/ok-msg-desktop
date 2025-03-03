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

//
// Created by gaojie on 25-1-17.
//

#include "RoomWindow.h"

#include <QGridLayout>
#include "OVideoViewport.h"
#include "application.h"
#include "painter/OPainterViewport.h"

namespace module::classroom {

RoomWindow::RoomWindow(const QString& roomName,
                       const lib::ortc::CtrlState& state, QWidget* parent)
        : QWidget(parent)  {
    qDebug() << __func__;
    setMinimumSize(WindowSize());
    setAttribute(Qt::WA_QuitOnClose, true);
    setAttribute(Qt::WA_DeleteOnClose, true);


    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);

    auto video = new OVideoViewport(this);
    layout->addWidget(video);

    auto viewport = new OPainterViewport(this);
    layout->addWidget(viewport);

    setLayout(layout);

    core = Core::Instance();
    core->start();
}

RoomWindow::~RoomWindow() {
    qDebug() << __func__;
    core->quit();
    Core::Destroy();
}

void RoomWindow::doLeaveMeet() {}
void RoomWindow::startCounter() {}
void RoomWindow::stopCounter() {}

}  // namespace module::classroom
