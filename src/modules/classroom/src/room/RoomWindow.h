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

#include <QWidget>
#include "core/Core.h"
#include "lib/messenger/Messenger.h"
#include "lib/ortc/ok_rtc.h"


namespace module::classroom {

class RoomWindow : public QWidget {
    Q_OBJECT
public:
    explicit RoomWindow(const QString& roomName,
                        const lib::ortc::CtrlState& state,
                        QWidget* parent = nullptr);
    ~RoomWindow() override;
    void doLeaveMeet();
    void startCounter();
    void stopCounter();



private:
   Core* core;

signals:
    /**
     * 离开
     */
    void roomLeft();
    void roomCreated();
    void roomDestroyed();
    void participantJoined(const lib::messenger::Participant& part);

};

}  // namespace module::classroom
