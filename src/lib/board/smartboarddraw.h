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

#include <map>
#include <string>
#include "smartboard.h"
#include "smartboarddrawline.h"

namespace gloox {
class Plugin;
class Tag;
class PersonalEventingProtocolFilter;
}  // namespace gloox

namespace lib::board {

using Style = std::map<std::string, std::string>;

class SmartBoardDraw : public ITag {
    std::string _id;
    std::map<DrawType, DrawItem*> _plugins;
    Style _style;
    Point _point;
    Position _position;

public:
    SmartBoardDraw();
    SmartBoardDraw(const gloox::Tag*, const gloox::PersonalEventingProtocolFilter* filter);

    const std::string& id() const;

    Style& style();

    const Point& point() const;

    const Position& position() const;
    void setPosition(const Position&);

    template <typename _DrawItem> const _DrawItem* findPlugin(DrawType type) {
        auto it = _plugins.find(type);
        if (it == _plugins.end()) {
            // 没找到
            return nullptr;
        } else {
            // 找到
            return (_DrawItem*)(it->second);
        }
    }

    template <typename _DrawItem> void addPlugin(_DrawItem* drawItem) {
        _plugins.insert(std::make_pair(drawItem->type(), dynamic_cast<DrawItem*>(drawItem)));
    }
};

// end of SmartBoard
}  // namespace lib::board
