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

#pragma once

#ifdef Q_OS_WIN
#undef DrawText  // 避免windows宏冲突
#endif

#include <QObject>
#include <memory>
#include "Controller.h"
#include "Draw.h"
#include "lib/messenger/messenger.h"

namespace lib::board {

class Bridge;

class Board : public QObject {
    Q_OBJECT
public:
    explicit Board(messenger::IM* im, QObject* parent = nullptr);
    ~Board() override;

    void sendDraw(std::shared_ptr<SmartBoardDraw> draw);
    void receiveDraw(SmartBoardDraw* draw);
    void sendController(std::shared_ptr<Controller> controller);
    void receiveController(Controller* controller);

protected:
private:
    std::unique_ptr<Bridge> bridge;

signals:
    void receivedDraw(SmartBoardDraw*);
    void receivedController(Controller*);
    void ss(const std::list<std::string>&, bool mute);
};

}  // namespace lib::board
