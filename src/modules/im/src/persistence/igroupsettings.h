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

#ifndef IGROUP_SETTINGS_H
#define IGROUP_SETTINGS_H

#include "src/base/interface.h"

#include <QStringList>

class IGroupSettings {
public:
    virtual ~IGroupSettings() = default;
    virtual QStringList getBlackList() const = 0;
    virtual void setBlackList(const QStringList& blist) = 0;
    virtual bool getGroupAlwaysNotify() const = 0;
    virtual void setGroupAlwaysNotify(bool newValue) = 0;

    virtual bool getShowGroupJoinLeaveMessages() const = 0;
    virtual void setShowGroupJoinLeaveMessages(bool newValue) = 0;

    DECLARE_SIGNAL(blackListChanged, QStringList const& blist);
    DECLARE_SIGNAL(showGroupJoinLeaveMessagesChanged, bool show);
};

#endif /*IGROUP_SETTINGS_H*/
