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

#pragma once

#include <lib/board/Board.h>
#include <QThread>
#include "lib/board/Draw.h"
#include "lib/messenger/messenger.h"

namespace module::classroom {

class Core : public QThread {
    Q_OBJECT
public:
    explicit Core(QObject* parent = nullptr);
    ~Core() override;

    static Core* Instance();
    static void Destroy();

    lib::messenger::Messenger* getMessenger() {
        return messenger;
    }

    void sendDraw(std::shared_ptr<lib::board::SmartBoardDraw> draw);

protected:
    void run() override;

private:
    lib::messenger::Messenger* messenger;
    std::unique_ptr<lib::board::Board> board;
};
}  // namespace module::classroom
