/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

#ifndef MESSAGE_SESSION_LIST_WIDGET_H
#define MESSAGE_SESSION_LIST_WIDGET_H


#include "MessageSessionWidget.h"
#include "genericchatitemlayout.h"
#include "src/core/core.h"
#include "src/model/friendmessagedispatcher.h"
#include "src/model/message.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include <QWidget>

class QVBoxLayout;
class QGridLayout;
class QPixmap;
class Widget;
class FriendWidget;
class GroupWidget;
//class CircleWidget;
//class CategoryWidget;
class FriendListLayout;
class GenericChatroomWidget;
class Friend;
class ContentLayout;
class MainLayout;

class MessageSessionListWidget : public QWidget {
  Q_OBJECT
public:

  using SortingMode = Settings::FriendListSortingMode;
  explicit MessageSessionListWidget(MainLayout *parent, bool groupsOnTop = true);
  ~MessageSessionListWidget();
  void setMode(SortingMode mode);
  SortingMode getMode() const;
  void reloadTheme();

  MessageSessionWidget *createMessageSession(const ToxPk &friendPk,const QString &sid,
                                             ChatType type);
  MessageSessionWidget *getMessageSession(const ToxPk &friendPk);

  void removeSessionWidget(MessageSessionWidget *w);
  void removeFriend(const ToxPk &friendPk);

  void setFriendStatus(const ToxPk &friendPk,  Status::Status status);
  void setFriendStatusMsg(const ToxPk &friendPk,  const QString& statusMsg);
  void setFriendName(const ToxPk &friendPk,  const QString& name);
  void setFriendAvatar(const ToxPk &friendPk, const QByteArray& avatar);
  void setFriendTyping(const ToxPk &pk, bool typing);

  void searchChatrooms(const QString &searchString, bool hideOnline = false,
                       bool hideOffline = false, bool hideGroups = false);

  void cycleContacts(GenericChatroomWidget *activeChatroomWidget, bool forward);

  void updateActivityTime(const QDateTime &date);
  void reDraw();

//  void setRecvGroupMessage(const GroupMessage& msg);

  void setRecvFriendMessage(ToxPk friendnumber,                       //
                            const FriendMessage &message, //
                            bool isAction);

//  CircleWidget *createCircleWidget(int id = -1);

  void toSendMessage(const ToxPk &pk, bool isGroup);


signals:
  void onCompactChanged(bool compact);
//  void connectCircleWidget(CircleWidget &circleWidget);
//  void searchCircle(CircleWidget &circleWidget);
public slots:
//  void renameGroupWidget(GroupWidget *groupWidget, const QString &newName);

//  void renameCircleWidget(CircleWidget *circleWidget, const QString &newName);
  void onFriendWidgetRenamed(FriendWidget *friendWidget);

  void slot_sessionClicked(GenericChatroomWidget *);

  void moveWidget(MessageSessionWidget *w, Status::Status s, bool add = false);


  void dayTimeout();

protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void showEvent(QShowEvent *event) override;


private:
  QLayout *nextLayout(QLayout *layout, bool forward) const;
  void moveFriends(QLayout *layout);
//  CategoryWidget *getTimeCategoryWidget(const Friend *frd) const;
  void sortByMode(SortingMode mode);
  void connectSessionWidget(MessageSessionWidget &sw);
  void updateFriendActivity(const Friend &frnd);
  void addWidget(MessageSessionWidget  *fw, Status::Status s, int circleIndex);

  SortingMode mode;

  bool groupsOnTop;
  FriendListLayout *listLayout;
//  GenericChatItemLayout *circleLayout = nullptr;
  QVBoxLayout *activityLayout = nullptr;
  QTimer *dayTimer;

  ContentLayout *m_contentLayout;

  QMap<QString, MessageSessionWidget *> sessionWidgets;

};

#endif // FRIENDLISTWIDGET_H
