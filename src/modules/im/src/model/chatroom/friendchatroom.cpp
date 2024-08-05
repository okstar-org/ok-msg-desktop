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

#include "src/model/chatroom/friendchatroom.h"
#include "src/grouplist.h"
#include "src/model/dialogs/idialogsmanager.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include "src/widget/contentdialog.h"

#include <QCollator>

namespace {

QString getShortName(const QString& name) {
    constexpr auto MAX_NAME_LENGTH = 30;
    if (name.length() <= MAX_NAME_LENGTH) {
        return name;
    }

    return name.left(MAX_NAME_LENGTH).trimmed() + ("...");
}

}  // namespace

FriendChatroom::FriendChatroom(const FriendId* frnd, IDialogsManager* dialogsManager)
        : frnd{frnd}, dialogsManager{dialogsManager} {
    qDebug() << __func__ << "friend" << frnd->getId();
}

FriendChatroom::~FriendChatroom() { qDebug() << __func__; }

const FriendId* FriendChatroom::getFriend() { return frnd; }

const ContactId& FriendChatroom::getContactId() { return *frnd; }

bool FriendChatroom::canBeInvited() const {
    return false;
    //    return Status::isOnline(frnd->getStatus());
}

int FriendChatroom::getCircleId() const { return 0; }

QString FriendChatroom::getCircleName() const {
    const auto circleId = getCircleId();
    return Settings::getInstance().getCircleName(circleId);
}

QString FriendChatroom::getAutoAcceptDir() const {
    return Settings::getInstance().getAutoAcceptDir(*frnd);
}

void FriendChatroom::setAutoAcceptDir(const QString& dir) {
    Settings::getInstance().setAutoAcceptDir(*frnd, dir);
}

void FriendChatroom::disableAutoAccept() { setAutoAcceptDir(QString{}); }

bool FriendChatroom::autoAcceptEnabled() const { return getAutoAcceptDir().isEmpty(); }

void FriendChatroom::inviteFriend(const Group* group) {
    const auto friendId = frnd->getId();
    const auto groupId = group->getId();
    //    Core::getInstance()->groupInviteFriend(friendId, groupId);
}

QVector<GroupToDisplay> FriendChatroom::getGroups() const {
    QVector<GroupToDisplay> groups;
    for (const auto group : GroupList::getAllGroups()) {
        const auto name = getShortName(group->getName());
        const GroupToDisplay groupToDisplay = {name, group};
        groups.push_back(groupToDisplay);
    }

    return groups;
}

/**
 * @brief Return sorted list of circles exclude current circle.
 */
QVector<CircleToDisplay> FriendChatroom::getOtherCircles() const {
    QVector<CircleToDisplay> circles;
    const auto currentCircleId = getCircleId();
    const auto& s = Settings::getInstance();
    for (int i = 0; i < s.getCircleCount(); ++i) {
        if (i == currentCircleId) {
            continue;
        }

        const auto name = getShortName(s.getCircleName(i));
        const CircleToDisplay circle = {name, i};
        circles.push_back(circle);
    }

    std::sort(circles.begin(), circles.end(),
              [](const CircleToDisplay& a, const CircleToDisplay& b) -> bool {
                  QCollator collator;
                  collator.setNumericMode(true);
                  return collator.compare(a.name, b.name) < 0;
              });

    return circles;
}

void FriendChatroom::resetEventFlags() {
    //    frnd->setEventFlag(false);
}

bool FriendChatroom::possibleToOpenInNewWindow() const {
    //    const auto friendPk = frnd->getId();
    const auto dialogs = dialogsManager->getFriendDialogs(*frnd);
    return !dialogs || dialogs->chatroomCount() > 1;
}

bool FriendChatroom::canBeRemovedFromWindow() const {
    const auto friendPk = frnd;
    const auto dialogs = dialogsManager->getFriendDialogs(*friendPk);
    return dialogs && dialogs->hasContact(ContactId(frnd->toString()));
}

bool FriendChatroom::friendCanBeRemoved() const {
    const auto dialogs = dialogsManager->getFriendDialogs(*frnd);
    return !dialogs || !dialogs->hasContact(ContactId(frnd->toString()));
}

void FriendChatroom::removeFriendFromDialogs() {
    auto dialogs = dialogsManager->getFriendDialogs(*frnd);
    dialogs->removeFriend(*frnd);
}
