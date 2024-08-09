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

#include "RecursiveSignalBlocker.h"

#include <QObject>
#include <QSignalBlocker>

/**
@class  RecursiveSignalBlocker
@brief  Recursively blocks all signals from an object and its children.
@note   All children must be created before the blocker is used.

Wraps a QSignalBlocker on each object. Signals will be unblocked when the
blocker gets destroyed. According to QSignalBlocker, we are also exception safe.
*/

/**
@brief      Creates a QSignalBlocker recursively on the object and child objects.
@param[in]  object  the object, which signals should be blocked
*/
namespace ok::base
{
RecursiveSignalBlocker::RecursiveSignalBlocker(QObject* object) { recursiveBlock(object); }

RecursiveSignalBlocker::~RecursiveSignalBlocker() { qDeleteAll(mBlockers); }

/**
@brief      Recursively blocks all signals of the object.
@param[in]  object  the object to block
*/
void RecursiveSignalBlocker::recursiveBlock(QObject* object) {
    mBlockers << new QSignalBlocker(object);

    for (QObject* child : object->children()) {
        recursiveBlock(child);
    }
}
}
