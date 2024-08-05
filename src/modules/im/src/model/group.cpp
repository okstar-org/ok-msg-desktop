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

#include "group.h"
#include <QDebug>
#include "friend.h"
#include "src/core/FriendId.h"
#include "src/core/contactid.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/groupid.h"
#include "src/friendlist.h"
#include "src/persistence/settings.h"
#include "src/widget/form/groupchatform.h"
#include "src/widget/groupwidget.h"

static const int MAX_GROUP_TITLE_LENGTH = 128;

namespace {

Group::Role parseRole(const QString& role) {
    if (role == "moderator") {
        return Group::Role::Moderator;
    } else if (role == "participant") {
        return Group::Role::Participant;
    } else if (role == "visitor") {
        return Group::Role::Visitor;
    }
    return Group::Role::None;
}

Group::Affiliation parseAffiliation(const QString& affiliation) {
    if (affiliation == "admin") {
        return Group::Affiliation::Admin;
    } else if (affiliation == "owner") {
        return Group::Affiliation::Owner;
    } else if (affiliation == "member") {
        return Group::Affiliation::Member;
    } else if (affiliation == "outcast") {
        return Group::Affiliation::Outcast;
    }
    return Group::Affiliation::None;
}

}  // namespace

Group::Group(const GroupId groupId_, const QString& name, bool isAvGroupchat,
             const QString& selfName, ICoreGroupQuery& groupQuery, ICoreIdHandler& idHandler)
        : Contact(groupId_, name, name, true)
        , groupId{groupId_}
        , avGroupchat{isAvGroupchat}
        , groupQuery(groupQuery)
        , idHandler(idHandler)
        , role{Role::None}
        , affiliation{Affiliation::None} {
    // in groupchats, we only notify on messages containing your name <-- dumb
    // sound notifications should be on all messages, but system popup
    // notification on naming is appropriate
    hasNewMessages = 0;
    userWasMentioned = 0;
}

void Group::setSubject(const QString& author, const QString& newTitle) {
    const QString shortTitle = newTitle.left(MAX_GROUP_TITLE_LENGTH);
    if (!shortTitle.isEmpty() && subject != shortTitle) {
        subject = shortTitle;
        emit subjectChanged(author, subject);
    }
}

void Group::updateUsername(const QString oldName, const QString newName) {
    //  const QString displayName = FriendList::decideNickname(pk, newName);
    //  qDebug() <<"updateUsername=>" << displayName;

    if (!peerDisplayNames.contains(oldName)) {
        return;
    }

    if (peerDisplayNames[oldName] != newName) {
        peerDisplayNames[newName] = newName;
        emit peerNameChanged(oldName, newName);
    }
}

bool Group::isAvGroupchat() const { return avGroupchat; }

int Group::getPeersCount() const { return peerCount; }

void Group::setPeerCount(uint32_t count) {
    peerCount = count;
    emit peerCountChanged(peerCount);
}

/**
 * @brief Gets the PKs and names of all peers
 * @return PKs and names of all peers, including our own PK and name
 */
const QMap<QString, QString>& Group::getPeerList() const { return peerDisplayNames; }

QString Group::getPeerDisplayName(const QString& resource) {
    return peerDisplayNames.value(resource, resource);
}

void Group::addPeer(const GroupOccupant& occ) {
    qDebug() << __func__ << occ.jid << occ.nick;

    peerDisplayNames[ToxPeer(occ.jid).resource] = occ.nick;

    // 判断成员是否为自己
    auto core = Core::getInstance();
    auto selfId = core->getSelfId();
    if (selfId.toString() == ContactId(occ.jid).toString()) {
        role = parseRole(occ.role);
        affiliation = parseAffiliation(occ.affiliation);
        statusCodes = occ.codes;
        emit privilegesChanged(role, affiliation, statusCodes);
    }
}

void Group::setEventFlag(bool f) { hasNewMessages = f; }

bool Group::getEventFlag() const { return hasNewMessages; }

void Group::setMentionedFlag(bool f) { userWasMentioned = f; }

bool Group::getMentionedFlag() const { return userWasMentioned; }

void Group::setDesc(const QString& desc_) {
    desc = desc_;
    emit descChanged(desc);
}

const QString& Group::getDesc() const { return desc; }

void Group::setName(const QString& name) {
    qDebug() << __func__ << name;
    Contact::setName(name);
}

void Group::stopAudioOfDepartedPeers(const FriendId& peerPk) {
    if (avGroupchat) {
        //    Core::getInstance()->getAv()->invalidateGroupCallPeerSource(peerPk.toString(),
        //    peerPk);
    }
}
