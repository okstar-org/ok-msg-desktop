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

#include "PainterDispatcher.h"

#include "PaintItem.h"

#include <QColor>
#include <QString>
#include <memory>

#include "lib/board/Draw.h"
#include "lib/board/DrawMove.h"
#include "lib/board/DrawRemove.h"

namespace module::classroom {

#undef DrawText  // XXX resolve conflict DrawText in WinUser.h

using ColorHelper = base::ColorHelper;

PainterDispatcher::PainterDispatcher(): core(Core::Instance()) {
    qDebug() << __func__ ;
}

PainterDispatcher::~PainterDispatcher()
{
    qDebug() << __func__;
}

void PainterDispatcher::onISharedPaintEvent_AddTask(int totalTaskCount, bool playBackWorking) {}

void PainterDispatcher::onISharedPaintEvent_AddPaintItem(std::shared_ptr<CPaintItem> item) {
    qDebug() << __func__;
    std::shared_ptr<lib::board::SmartBoardDraw> draw = item->serialize();
    core->sendDraw(draw);
}

void PainterDispatcher::onISharedPaintEvent_RemovePaintItem(std::shared_ptr<CPaintItem> item) {
    qDebug() << __func__;

    std::shared_ptr<lib::board::SmartBoardDraw> draw =
            std::make_shared<lib::board::SmartBoardDraw>();

    auto move = new lib::board::DrawRemove();
    move->setRemoveId(item->itemId());
    draw->addPlugin(move);

    core->sendDraw(draw);
}

void PainterDispatcher::onISharedPaintEvent_MovePaintItem(std::shared_ptr<CPaintItem> item,
                                                          double x,
                                                          double y) {
    qDebug() << __func__;

    std::shared_ptr<lib::board::SmartBoardDraw> draw =
            std::make_shared<lib::board::SmartBoardDraw>();

    auto move = new lib::board::DrawMove();
    move->setMoveId(item->itemId());
    move->setPosition({(int)x, (int)y, 0});
    draw->addPlugin(move);

    core->sendDraw(draw);
}

void PainterDispatcher::onISharedPaintEvent_UpdatePaintItem(std::shared_ptr<CPaintItem> item) {
    qDebug() << __func__;
    item->setAction(CPaintItem::Action::Update);
    std::shared_ptr<lib::board::SmartBoardDraw> draw = item->serialize();
    core->sendDraw(draw);
}

}  // namespace module::classroom
