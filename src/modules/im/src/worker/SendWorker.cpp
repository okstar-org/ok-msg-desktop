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

//
// Created by gaojie on 24-5-14.
//

#include "SendWorker.h"

#include <src/model/chatroom/groupchatroom.h>
#include <src/model/friendmessagedispatcher.h>
#include <src/model/groupmessagedispatcher.h>
#include <src/nexus.h>
#include <src/persistence/profile.h>
#include <src/persistence/settings.h>
#include <src/widget/chatformheader.h>
#include <src/widget/contentdialogmanager.h>
#include <src/widget/form/groupchatform.h>

SendWorker::SendWorker(const FriendId& friendId) : contactId{friendId} {
    qDebug() << __func__ << "friend:" << friendId.toString();

    auto core = Core::getInstance();
    auto& settings = Settings::getInstance();
    auto profile = Nexus::getProfile();
    auto history = profile->getHistory();

    initChatHeader(friendId);

    messageDispatcher =
            std::make_unique<FriendMessageDispatcher>(friendId, sharedParams, *core, *core);

    chatHistory = std::make_unique<ChatHistory>(friendId, history, *core, settings,
                                                *messageDispatcher.get());

    chatForm = std::make_unique<ChatForm>(&friendId, *chatHistory.get(), *messageDispatcher.get());

    chatRoom = std::make_unique<FriendChatroom>(&friendId, ContentDialogManager::getInstance());
}

SendWorker::SendWorker(const GroupId& groupId) : contactId{groupId} {
    qDebug() << __func__ << "group:" << groupId.toString();

    auto profile = Nexus::getProfile();
    auto core = Core::getInstance();
    auto& settings = Settings::getInstance();
    auto history = profile->getHistory();

    initChatHeader(groupId);

    messageDispatcher = std::make_unique<GroupMessageDispatcher>(groupId, sharedParams, *core,
                                                                 *core, Settings::getInstance());

    chatLog = std::make_unique<SessionChatLog>(*core);
    connect(messageDispatcher.get(), &IMessageDispatcher::messageSent, chatLog.get(),
            &SessionChatLog::onMessageSent);
    connect(messageDispatcher.get(), &IMessageDispatcher::messageComplete, chatLog.get(),
            &SessionChatLog::onMessageComplete);
    connect(messageDispatcher.get(), &IMessageDispatcher::messageReceived, chatLog.get(),
            &SessionChatLog::onMessageReceived);

    chatForm = std::make_unique<GroupChatForm>(&groupId, *chatLog.get(), *messageDispatcher.get(),
                                               settings);

    chatRoom = std::make_unique<GroupChatroom>(&groupId, ContentDialogManager::getInstance());
}

SendWorker::~SendWorker() { qDebug() << __func__; }

void SendWorker::clearHistory() {
    auto profile = Nexus::getProfile();
    auto history = profile->getHistory();
    history->removeFriendHistory(contactId.toString());
}

std::unique_ptr<SendWorker> SendWorker::forFriend(const FriendId& friend_) {
    return std::make_unique<SendWorker>(friend_);
}

std::unique_ptr<SendWorker> SendWorker::forGroup(const GroupId& group) {
    return std::make_unique<SendWorker>(group);
}

void SendWorker::initChatHeader(const ContactId& contactId) {
    headWidget = std::make_unique<ChatFormHeader>(contactId);

    connect(headWidget.get(), &ChatFormHeader::callAccepted, this,
            [this](const ToxPeer& p) { emit acceptCall(p, lastCallIsVideo); });

    connect(headWidget.get(), &ChatFormHeader::callRejected, this,
            [this](const ToxPeer& p) { emit rejectCall(p); });

    connect(headWidget.get(), &ChatFormHeader::callTriggered, this, &SendWorker::onCallTriggered);

    connect(headWidget.get(), &ChatFormHeader::videoCallTriggered, this,
            &SendWorker::onVideoCallTriggered);
}

void SendWorker::startCounter(bool video) {
    qDebug() << __func__;

    if (!callDuration) {
        callDuration = std::make_unique<CallDurationForm>();
        callDuration->setContact(headWidget->getContact());

        connect(callDuration.get(), &CallDurationForm::endCall, [&]() { emit endCall(); });
        connect(callDuration.get(), &CallDurationForm::muteSpeaker,
                [&](bool mute) { emit muteSpeaker(mute); });
        connect(callDuration.get(), &CallDurationForm::muteMicrophone,
                [&](bool mute) { emit muteMicrophone(mute); });
    }

    callDuration->show();
    if (video) {
        callDuration->showNetcam();
    } else {
        callDuration->showAvatar();
    }
}

void SendWorker::stopCounter(bool error) {
    qDebug() << __func__;
    if (!callDuration) {
        return;
    }
    callDuration.reset();
}
