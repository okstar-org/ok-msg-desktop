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
  regeneratePeerList();
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

void Group::regeneratePeerList() {
  // NOTE: there's a bit of a race here. Core emits a signal for both
  // groupPeerlistChanged and groupPeerNameChanged back-to-back when a peer
  // joins our group. If we get both before we process this slot,
  // core->getGroupPeerNames will contain the new peer name, and we'll ignore
  // the name changed signal, and emit a single userJoined with the correct
  // name. But, if we receive the name changed signal a little later, we will
  // emit userJoined before we have their username, using just their ToxPk, then
  // shortly after emit another peerNameChanged signal. This can cause
  // double-updated to UI and chatlog, but is unavoidable given the API of
  // toxcore.

  //  QStringList peers = groupQuery.getGroupPeerNames(toxGroupNum);
  //  const auto oldPeerNames = peerDisplayNames;
  //  peerDisplayNames.clear();
  //  const int nPeers = peerDisplayNames.size();
  //  for (int i = 0; i < nPeers; ++i) {
  //    const auto pk = groupQuery.getGroupPeerPk(toxGroupNum, peers[i]);
  //    if (pk == idHandler.getSelfPublicKey()) {
  //      peerDisplayNames[pk] = idHandler.getUsername();
  //    } else {
  //      peerDisplayNames[pk] = FriendList::decideNickname(pk, peers[i]);
  //    }
  //  }
  //  for (const auto &pk : oldPeerNames.keys()) {
  //    if (!peerDisplayNames.contains(pk)) {
  //      emit userLeft(pk, oldPeerNames.value(pk));
  //      stopAudioOfDepartedPeers(pk);
  //    }
  //  }
    for (const auto &pk : peerDisplayNames.keys()) {
//      if (!oldPeerNames.contains(pk)) {
        emit userJoined(pk, peerDisplayNames.value(pk));
//      }
    }
  //  for (const auto &pk : peerDisplayNames.keys()) {
  //    if (oldPeerNames.contains(pk) &&
  //        oldPeerNames.value(pk) != peerDisplayNames.value(pk)) {
  //      emit peerNameChanged(pk, oldPeerNames.value(pk),
  //                           peerDisplayNames.value(pk));
  //    }
  //  }
  //  if (oldPeerNames.size() != nPeers) {
  //    emit numPeersChanged(nPeers);
  //  }
}

void Group::updateUsername(ToxPk pk, const QString newName) {
  const QString displayName = FriendList::decideNickname(pk, newName);
//  qDebug() <<"updateUsername=>" << displayName;

  if(!peerDisplayNames.contains(pk)){
        return ;
  }

  if (peerDisplayNames[pk] != displayName) {
    // there could be no actual change even if their username changed due to an
    // alias being set
    const auto oldName = peerDisplayNames[pk];
    peerDisplayNames[pk] = displayName;
    emit peerNameChanged(pk, oldName, displayName);
  }
}

bool Group::isAvGroupchat() const { return avGroupchat; }

QString Group::getId() const { return toxGroupNum; }

const GroupId &Group::getPersistentId() const { return groupId; }

int Group::getPeersCount() const { return peerDisplayNames.size(); }

/**
 * @brief Gets the PKs and names of all peers
 * @return PKs and names of all peers, including our own PK and name
 */
const QMap<ToxPk, QString> &Group::getPeerList() const {
  return peerDisplayNames;
}

void Group::addPeerName(GroupId groupId) {
  peerDisplayNames[ToxPk(groupId)] = groupId.getUsername();
}

void Group::setEventFlag(bool f) { hasNewMessages = f; }

bool Group::getEventFlag() const { return hasNewMessages; }

void Group::setMentionedFlag(bool f) { userWasMentioned = f; }

bool Group::getMentionedFlag() const { return userWasMentioned; }

QString Group::resolveToxId(const ToxPk &id) const {
  auto it = peerDisplayNames.find(id);

  if (it != peerDisplayNames.end()) {
    return *it;
  }

  return QString();
}

void Group::setSelfName(const QString &name) { selfName = name; }

QString Group::getSelfName() const { return selfName; }

void Group::stopAudioOfDepartedPeers(const ToxPk &peerPk) {
  if (avGroupchat) {
    Core *core = Core::getInstance();
    core->getAv()->invalidateGroupCallPeerSource(toxGroupNum, peerPk);
  }
}
