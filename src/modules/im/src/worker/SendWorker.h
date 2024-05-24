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

#ifndef OKMSG_PROJECT_SENDWORKER_H
#define OKMSG_PROJECT_SENDWORKER_H

#include <QObject>
#include "src/model/message.h"
#include <src/model/chathistory.h>
#include <src/model/chatroom/friendchatroom.h>
#include <src/model/imessagedispatcher.h>
#include <src/widget/form/chatform.h>

class SendWorker : public QObject{
    Q_OBJECT
public:
    SendWorker(const ToxPk& m_friend);
    SendWorker(const GroupId& m_group);
    ~SendWorker();

    void clearHistory();

 static std::unique_ptr<SendWorker> forFriend(const ToxPk& m_friend);
 static std::unique_ptr<SendWorker> forGroup(const GroupId& m_group);


 GenericChatForm* getChatForm() const {
     return chatForm.get();
 };

 Chatroom* getChatroom() const {
     return chatRoom.get();
 }

 IMessageDispatcher* dispacher(){
     return messageDispatcher.get();
 }

 QList<Message> getLastTextMessage(){
    return chatHistory->getLastTextMessage(1);
 }

 IChatLog* getChatLog()const{
     return chatHistory.get();
 }

private:
    MessageProcessor::SharedParams sharedParams;

    std::unique_ptr<IMessageDispatcher> messageDispatcher;
    std::unique_ptr<ChatHistory> chatHistory;
    std::unique_ptr<SessionChatLog> chatLog;
    std::unique_ptr<GenericChatForm> chatForm;
    std::unique_ptr<Chatroom> chatRoom;

    ContactId contactId;
};

#endif // OKMSG_PROJECT_SENDWORKER_H
