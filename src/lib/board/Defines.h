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
#include <vector>

namespace gloox {
class Tag;
}

namespace lib::board {

const static std::string XMPP_SMARTBOARD_DRAW = "https://okstar.org/board/draw/0";
const static std::string XMPP_SMARTBOARD_CONTROLLER = "https://okstar.org/board/controller/0";

class ITag {
public:
    virtual gloox::Tag* tag() = 0;
};

enum DrawType { Line, Text, Move, File, Remove };

enum ControllerType {
    Select,
    Voice,
};

enum Action { Create, Update, ON, OFF };

struct Point {
    double x = 0;
    double y = 0;
};

struct Position {
    int x = 0;
    int y = 0;
    int z = 0;
};

using DrawId = std::string;
using PointList = std::vector<Point>;
using UserList = std::vector<std::string>;

class DrawItem {
protected:
    DrawType _type;
    DrawId _id;
    Action _action;

    std::vector<Point> _points;

public:
    DrawItem();
    ~DrawItem();

    const DrawId& id() const;

    Action action() const;
    void setAction(Action);

    // const std::map<std::string, std::string> &style() const;
    const std::vector<Point>& points() const;

    DrawType type();
};

class ControllerItem {
protected:
    ControllerType _type;
    DrawId _id;
    Action _action;

public:
    ControllerItem();
    ControllerItem(const DrawId&);
    ~ControllerItem();

    Action action() const;
    void setAction(Action);

    ControllerType type() const;
};

using ControllerMap = std::map<ControllerType, ControllerItem*>;
using DrawMap = std::map<DrawType, DrawItem*>;


}  // namespace lib::board
