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
#include "src/core/core.h"
#include "src/model/group.h"
#include <QDebug>
#include <QHash>

QHash<const GroupId, Group*> GroupList::groupList;
QHash<QString, GroupId> GroupList::id2key;
Group* GroupList::addGroup(QString groupNum, const GroupId& groupId,
                           const QString& name, bool isAvGroupchat,
                           const QString& selfName)
{
    qDebug()<<"addGroup"<<groupNum << "groupId" << groupId.toString();

    auto checker = groupList.find(groupId);
    if (checker != groupList.end())
        qWarning() << "addGroup: groupId already taken";

    // TODO: Core instance is bad but grouplist is also an instance so we can
    // deal with this later
    auto core = Core::getInstance();
    Group* newGroup = new Group(groupNum, groupId, name, isAvGroupchat, selfName, *core, *core);
    groupList[groupId] = newGroup;
    id2key[groupNum] = groupId;
    return newGroup;
}

Group* GroupList::findGroup(const GroupId& groupId)
{
    auto g_it = groupList.find(groupId);
    if (g_it != groupList.end())
        return *g_it;

    return nullptr;
}

const GroupId& GroupList::id2Key(QString groupNum)
{
    return id2key[groupNum];
}

void GroupList::removeGroup(const GroupId& groupId, bool /*fake*/)
{
    auto g_it = groupList.find(groupId);
    if (g_it != groupList.end()) {
        groupList.erase(g_it);
    }
}

QList<Group*> GroupList::getAllGroups()
{
    QList<Group*> res;

    for (auto it : groupList)
        res.append(it);

    return res;
}

void GroupList::clear()
{
    for (auto groupptr : groupList)
        delete groupptr;
    groupList.clear();
}
