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

#include "grouplist.h"
#include <QDebug>
#include <QHash>
#include "src/core/core.h"
#include "src/model/group.h"

GroupMap GroupList::groupMap;

Group* GroupList::addGroup(const GroupId& groupId,
                           const QString& name,
                           bool isAvGroupchat,
                           const QString& selfName) {
    qDebug() << "addGroup" << "groupId" << groupId.toString();

    auto checker = groupMap.value(groupId.toString());
    if (checker) {
        qWarning() << "addGroup: groupId already taken";
        return checker;
    }

    auto core = Core::getInstance();
    Group* newGroup = new Group(groupId, name, isAvGroupchat, selfName, *core, *core);
    groupMap[groupId.toString()] = newGroup;

    return newGroup;
}

Group* GroupList::findGroup(const GroupId& groupId) { return groupMap.value(groupId.toString()); }

void GroupList::removeGroup(const GroupId& groupId, bool /*fake*/) {
    auto g_it = groupMap.find(groupId.toString());
    if (g_it != groupMap.end()) {
        delete *g_it;
        groupMap.erase(g_it);
    }
}

QList<Group*> GroupList::getAllGroups() {
    QList<Group*> res;

    for (auto it : groupMap) res.append(it);

    return res;
}

void GroupList::clear() {
    for (auto groupptr : groupMap) delete groupptr;
    groupMap.clear();
}
