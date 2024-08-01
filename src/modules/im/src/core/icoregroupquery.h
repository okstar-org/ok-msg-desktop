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

#ifndef ICORE_GROUP_QUERY_H
#define ICORE_GROUP_QUERY_H

#include "FriendId.h"
#include "groupid.h"

#include <QString>
#include <QStringList>

#include <cstdint>

class ICoreGroupQuery {
public:
    virtual ~ICoreGroupQuery() = default;
    virtual GroupId getGroupPersistentId(QString groupId) const = 0;
    virtual uint32_t getGroupNumberPeers(QString groupId) const = 0;
    virtual QString getGroupPeerName(QString groupId, QString peerId) const = 0;
    virtual ToxPeer getGroupPeerPk(QString groupId, QString peerId) const = 0;
    virtual QStringList getGroupPeerNames(QString groupId) const = 0;
    virtual bool getGroupAvEnabled(QString groupId) const = 0;
};

#endif /*ICORE_GROUP_QUERY_H*/
