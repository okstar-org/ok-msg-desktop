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

#include "painterevent.h"

namespace module::classroom {

PainterEvent::PainterEvent() {}

PainterEvent::~PainterEvent() {}

void PainterEvent::onMoveItem(std::shared_ptr<CPaintItem> item) {
    _manager->movePaintItem(item);
}

void PainterEvent::onDrawItem(std::shared_ptr<CPaintItem> item) {
    _manager->addPaintItem(item);
}

void PainterEvent::onUpdateItem(std::shared_ptr<CPaintItem> item) {
    _manager->updatePaintItem(item);
}

void PainterEvent::onRemoveItem(std::shared_ptr<CPaintItem> item) {
    _manager->removePaintItem(item);
}

QString PainterEvent::onGetToolTipText(std::shared_ptr<CPaintItem> item) {
    return "";
}

std::shared_ptr<CPaintItem> PainterEvent::onFindItem(ItemId itemId) {
    return _manager->findPaintItem("", itemId);
}

}  // namespace module::classroom
