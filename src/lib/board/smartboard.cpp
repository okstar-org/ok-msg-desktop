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

#include "smartboard.h"

namespace lib::board {
DrawItem::DrawItem() {}

DrawItem::~DrawItem() {}

const DrawId& DrawItem::id() const {
    return _id;
}

Action DrawItem::action() const {
    return _action;
}

void DrawItem::setAction(Action action) {
    _action = action;
}

const std::vector<Point>& DrawItem::points() const {
    return _points;
}

DrawType DrawItem::type() {
    return _type;
}

/**
 * ControllerItem
 *
 */
ControllerItem::ControllerItem() /*: Plugin()*/ {}

ControllerItem::ControllerItem(const DrawId& id) : _id(id) /*, Plugin()*/ {}

ControllerItem::~ControllerItem() {}

Action ControllerItem::action() const {
    return _action;
}

void ControllerItem::setAction(Action action) {
    _action = action;
}
ControllerType ControllerItem::type() const {
    return _type;
}
}  // namespace lib::board
