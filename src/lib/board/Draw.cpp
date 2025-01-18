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

#include "Draw.h"

#include <tag.h>

namespace lib::board {

SmartBoardDraw::SmartBoardDraw() {}

const std::string& SmartBoardDraw::id() const {
    return _id;
}

std::map<std::string, std::string>& SmartBoardDraw::style() {
    return _style;
}

const Point& SmartBoardDraw::point() const {
    return _point;
}

const Position& SmartBoardDraw::position() const {
    return _position;
}

void SmartBoardDraw::setPosition(const Position& position) {
    _position = position;
}

gloox::Tag* SmartBoardDraw::tag() {
    auto* t = new gloox::Tag("draw", gloox::XMLNS, XMPP_SMARTBOARD_DRAW);
    t->addAttribute("id", _id);

    //<style>
    //  <color>red</color>
    //</style>

    auto style = new gloox::Tag("style");
    for (auto& sty : _style) {
        style->addChild(new gloox::Tag(sty.first, sty.second));
    }
    t->addChild(style);

    auto pos = new gloox::Tag("pos");
    /**
     * <pos x=0 y=0 z=0></pos>
     */
    pos->addAttribute("x", _position.x);
    pos->addAttribute("y", _position.y);
    pos->addAttribute("z", _position.z);
    t->addChild(pos);

    return t;
}

}  // namespace lib::board
