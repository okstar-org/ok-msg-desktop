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

#include "DrawMove.h"

namespace lib::board {
DrawMove::DrawMove() {}

void DrawMove::setMoveId(const DrawId& id) {
    _id = id;
}

const DrawId& DrawMove::moveId() const {
    return _id;
}

void DrawMove::setPosition(const Position& position) {
    _position = position;
}

const Position& DrawMove::position() const {
    return _position;
}

}  // namespace lib::board
