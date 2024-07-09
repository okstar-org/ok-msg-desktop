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
#include "src/core/FriendId.h"
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
                         const ContactId &cid,
                         ChatType);

    ~MessageSessionWidget();

    void contextMenuEvent(QContextMenuEvent* event) override final;
    void setAsActiveChatroom() override final;
    void setAsInactiveChatroom() override final;

    void setStatus(Status::Status status, bool event);
    void setStatusMsg(const QString& msg) ;
    void setTyping(bool typing);
    void setName(const QString& name);
    void resetEventFlags() override final;
    QString getStatusString() const override final;


    void search(const QString& searchString, bool hide = false);

    void setRecvMessage(const FriendMessage &message,
                        bool isAction);

    void setMessageReceipt(const ReceiptNum &receipt);


    void setRecvGroupMessage(const GroupMessage& msg);

    void setFileReceived(const ToxFile& file);
    void setFileCancelled(const QString &fileId);

    void clearHistory();

    void setFriend(const Friend* f);
    void removeFriend();

    void setAvInvite(const ToxPeer &peerId, bool video);
    void setAvStart(const FriendId& friendId, bool video);
    void setAvEnd(const FriendId& friendId, bool error);

    void setGroup(const Group* g);
    void removeGroup();

    void clearReceipts();

signals:
    void removeFriend(const FriendId& friendPk);
    void copyFriendIdToClipboard(const FriendId& friendPk);
    void contextMenuCalled(QContextMenuEvent* event);
    void friendHistoryRemoved();
    void widgetClicked(MessageSessionWidget* widget);
    void widgetRenamed(MessageSessionWidget* widget);
    void searchCircle(CircleWidget& circleWidget);
    void updateFriendActivity(Friend& frnd);
//    void setActive(bool active);
    void deleteSession(const QString &contactId);

public slots:
  void onAvatarSet(const FriendId& friendPk, const QPixmap& pic);
  void onAvatarRemoved(const FriendId& friendPk);
  void onContextMenuCalled(QContextMenuEvent* event);
  void do_widgetClicked( );
  void showEvent(QShowEvent *) override;

private slots:
    void removeChat();
    void moveToNewCircle();
    void removeFromCircle();
    void moveToCircle(int circleId);
    void changeAutoAccept(bool enable);
    void showDetails();
    void onMessageSent(DispatchedMessageId id, const Message & message);
    void doAcceptCall(const ToxPeer& p, bool video);
    void doRejectCall(const ToxPeer& p);
    void doCall();
    void doVideoCall();
    void endCall();
    void doMuteMicrophone(bool mute);
    void doSilenceSpeaker(bool mute);

protected:
    virtual void mousePressEvent(QMouseEvent* ev) override;
    virtual void mouseMoveEvent(QMouseEvent* ev) override;
    void setFriendAlias();
    void onActiveSet(bool active) override;
    void paintEvent(QPaintEvent *e) override;

private:

    ContentLayout* contentLayout;

    std::unique_ptr<ContentWidget> contentWidget;
    std::unique_ptr<SendWorker> sendWorker;

    //联系人ID(朋友ID、群聊ID共享)
    ContactId contactId;
    FriendId friendId;
    GroupId groupId;

};

#endif // FRIENDWIDGET_H
