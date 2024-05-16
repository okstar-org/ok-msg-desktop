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

#ifndef FRIENDWIDGET_H
#define FRIENDWIDGET_H

#include "ContentWidget.h"
#include "genericchatroomwidget.h"
#include "src/core/toxpk.h"
#include "src/model/chatroom/groupchatroom.h"
#include "src/model/friendmessagedispatcher.h"
#include "src/model/message.h"
#include "src/model/sessionchatlog.h"

#include <memory>

class FriendChatroom;
class QPixmap;
class MaskablePixmapWidget;
class CircleWidget;
class ChatForm;
class ChatHistory;
class ContentDialog;
class ContentLayout;
class Widget;
class AboutFriendForm;

class FriendWidget : public GenericChatroomWidget
{
    Q_OBJECT

  public:
    FriendWidget(ContentLayout* layout, const ToxPk &friendPk, bool isFriend, bool compact);

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

    const Friend* getFriend() const ;
    const Contact* getContact() const ;

    void search(const QString& searchString, bool hide = false);
    void setRecvMessage(const FriendMessage &message,
                        bool isAction);
    virtual void updateStatusLight(Status::Status status, bool event) override;

signals:
    void friendWidgetClicked(FriendWidget* widget);
    void removeFriend(const ToxPk& friendPk);
    void addFriend(const ToxPk& friendPk);
    void copyFriendIdToClipboard(const ToxPk& friendPk);
    void contextMenuCalled(QContextMenuEvent* event);
    void friendHistoryRemoved();
    void friendWidgetRenamed(FriendWidget* friendWidget);
    void searchCircle(CircleWidget& circleWidget);
    void updateFriendActivity(const Friend& frnd);
//    void setActive(bool active);
public slots:
  void onAvatarSet(const ToxPk& friendPk, const QPixmap& pic);
  void onAvatarRemoved(const ToxPk& friendPk);
  void onContextMenuCalled(QContextMenuEvent* event);
  void do_widgetClicked(GenericChatroomWidget *w);
  void showDetails();

protected:
    virtual void mousePressEvent(QMouseEvent* ev) override;
    virtual void mouseMoveEvent(QMouseEvent* ev) override;

    void onSetActive(bool active) override;
  private:
    ContentLayout* contentLayout;
//    ContentWidget* contentWidget;

    std::unique_ptr<AboutFriendForm> about;

    MessageProcessor::SharedParams sharedMessageProcessorParams;
    std::unique_ptr<FriendMessageDispatcher> messageDispatcher;
    std::unique_ptr<ChatHistory> chatHistory;
    std::unique_ptr<ChatForm> chatForm;
    std::unique_ptr<FriendChatroom> chatRoom;
    std::unique_ptr<SessionChatLog> chatLog;
    Friend *m_friend;


    bool isDefaultAvatar;

    ContentDialog *createContentDialog() const;
    ContentDialog * addFriendDialog(const Friend *frnd );

private slots:
    void removeChatWindow();
    void moveToNewCircle();
    void removeFromCircle();
    void moveToCircle(int circleId);
    void changeAutoAccept(bool enable);

};

#endif // FRIENDWIDGET_H
