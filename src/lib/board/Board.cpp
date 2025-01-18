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

#include "Board.h"

#include <base/logs.h>
#include <gloox.h>
#include <pubsubevent.h>
#include <pubsubitem.h>

#include "Bridge.h"
#include "Controller.h"
#include "Draw.h"
#include "lib/messenger/IM.h"

namespace lib::board {

const static std::string PUBSUB_NODE = "SmartBoard";
const static std::string PUBSUB_SUBDOMAIN = "pubsub";

Board::Board(messenger::IM* im, QObject* parent) : QObject(parent) {
    qDebug() << __func__;
    BridgeConf conf = {.node = PUBSUB_NODE, .subdomain = PUBSUB_SUBDOMAIN + "." + im->host()};
    bridge = std::make_unique<Bridge>(im, conf);
}

Board::~Board() {
    qDebug() << __func__;
}

void Board::sendDraw(std::shared_ptr<SmartBoardDraw> draw) {
    qDebug() << __func__ << draw.get();
    if (!draw) {
        qWarning() << "draw is nullptr!";
        return;
    }

    auto item = new gloox::PubSub::Item();
    item->setPayload(draw->tag());

    gloox::PubSub::ItemList items;
    items.push_back(item);
    bridge->sendEvent(items);
}

void Board::receiveDraw(SmartBoardDraw* draw) {
    qDebug() << draw;
    emit receivedDraw(draw);
}

void Board::sendController(std::shared_ptr<Controller> controller) {
    qDebug() << controller.get();
    if (!controller) {
        return;
    }

    gloox::PubSub::ItemList items;

    gloox::PubSub::Item* item = new gloox::PubSub::Item();
    //    item->setPayload(controller->tag());

    items.push_back(item);
    bridge->sendEvent(items);
}

void Board::receiveController(Controller* controller) {
    qDebug() << controller;
    if (!controller) return;

    // TODO
    // const UserList &userList = controller->userList();
    // const ControllerVoice *cVoice =
    // controller->findPlugin<ControllerVoice>(ControllerType::Voice); if
    // (cVoice)
    // {

    // bool mute = cVoice->action() == Action::ON;

    //
    // emit ss(userList, mute);
    // }
    //    emit receivedController(controller);
}

}  // namespace lib::board
