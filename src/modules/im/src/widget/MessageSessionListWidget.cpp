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

#include "MessageSessionListWidget.h"
#include "ChatWidget.h"
#include "base/OkSettings.h"
#include "base/times.h"
#include "circlewidget.h"
#include "contentdialogmanager.h"
#include "friendlistlayout.h"
#include "friendwidget.h"
#include "groupwidget.h"
#include "src/friendlist.h"
#include "src/model/chathistory.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
//#include "src/widget/categorywidget.h"
#include "widget.h"
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QGridLayout>
#include <QMimeData>
#include <QTimer>
#include <cassert>

enum class Time {
  Today,
  Yesterday,
  ThisWeek,
  ThisMonth,
  Month1Ago,
  Month2Ago,
  Month3Ago,
  Month4Ago,
  Month5Ago,
  LongAgo,
  Never
};

static const int LAST_TIME = static_cast<int>(Time::Never);

inline QDateTime getActiveTimeFriend(const Friend *contact) {
  return Settings::getInstance().getFriendActivity(contact->getPublicKey());
}

MessageSessionListWidget::MessageSessionListWidget(MainLayout *parent,
                                                   bool groupsOnTop)
    : QWidget(parent), groupsOnTop(groupsOnTop) {

  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

  m_contentLayout = parent->getContentLayout();

  // Prevent QLayout's add child warning before setting the mode.
  listLayout = new FriendListLayout();
  listLayout->removeItem(listLayout->getLayoutOnline());
  listLayout->removeItem(listLayout->getLayoutOffline());
  setLayout(listLayout);

  mode = Settings::getInstance().getFriendSortingMode();
  sortByMode(mode);


  dayTimer = new QTimer(this);
  dayTimer->setTimerType(Qt::VeryCoarseTimer);
  connect(dayTimer, &QTimer::timeout, this, &MessageSessionListWidget::dayTimeout);
  dayTimer->start(base::Times::timeUntilTomorrow());

  setAcceptDrops(true);

  auto &settings = Settings::getInstance();
  connect(&settings, &Settings::compactLayoutChanged,
          this, &MessageSessionListWidget::onCompactChanged);

//  connect(&settings, &Settings::groupchatPositionChanged, this,
//          &MessageSessionListWidget::onGroupchatPositionChanged);
}

MessageSessionListWidget::~MessageSessionListWidget() {
  if (activityLayout != nullptr) {
    QLayoutItem *item;
    while ((item = activityLayout->takeAt(0)) != nullptr) {
      delete item->widget();
      delete item;
    }
    delete activityLayout;
  }

  //  if (circleLayout != nullptr) {
  //    QLayoutItem *item;
  //    while ((item = circleLayout->getLayout()->takeAt(0)) != nullptr) {
  //      delete item->widget();
  //      delete item;
  //    }
  //    delete circleLayout;
  //  }
}


MessageSessionWidget *MessageSessionListWidget::createMessageSession(
        const ToxPk &friendPk, const QString &sid, ChatType type) {
  qDebug() << __func__ << "friend:" << friendPk.toString();

  auto core = Core::getInstance();
  auto profile = Nexus::getProfile();

  auto &settings = Settings::getInstance();


  auto sw = getMessageSession(friendPk);
  if(!sw){
      sw = new MessageSessionWidget(m_contentLayout, friendPk, type);
      qDebug() << "create friend:" << friendPk.toString() <<" session:" <<sw;
      connectSessionWidget(*sw);
    }

  //  TODO 连接朋友活跃状态
  //  connect(chatForm, &ChatForm::updateFriendActivity, this,
  //          &Widget::updateFriendActivity);

  //  friendMessageDispatchers[friendPk] = messageDispatcher;
  //  friendChatLogs[friendPk] = chatHistory;
  //  friendChatrooms[friendPk] = chatRoom;
  //  friendWidgets[friendPk] = friendWidget;
  //  chatForms[friendPk] = chatForm;

  //  const auto activityTime = settings.getFriendActivity(friendPk);
  //  const auto chatTime = chatForm->getLatestTime();
  //  if (chatTime > activityTime && chatTime.isValid()) {
  //    settings.setFriendActivity(friendPk, chatTime);
  //  }
  if(type==ChatType::GroupChat){
      addWidget(sw, Status::Status::Online, 0);
  }else{
    auto status = core->getFriendStatus(friendPk.toString());
    addWidget(sw, status, settings.getFriendCircleID(friendPk));
//    setFriendStatus(friendPk, status);
  }

  //
  //  auto notifyReceivedCallback = [this, friendPk](const ToxPk &author,
  //                                                 const Message &message) {
  //    auto isTargeted =
  //        std::any_of(message.metadata.begin(), message.metadata.end(),
  //                    [](MessageMetadata metadata) {
  //                      return metadata.type ==
  //                      MessageMetadataType::selfMention;
  //                    });
  //    newFriendMessageAlert(friendPk, message.content);
  //  };
  //
  //  auto notifyReceivedConnection =
  //      connect(messageDispatcher.get(),
  //              &IMessageDispatcher::messageReceived, notifyReceivedCallback);
  //
  //  friendAlertConnections.insert(friendPk, notifyReceivedConnection);
  //
  //  connect(newfriend, &Friend::aliasChanged, this,
  //          &Widget::onFriendAliasChanged);
  //  connect(newfriend, &Friend::displayedNameChanged, this,
  //          &Widget::onFriendDisplayedNameChanged);
  //
  //  connect(chatForm, &ChatForm::incomingNotification, this,
  //          &Widget::incomingNotification);
  //  connect(chatForm, &ChatForm::outgoingNotification, this,
  //          &Widget::outgoingNotification);
  //  connect(chatForm, &ChatForm::stopNotification, this,
  //          &Widget::onStopNotification);
  //  connect(chatForm, &ChatForm::endCallNotification, this,
  //  &Widget::onCallEnd); connect(chatForm, &ChatForm::rejectCall, this,
  //  &Widget::onRejectCall);
  //
  //  connect(friendWidget, &FriendWidget::newWindowOpened, this,
  //  &Widget::openNewDialog); connect(friendWidget,
  //  &FriendWidget::chatroomWidgetClicked, this,
  //          &Widget::onChatroomWidgetClicked);
  //  connect(friendWidget, &FriendWidget::chatroomWidgetClicked, chatForm,
  //          &ChatForm::focusInput);
  //  connect(friendWidget, &FriendWidget::friendHistoryRemoved, chatForm,
  //          &ChatForm::clearChatArea);
  //  connect(friendWidget, &FriendWidget::copyFriendIdToClipboard, this,
  //          &Widget::copyFriendIdToClipboard);
  //
  //  connect(friendWidget, &FriendWidget::contextMenuCalled, friendWidget,
  //          &FriendWidget::onContextMenuCalled);
  //
  //  connect(friendWidget, &FriendWidget::addFriend, //
  //          this, &Widget::addFriend0);
  //
  //  connect(friendWidget, SIGNAL(removeFriend(const ToxPk &)), this,
  //          SLOT(removeFriend(const ToxPk &)));
  //
  //  Profile *profile = Nexus::getProfile();
  //  connect(profile, &Profile::friendAvatarSet, friendWidget,
  //          &FriendWidget::onAvatarSet);
  //  connect(profile, &Profile::friendAvatarRemoved, friendWidget,
  //          &FriendWidget::onAvatarRemoved);
  //
  //  // Try to get the avatar from the cache
  //  QPixmap avatar = Nexus::getProfile()->loadAvatar(friendPk);
  //  if (!avatar.isNull()) {
  //    //chatForm->onAvatarChanged(friendPk, avatar);
  //    //friendWidget->onAvatarSet(friendPk, avatar);
  //  }
  //
//    FilterCriteria filter = getFilterCriteria();
//    friendWidget->search(ui->searchContactText->text(),
//    filterOffline(filter));

  //  core->getFriendInfo(friendPk.toString());

  return sw;
}

void MessageSessionListWidget::addWidget(MessageSessionWidget *w,
                                         Status::Status s,
                                         int circleIndex) {


    listLayout->addWidget(w);
    sessionWidgets.insert(w->getContactId().toString(), w);



//  connect(fw, &MessageSessionWidget::friendWidgetRenamed, this,
//          &MessageSessionListWidget::onFriendWidgetRenamed);
//  connect(fw, &MessageSessionWidget::friendWidgetClicked, this,
//          &MessageSessionListWidget::slot_friendClicked);
}


void MessageSessionListWidget::connectSessionWidget(MessageSessionWidget &sw) {
//  connect(&sw, &MessageSessionWidget::searchCircle, this,
//          &MessageSessionListWidget::searchCircle);
  connect(&sw, &MessageSessionWidget::updateFriendActivity, this,
          &MessageSessionListWidget::updateFriendActivity);
  connect(&sw, &MessageSessionWidget::widgetClicked, this,
          &MessageSessionListWidget::slot_sessionClicked);
}

void MessageSessionListWidget::updateFriendActivity(const Friend &frnd) {
  const ToxPk &pk = frnd.getPublicKey();
  auto &settings = Settings::getInstance();
  const auto oldTime = settings.getFriendActivity(pk);
  const auto newTime = QDateTime::currentDateTime();
  settings.setFriendActivity(pk, newTime);
  MessageSessionWidget *widget = getMessageSession(frnd.getPublicKey());
  moveWidget(widget, frnd.getStatus());
  updateActivityTime(oldTime); // update old category widget
}

void MessageSessionListWidget::setMode(SortingMode mode) {
  if (this->mode == mode)
    return;

  this->mode = mode;
  Settings::getInstance().setFriendSortingMode(mode);

  sortByMode(mode);
}

void MessageSessionListWidget::sortByMode(SortingMode mode) {
  if (mode == SortingMode::Name) {
    //    circleLayout = new GenericChatItemLayout;
    //    circleLayout->getLayout()->setSpacing(0);
    //    circleLayout->getLayout()->setMargin(0);

    //    for (int i = 0; i < Settings::getInstance().getCircleCount(); ++i) {
    //      addCircleWidget(i);
    //      CircleWidget::getFromID(i)->setVisible(false);
    //    }

    // Only display circles once all created to avoid artifacts.
    //    for (int i = 0; i < Settings::getInstance().getCircleCount(); ++i)
    //      CircleWidget::getFromID(i)->setVisible(true);

    int count = activityLayout ? activityLayout->count() : 0;
    for (int i = 0; i < count; i++) {
      QWidget *widget = activityLayout->itemAt(i)->widget();
//      CategoryWidget *categoryWidget = qobject_cast<CategoryWidget *>(widget);
//      if (categoryWidget) {
//        categoryWidget->moveFriendWidgets(this);
//      } else {
//        qWarning() << "Unexpected widget";
//      }
    }

//    listLayout->addLayout(listLayout->getLayout());
    listLayout->addLayout(listLayout->getLayoutOffline());
    //    listLayout->addLayout(circleLayout->getLayout());
//    onGroupchatPositionChanged(groupsOnTop);

    if (activityLayout != nullptr) {
      QLayoutItem *item;
      while ((item = activityLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
      }
      delete activityLayout;
      activityLayout = nullptr;
    }

    reDraw();
  } else if (mode == SortingMode::Activity) {
    QLocale ql(Settings::getInstance().getTranslation());
    QDate today = QDate::currentDate();
#define COMMENT "Category for sorting friends by activity"
    // clang-format off
        const QMap<Time, QString> names {
            { Time::Today,     tr("Today",                      COMMENT) },
            { Time::Yesterday, tr("Yesterday",                  COMMENT) },
            { Time::ThisWeek,  tr("Last 7 days",                COMMENT) },
            { Time::ThisMonth, tr("This month",                 COMMENT) },
            { Time::LongAgo,   tr("Older than 6 Months",        COMMENT) },
            { Time::Never,     tr("Never",                      COMMENT) },
            { Time::Month1Ago, ql.monthName(today.addMonths(-1).month()) },
            { Time::Month2Ago, ql.monthName(today.addMonths(-2).month()) },
            { Time::Month3Ago, ql.monthName(today.addMonths(-3).month()) },
            { Time::Month4Ago, ql.monthName(today.addMonths(-4).month()) },
            { Time::Month5Ago, ql.monthName(today.addMonths(-5).month()) },
        };
// clang-format on
#undef COMMENT

    activityLayout = new QVBoxLayout();
//    bool compact = Settings::getInstance().getCompactLayout();
//    for (Time t : names.keys()) {
//      CategoryWidget *category = new CategoryWidget(compact, this);
//      category->setName(names[t]);
//      activityLayout->addWidget(category);
//    }

    moveFriends(listLayout->getLayoutOffline());
    moveFriends(listLayout->getLayoutOnline());
    //    if (circleLayout != nullptr) {
    //      moveFriends(circleLayout->getLayout());
    //    }

    for (int i = 0; i < activityLayout->count(); ++i) {
      QWidget *widget = activityLayout->itemAt(i)->widget();
//      CategoryWidget *categoryWidget = qobject_cast<CategoryWidget *>(widget);
//      categoryWidget->setVisible(categoryWidget->hasChatrooms());
    }

    listLayout->removeItem(listLayout->getLayoutOnline());
    listLayout->removeItem(listLayout->getLayoutOffline());

    //    if (circleLayout != nullptr) {
    //      listLayout->removeItem(circleLayout->getLayout());
    //
    //      QLayoutItem *item;
    //      while ((item = circleLayout->getLayout()->takeAt(0)) != nullptr) {
    //        delete item->widget();
    //        delete item;
    //      }
    //      delete circleLayout;
    //      circleLayout = nullptr;
    //    }

    listLayout->insertLayout(1, activityLayout);

    reDraw();
  }
}

void MessageSessionListWidget::moveFriends(QLayout *layout) {
  while (!layout->isEmpty()) {
    QWidget *widget = layout->itemAt(0)->widget();
    FriendWidget *friendWidget = qobject_cast<FriendWidget *>(widget);
//    CircleWidget *circleWidget = qobject_cast<CircleWidget *>(widget);
//    if (circleWidget) {
//      circleWidget->moveFriendWidgets(this);
//    } else if (friendWidget) {
//      const Friend *contact = friendWidget->getFriend();
//      auto *categoryWidget = getTimeCategoryWidget(contact);
//      categoryWidget->addFriendWidget(friendWidget, contact->getStatus());
//    }
  }
}

//CategoryWidget *
//MessageSessionListWidget::getTimeCategoryWidget(const Friend *frd) const {
//  const auto activityTime = getActiveTimeFriend(frd);
//  int timeIndex = static_cast<int>(base::getTimeBucket(activityTime));
//  QWidget *widget = activityLayout->itemAt(timeIndex)->widget();
//  return qobject_cast<CategoryWidget *>(widget);
//}

MessageSessionListWidget::SortingMode MessageSessionListWidget::getMode() const { return mode; }

void MessageSessionListWidget::removeSessionWidget(MessageSessionWidget *w) {
    qDebug() << __func__<<"widget:"<<w;
//  const Friend *contact = w->getFriend();

//  if (mode == SortingMode::Activity) {
//    auto *categoryWidget = getTimeCategoryWidget(contact);
//    categoryWidget->removeFriendWidget(w, contact->getStatus());
//    categoryWidget->setVisible(categoryWidget->hasChatrooms());
//  } else {
//    int id = Settings::getInstance().getFriendCircleID(contact->getPublicKey());
//    CircleWidget *circleWidget = CircleWidget::getFromID(id);
//    if (circleWidget != nullptr) {
//      circleWidget->removeFriendWidget(w, contact->getStatus());
//      emit searchCircle(*circleWidget);
//    }
//  }
}

void MessageSessionListWidget::searchChatrooms(const QString &searchString,
                                       bool hideOnline, bool hideOffline,
                                       bool hideGroups) {

  listLayout->searchChatrooms(searchString, hideOnline, hideOffline);

}

void MessageSessionListWidget::onFriendWidgetRenamed(FriendWidget *friendWidget) {
  const Friend *contact = friendWidget->getFriend();
  auto status = contact->getStatus();
//  if (mode == SortingMode::Activity) {
//    auto *categoryWidget = getTimeCategoryWidget(contact);
//    categoryWidget->removeFriendWidget(friendWidget, status);
//    categoryWidget->addFriendWidget(friendWidget, status);
//  } else {
//    int id = Settings::getInstance().getFriendCircleID(contact->getPublicKey());
//    CircleWidget *circleWidget = CircleWidget::getFromID(id);
//    if (circleWidget != nullptr) {
//      circleWidget->removeFriendWidget(friendWidget, status);
//      circleWidget->addFriendWidget(friendWidget, status);
//      emit searchCircle(*circleWidget);
//    } else {
      listLayout->removeFriendWidget(friendWidget, status);
      listLayout->addFriendWidget(friendWidget, status);
//    }
//  }
}



void MessageSessionListWidget::cycleContacts(
    GenericChatroomWidget *activeChatroomWidget, bool forward) {
  if (!activeChatroomWidget) {
    return;
  }

  int index = -1;
  FriendWidget *friendWidget =
      qobject_cast<FriendWidget *>(activeChatroomWidget);

  if (mode == SortingMode::Activity) {
    if (!friendWidget) {
      return;
    }

    const auto activityTime = getActiveTimeFriend(friendWidget->getFriend());
    index = static_cast<int>(base::getTimeBucket(activityTime));
    QWidget *widget = activityLayout->itemAt(index)->widget();
//    CategoryWidget *categoryWidget = qobject_cast<CategoryWidget *>(widget);
//    if (categoryWidget == nullptr ||
//        categoryWidget->cycleContacts(friendWidget, forward)) {
//      return;
//    }

    index += forward ? 1 : -1;

    for (;;) {
      // Bounds checking.
      if (index < 0) {
        index = LAST_TIME;
        continue;
      } else if (index > LAST_TIME) {
        index = 0;
        continue;
      }

      auto *widget = activityLayout->itemAt(index)->widget();
//      categoryWidget = qobject_cast<CategoryWidget *>(widget);

//      if (categoryWidget != nullptr) {
//        if (!categoryWidget->cycleContacts(forward)) {
          // Skip empty or finished categories.
//          index += forward ? 1 : -1;
//          continue;
//        }
//      }

      break;
    }

    return;
  }

  QLayout *currentLayout = nullptr;
//  CircleWidget *circleWidget = nullptr;

  if (friendWidget != nullptr) {
    const ToxPk &pk = friendWidget->getFriend()->getPublicKey();
    uint32_t circleId = Settings::getInstance().getFriendCircleID(pk);
//    circleWidget = CircleWidget::getFromID(circleId);
//    if (circleWidget != nullptr) {
//      if (circleWidget->cycleContacts(friendWidget, forward)) {
//        return;
//      }

//      //      index = circleLayout->indexOfSortedWidget(circleWidget);
//      //      currentLayout = circleLayout->getLayout();
//    } else {
      currentLayout = listLayout->getLayoutOnline();
      index = listLayout->indexOfFriendWidget(friendWidget, true);
      if (index == -1) {
        currentLayout = listLayout->getLayoutOffline();
        index = listLayout->indexOfFriendWidget(friendWidget, false);
      }
    }
//  }

  index += forward ? 1 : -1;

  for (;;) {
    // Bounds checking.
    if (index < 0) {
      currentLayout = nextLayout(currentLayout, forward);
      index = currentLayout->count() - 1;
      continue;
    } else if (index >= currentLayout->count()) {
      currentLayout = nextLayout(currentLayout, forward);
      index = 0;
      continue;
    }

    // Go to the actual next index.
    if (currentLayout == listLayout->getLayoutOnline() ||
        currentLayout == listLayout->getLayoutOffline() ) {
      GenericChatroomWidget *chatWidget = qobject_cast<GenericChatroomWidget *>(
          currentLayout->itemAt(index)->widget());

      if (chatWidget != nullptr)
        emit chatWidget->chatroomWidgetClicked(chatWidget);

      return;
    }
    //    else if (currentLayout == circleLayout->getLayout()) {
    //      circleWidget =
    //          qobject_cast<CircleWidget
    //          *>(currentLayout->itemAt(index)->widget());
    //      if (circleWidget != nullptr) {
    //        if (!circleWidget->cycleContacts(forward)) {
    //           Skip empty or finished circles.
    //          index += forward ? 1 : -1;
    //          continue;
    //        }
    //      }
    //      return;
    //    } else {
    //      return;
    //    }
  }
}

void MessageSessionListWidget::dragEnterEvent(QDragEnterEvent *event) {
  if (!event->mimeData()->hasFormat("toxPk")) {
    return;
  }
  ToxPk toxPk(event->mimeData()->data("toxPk"));
  Friend *frnd = FriendList::findFriend(toxPk);
  if (frnd)
    event->acceptProposedAction();
}

void MessageSessionListWidget::dropEvent(QDropEvent *event) {
  // Check, that the element is dropped from qTox
  QObject *o = event->source();
  auto widget = qobject_cast<MessageSessionWidget *>(o);
  if (!widget)
    return;

  // Check, that the user has a friend with the same ToxPk
  assert(event->mimeData()->hasFormat("toxPk"));
  const ToxPk toxPk{event->mimeData()->data("toxPk")};
  Friend *f = FriendList::findFriend(toxPk);
  if (!f)
    return;

  // Save CircleWidget before changing the Id
//  int circleId = Settings::getInstance().getFriendCircleID(f->getPublicKey());
//  CircleWidget *circleWidget = CircleWidget::getFromID(circleId);

  moveWidget(widget, f->getStatus(), true);

//  if (circleWidget)
//    circleWidget->updateStatus();
}

void MessageSessionListWidget::showEvent(QShowEvent *event) {
  //  auto core = Core::getInstance();
  //  connect(core, &Core::friendAdded, this, &MessageSessionListWidget::addFriend);
  //  for (auto &friendId : core->loadFriendList()) {
  //    qDebug() << "friendId:" << friendId;
  //  };
}

void MessageSessionListWidget::dayTimeout() {
  if (mode == SortingMode::Activity) {
    setMode(SortingMode::Name);
    setMode(SortingMode::Activity); // Refresh all.
  }

  dayTimer->start(base::Times::timeUntilTomorrow());
}

void MessageSessionListWidget::moveWidget(MessageSessionWidget *widget, Status::Status s,
                                  bool add) {
//  if (mode == SortingMode::Name) {
//    const Friend *f = widget->getFriend();
//    int circleId = Settings::getInstance().getFriendCircleID(f->getPublicKey());
//    CircleWidget *circleWidget = CircleWidget::getFromID(circleId);

//    if (circleWidget == nullptr || add) {
//      if (circleId != -1)
//        Settings::getInstance().setFriendCircleID(f->getPublicKey(), -1);
//      listLayout->addWidget(widget);
//      return;
//    }

//    circleWidget->addFriendWidget(widget, s);
//  } else {
//    const Friend *contact = widget->getFriend();
//    auto *categoryWidget = getTimeCategoryWidget(contact);
//    categoryWidget->addFriendWidget(widget, contact->getStatus());
//    categoryWidget->show();
//  }
}

void MessageSessionListWidget::updateActivityTime(const QDateTime &time) {
  if (mode != SortingMode::Activity)
    return;

  int timeIndex = static_cast<int>(base::getTimeBucket(time));
  QWidget *widget = activityLayout->itemAt(timeIndex)->widget();
//  CategoryWidget *categoryWidget = static_cast<CategoryWidget *>(widget);
//  categoryWidget->updateStatus();
//  categoryWidget->setVisible(categoryWidget->hasChatrooms());
}

// update widget after add/delete/hide/show
void MessageSessionListWidget::reDraw() {
  hide();
  show();
  resize(QSize()); // lifehack
}

//CircleWidget *MessageSessionListWidget::createCircleWidget(int id) {
//  if (id == -1)
//    id = Settings::getInstance().addCircle();

//  // Stop, after it has been created. Code after this is for displaying.
//  if (mode == SortingMode::Activity)
//    return nullptr;

//  //  assert(circleLayout != nullptr);

////  CircleWidget *circleWidget = new CircleWidget(this, id);
////  emit connectCircleWidget(*circleWidget);
//  //  circleLayout->addSortedWidget(circleWidget);
////  connect(this, &MessageSessionListWidget::onCompactChanged, circleWidget,
////          &CircleWidget::onCompactChanged);
////  connect(circleWidget, &CircleWidget::renameRequested, this,
////          &MessageSessionListWidget::renameCircleWidget);
////  circleWidget->show(); // Avoid flickering.

////  return circleWidget;
//  return nullptr;
//}

void MessageSessionListWidget::toSendMessage(const ToxPk &pk, bool isGroup)
{
    qDebug() << __func__<< pk.toString();
    auto w = sessionWidgets.value(pk.toString());
    if(!w){
        qDebug() << "Create session for"<<pk.toString();
        w = createMessageSession(pk, "", isGroup ? ChatType::GroupChat:ChatType::Chat);
    }
    emit w->chatroomWidgetClicked(w);

}

QLayout *MessageSessionListWidget::nextLayout(QLayout *layout, bool forward) const {
    qDebug() <<"nextLayout:"<<layout<<"forward:"<<forward;
    if (forward) {
      if (groupsOnTop)
        return listLayout->getLayoutOnline();

      return listLayout->getLayoutOffline();
    } else {
      if (groupsOnTop)
        //        return circleLayout->getLayout();

        return listLayout->getLayoutOnline();
    }

    return nullptr;
}

MessageSessionWidget *MessageSessionListWidget::getMessageSession(const ToxPk &friendPk) {
  return sessionWidgets.value(friendPk.toString());
}

void MessageSessionListWidget::removeFriend(const ToxPk &friendPk) {

  auto f = getMessageSession(friendPk);
  if (!f) {
    return;
  }

  FriendList::removeFriend(friendPk, false);
  removeSessionWidget(f);
  f->deleteLater();
}


void MessageSessionListWidget::slot_sessionClicked(GenericChatroomWidget *actived) {
  for (auto w : sessionWidgets) {
    if (w != actived) {
      w->setActive(false);
    } else {
      w->setActive(true);
    }
  }
}

void MessageSessionListWidget::setRecvFriendMessage(
    ToxPk friendnumber, const FriendMessage &message,
    bool isAction) {

  Friend *f = FriendList::findFriend(friendnumber);
  if (!f) {
    /**
     * 陌生人消息（可能是已经将对方删除，通讯录没有对方记录）
     */
    qWarning() << "Can not find friend:" << friendnumber.toString()
               << ", so add it to contact.";
    createMessageSession(friendnumber, message.id, ChatType::Chat);
    return;
  }

  auto fw = getMessageSession(friendnumber);
  fw->setRecvMessage(message, isAction);
}


void MessageSessionListWidget::setFriendStatus(const ToxPk &friendPk, Status::Status status) {
  auto fw = getMessageSession(friendPk);
  if (!fw) {
    qWarning() << "friend widget is no existing.";
    return;
  }
  fw->setStatus(status, false);
}

void MessageSessionListWidget::setFriendStatusMsg(const ToxPk &friendPk,
                                          const QString &statusMsg) {
  auto fw = getMessageSession(friendPk);
  if (!fw) {
    qWarning() << "friend widget no exist.";
    return;
  }

  fw->setStatusMsg(statusMsg);
}

void MessageSessionListWidget::setFriendName(const ToxPk &friendPk,
                                     const QString &name) {

    auto w = FriendList::findFriend(friendPk);

  if(!w){
      qWarning() <<"friend is no existing." << friendPk.toString();
      return;
  }

    w->setName(name);



}

void MessageSessionListWidget::setFriendAvatar(const ToxPk &friendPk,
                                       const QByteArray &avatar) {
  auto fw = getMessageSession(friendPk);
  if (!fw) {
    qWarning() << "friend is no exist.";
    return;
  }

  QPixmap p;
  p.loadFromData(avatar);
  fw->setAvatar(p);
}

void MessageSessionListWidget::setFriendTyping(const ToxPk &friendId, bool isTyping) {
  auto fw = getMessageSession(friendId);
  if (fw)
    fw->setTyping(isTyping);
}

void MessageSessionListWidget::reloadTheme() {
  for (auto fw: sessionWidgets) {
    fw->reloadTheme();
  }
}
