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

#include "Defines.h"

namespace gloox {
class Plugin;
class Tag;
class PersonalEventingProtocolFilter;
}  // namespace gloox

namespace lib::board {

class Controller {
private:
    std::string _id;
    UserList _userList;
    ControllerMap _plugins;

public:
    explicit Controller(const std::string&);
    Controller(const gloox::Tag*, const gloox::PersonalEventingProtocolFilter* filter);

    const UserList& userList() const;

    template <typename _ControllerItem> const _ControllerItem* findPlugin(ControllerType type) {
        auto it = _plugins.find(type);
        if (it == _plugins.end()) {
            // 没找到
            return nullptr;
        } else {
            // 找到
            return (_ControllerItem*)(it->second);
        }
    }

    template <typename _ControllerItem> void addPlugin(_ControllerItem* drawItem) {
        _plugins.insert(std::make_pair(drawItem->type(), dynamic_cast<ControllerItem*>(drawItem)));
    }
};
}  // namespace lib::board
