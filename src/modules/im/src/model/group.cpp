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
#include "friend.h"
#include "src/core/contactid.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/groupid.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"
#include "src/persistence/settings.h"
#include "src/widget/form/groupchatform.h"
#include "src/widget/groupwidget.h"
#include <QDebug>

static const int MAX_GROUP_TITLE_LENGTH = 128;

Group::Group(QString groupId, const GroupId persistentGroupId,
             const QString &name, bool isAvGroupchat, const QString &selfName,
             ICoreGroupQuery &groupQuery, ICoreIdHandler &idHandler)
    : selfName{selfName}, title{name},
      toxGroupNum(groupId), groupId{persistentGroupId},
      avGroupchat{isAvGroupchat}, groupQuery(groupQuery), idHandler(idHandler) {
  // in groupchats, we only notify on messages containing your name <-- dumb
  // sound notifications should be on all messages, but system popup
  // notification on naming is appropriate
  hasNewMessages = 0;
  userWasMentioned = 0;

}

void Group::setName(const QString &newTitle) {
  const QString shortTitle = newTitle.left(MAX_GROUP_TITLE_LENGTH);
  if (!shortTitle.isEmpty() && title != shortTitle) {
    title = shortTitle;
    emit displayedNameChanged(title);
    emit titleChangedByUser(title);
    emit titleChanged(selfName, title);
  }
}

void Group::setTitle(const QString &author, const QString &newTitle) {
  const QString shortTitle = newTitle.left(MAX_GROUP_TITLE_LENGTH);
  if (!shortTitle.isEmpty() && title != shortTitle) {
    title = shortTitle;
    emit displayedNameChanged(title);
    emit titleChanged(author, title);
  }
}

QString Group::getName() const { return title; }

QString Group::getDisplayedName() const { return getName(); }


void Group::updateUsername(const QString oldName, const QString newName) {
//  const QString displayName = FriendList::decideNickname(pk, newName);
//  qDebug() <<"updateUsername=>" << displayName;

  if(!peerDisplayNames.contains(oldName)){
     return;
  }

  if (peerDisplayNames[oldName] != newName) {
    peerDisplayNames[newName] = newName;
    emit peerNameChanged(oldName, newName);
  }
}

bool Group::isAvGroupchat() const { return avGroupchat; }

QString Group::getId() const { return toxGroupNum; }

const GroupId &Group::getPersistentId() const { return groupId; }

int Group::getPeersCount() const { return peerCount; }

void Group::setPeerCount(uint32_t count)
{
    peerCount = count;
    emit peerCountChanged(peerCount);
}

/**
 * @brief Gets the PKs and names of all peers
 * @return PKs and names of all peers, including our own PK and name
 */
const QMap<QString, QString> &Group::getPeerList() const {
  return peerDisplayNames;
}

void Group::addPeer(const GroupOccupant &occ) {
  peerDisplayNames[occ.nick] = occ.nick;
}

void Group::setEventFlag(bool f) { hasNewMessages = f; }

bool Group::getEventFlag() const { return hasNewMessages; }

void Group::setMentionedFlag(bool f) { userWasMentioned = f; }

bool Group::getMentionedFlag() const { return userWasMentioned; }



void Group::setSelfName(const QString &name) { selfName = name; }

QString Group::getSelfName() const { return selfName; }

void Group::setDesc(const QString &desc_)
{
    desc = desc_;
    emit descChanged(desc);
}

const QString& Group::getDesc() const
{
    return desc;
}

void Group::stopAudioOfDepartedPeers(const ToxPk &peerPk) {
  if (avGroupchat) {
    Core *core = Core::getInstance();
    core->getAv()->invalidateGroupCallPeerSource(toxGroupNum, peerPk);
  }
}
