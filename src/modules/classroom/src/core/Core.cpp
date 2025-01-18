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

#include "Core.h"
#include "application.h"
#include "lib/board/Board.h"

namespace module::classroom {

static Core* core = nullptr;

Core::Core(QObject* parent) : QThread(parent) {
    auto profile = ok::Application::Instance()->getProfile();
    messenger = profile->getMessenger();

    board = std::make_unique<lib::board::Board>(messenger->im());
}

Core::~Core() {
    qDebug() << __func__;
}

Core* Core::Instance() {
    if (!core) {
        core = new Core();
    }
    return core;
}

void Core::Destroy() {
    core->deleteLater();
}

void Core::sendDraw(std::shared_ptr<lib::board::SmartBoardDraw> draw) {
    qDebug() << __func__ << "draw:" << qstring(draw->id());
    board->sendDraw(draw);
}

void Core::run() {}

}  // namespace module::classroom
