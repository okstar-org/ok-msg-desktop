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

#include <src/model/friendmessagedispatcher.h>
#include <src/model/groupmessagedispatcher.h>
#include <src/nexus.h>
#include <src/persistence/settings.h>
#include <src/persistence/profile.h>
#include <src/widget/form/groupchatform.h>
#include <src/widget/contentdialogmanager.h>
#include <src/model/chatroom/groupchatroom.h>



SendWorker::SendWorker(const Friend &m_friend){
    qDebug() << __func__ <<"friend:"<<m_friend.getId();

    auto core = Core::getInstance();
    auto &settings = Settings::getInstance();
    auto profile = Nexus::getProfile();
    auto history = profile->getHistory();


       messageDispatcher = std::make_unique<FriendMessageDispatcher>(
           m_friend, sharedParams, *core, *core);

       chatHistory = std::make_unique<ChatHistory>(m_friend,
                                                   history,
                                                   *core,
                                                   settings,
                                                   *messageDispatcher.get());

//       chatLog = std::make_unique<SessionChatLog>(*core);



      chatForm = std::make_unique<ChatForm>(&m_friend,
                                            *chatHistory.get(),
                                            *messageDispatcher.get());


      chatRoom = std::make_unique<FriendChatroom>(&m_friend,
                                                  ContentDialogManager::getInstance());

}

SendWorker::SendWorker(const Group &group)
{
    qDebug() << __func__ <<"group:"<<group.getId();
    auto profile = Nexus::getProfile();
    auto core = Core::getInstance();
    auto &settings = Settings::getInstance();
    auto history = profile->getHistory();

    messageDispatcher = std::make_unique<GroupMessageDispatcher>(
        group, sharedParams, *core, *core,
        Settings::getInstance());

     chatLog = std::make_unique<SessionChatLog>(*core);
     connect(messageDispatcher.get(), &IMessageDispatcher::messageSent,
             chatLog.get(), &SessionChatLog::onMessageSent);
     connect(messageDispatcher.get(), &IMessageDispatcher::messageComplete,
             chatLog.get(), &SessionChatLog::onMessageComplete);
     connect(messageDispatcher.get(), &IMessageDispatcher::messageReceived,
             chatLog.get(), &SessionChatLog::onMessageReceived);


     chatForm = std::make_unique<GroupChatForm>(&group,
                                                *chatLog.get(),
                                                *messageDispatcher.get(),
                                                settings);

     chatRoom = std::make_unique<GroupChatroom>(&group,
                                                ContentDialogManager::getInstance());
}

SendWorker::~SendWorker()
{
    qDebug()<<__func__;
}

std::unique_ptr<SendWorker> SendWorker::forFriend(const Friend& friend_){
   return std::make_unique<SendWorker>(friend_);
}

std::unique_ptr<SendWorker> SendWorker::forGroup(const Group &group)
{
    return std::make_unique<SendWorker>(group);
}
