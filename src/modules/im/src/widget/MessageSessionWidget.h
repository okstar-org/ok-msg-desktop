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

#ifndef OK_MESSAGE_SESSION_WIDGET_H
#define OK_MESSAGE_SESSION_WIDGET_H

#include "ContentWidget.h"
#include "genericchatroomwidget.h"
#include "src/core/toxpk.h"
#include "src/model/chatroom/groupchatroom.h"
#include "src/model/friendmessagedispatcher.h"
#include "src/model/message.h"
#include "src/model/sessionchatlog.h"

#include <memory>

#include <src/worker/SendWorker.h>

class FriendChatroom;
class QPixmap;
class MaskablePixmapWidget;
class CircleWidget;
class ChatForm;
class ChatHistory;
class ContentDialog;
class ContentLayout;
class Widget;
class FriendWidget;

class MessageSessionWidget : public GenericChatroomWidget
{
    Q_OBJECT

  public:
    MessageSessionWidget(ContentLayout* layout,
                         const ToxPk &friendPk,
                         ChatType);

    void contextMenuEvent(QContextMenuEvent* event) override final;
    void setAsActiveChatroom() override final;
    void setAsInactiveChatroom() override final;
    void setAvatar(const QPixmap &pixmap) override final;
    void setStatus(Status::Status status, bool event);
    void setStatusMsg(const QString& msg) ;
    void setTyping(bool typing);
    void setName(const QString& name);
    void resetEventFlags() override final;
    QString getStatusString() const override final;


    const ContactId &getContactId() const   {return contactId; };


    void search(const QString& searchString, bool hide = false);

    void setRecvMessage(const FriendMessage &message,
                        bool isAction);

    void updateStatusLight(Status::Status status, bool event) override final;

signals:
    void removeFriend(const ToxPk& friendPk);
    void addFriend(const ToxPk& friendPk);
    void copyFriendIdToClipboard(const ToxPk& friendPk);
    void contextMenuCalled(QContextMenuEvent* event);
    void friendHistoryRemoved();
    void widgetClicked(MessageSessionWidget* widget);
    void widgetRenamed(MessageSessionWidget* friendWidget);
    void searchCircle(CircleWidget& circleWidget);
    void updateFriendActivity(Friend& frnd);
//    void setActive(bool active);
public slots:
  void onAvatarSet(const ToxPk& friendPk, const QPixmap& pic);
  void onAvatarRemoved(const ToxPk& friendPk);
  void onContextMenuCalled(QContextMenuEvent* event);
  void do_widgetClicked( );
  void showEvent(QShowEvent *) override;

protected:
    virtual void mousePressEvent(QMouseEvent* ev) override;
    virtual void mouseMoveEvent(QMouseEvent* ev) override;
    void setFriendAlias();
    void onSetActive(bool active) override;
  private:
    ContentLayout* contentLayout;
    ContentWidget* contentWidget;

//    MessageProcessor::SharedParams sharedMessageProcessorParams;
//    std::unique_ptr<FriendMessageDispatcher> messageDispatcher;
//    std::unique_ptr<ChatHistory> chatHistory;
//    std::unique_ptr<ChatForm> chatForm;
//    std::unique_ptr<SessionChatLog> chatLog;
//    std::unique_ptr<FriendChatroom> chatRoom;

    const ContactId contactId;

    SendWorker *sendWorker;

    bool isDefaultAvatar;

    ContentDialog *createContentDialog() const;


private slots:
    void removeChatWindow();
    void moveToNewCircle();
    void removeFromCircle();
    void moveToCircle(int circleId);
    void changeAutoAccept(bool enable);
    void showDetails();
};

#endif // FRIENDWIDGET_H
