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

#include "friendlistwidget.h"
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

inline QDateTime getActiveTimeFriend(const Friend *contact) {
  return Settings::getInstance().getFriendActivity(contact->getPublicKey());
}

FriendListWidget::FriendListWidget(MainLayout *parent, ContentLayout*contentLayout, bool groupsOnTop)
    : QWidget(parent), m_contentLayout{contentLayout}, groupsOnTop(groupsOnTop) {
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);



  groupLayout.getLayout()->setSpacing(0);
  groupLayout.getLayout()->setMargin(0);

  // Prevent QLayout's add child warning before setting the mode.
  listLayout = new FriendListLayout(this);
  listLayout->removeItem(listLayout->getLayoutOnline());
  listLayout->removeItem(listLayout->getLayoutOffline());
  setLayout(listLayout);

  mode = Settings::getInstance().getFriendSortingMode();
  sortByMode(mode);

  onGroupchatPositionChanged(groupsOnTop);
  dayTimer = new QTimer(this);
  dayTimer->setTimerType(Qt::VeryCoarseTimer);
  connect(dayTimer, &QTimer::timeout, this, &FriendListWidget::dayTimeout);
  dayTimer->start(base::Times::timeUntilTomorrow());

  setAcceptDrops(true);

  auto &settings = Settings::getInstance();


  connect(&settings, &Settings::groupchatPositionChanged, this,
          &FriendListWidget::onGroupchatPositionChanged);

  auto widget = Widget::getInstance();
  connect(widget, &Widget::toShowDetails, this,
          &FriendListWidget::do_toShowDetails);

  connect(Nexus::getProfile(), &Profile::coreChanged,
          this, &FriendListWidget::onCoreChanged);

}

FriendListWidget::~FriendListWidget() {
  if (activityLayout != nullptr) {
    QLayoutItem *item;
    while ((item = activityLayout->takeAt(0)) != nullptr) {
      delete item->widget();
      delete item;
    }
    delete activityLayout;
  }

}

FriendWidget *FriendListWidget::addFriend(const FriendInfo &friendInfo) {
  qDebug() << __func__ << friendInfo;
  auto exist = getFriend(friendInfo.id);
  if (exist) {
      qWarning() <<"Exist friend"<< friendInfo.id;
    return exist;
  }

  auto core = Core::getInstance();
  auto profile = Nexus::getProfile();

  auto &settings = Settings::getInstance();
  const auto compact = settings.getCompactLayout();

  auto fw = new FriendWidget(m_contentLayout, friendInfo, this);
  connectFriendWidget(*fw);
  friendWidgets.insert(friendInfo.getId().toString(), fw);
  //  CircleWidget *circleWidget = CircleWidget::getFromID(circleIndex);
  //  if (circleWidget == nullptr)
  //    moveWidget(fw, s, true);
  //  else
  //    circleWidget->addFriendWidget(fw, s);
  auto status = core->getFriendStatus(friendInfo.toString());
  listLayout->addFriendWidget(fw,status);
  setFriendStatus(friendInfo.getId(), status);


  //  TODO 连接朋友活跃状态
  //  connect(chatForm, &ChatForm::updateFriendActivity, this,
  //          &Widget::updateFriendActivity);

  //  friendMessageDispatchers[friendInfo] = messageDispatcher;
  //  friendChatLogs[friendInfo] = chatHistory;
  //  friendChatrooms[friendInfo] = chatRoom;
  //  friendWidgets[friendInfo] = friendWidget;
  //  chatForms[friendInfo] = chatForm;

  //  const auto activityTime = settings.getFriendActivity(friendInfo);
  //  const auto chatTime = chatForm->getLatestTime();
  //  if (chatTime > activityTime && chatTime.isValid()) {
  //    settings.setFriendActivity(friendInfo, chatTime);
  //  }

//  listLayout->addWidget(fw);


  //
  //  auto notifyReceivedCallback = [this, friendInfo](const ToxPk &author,
  //                                                 const Message &message) {
  //    auto isTargeted =
  //        std::any_of(message.metadata.begin(), message.metadata.end(),
  //                    [](MessageMetadata metadata) {
  //                      return metadata.type ==
  //                      MessageMetadataType::selfMention;
  //                    });
  //    newFriendMessageAlert(friendInfo, message.content);
  //  };
  //
  //  auto notifyReceivedConnection =
  //      connect(messageDispatcher.get(),
  //              &IMessageDispatcher::messageReceived, notifyReceivedCallback);
  //
  //  friendAlertConnections.insert(friendInfo, notifyReceivedConnection);
  //
  //  connect(newfriend, &IMFriend::aliasChanged, this,
  //          &Widget::onFriendAliasChanged);
  //  connect(newfriend, &IMFriend::displayedNameChanged, this,
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

  //
  //  Profile *profile = Nexus::getProfile();
  //  connect(profile, &Profile::friendAvatarSet, friendWidget,
  //          &FriendWidget::onAvatarSet);
  //  connect(profile, &Profile::friendAvatarRemoved, friendWidget,
  //          &FriendWidget::onAvatarRemoved);
  //
  //  // Try to get the avatar from the cache
  //  QPixmap avatar = Nexus::getProfile()->loadAvatar(friendInfo);
  //  if (!avatar.isNull()) {
  //    //chatForm->onAvatarChanged(friendInfo, avatar);
  //    //friendWidget->onAvatarSet(friendInfo, avatar);
  //  }
  //
//    FilterCriteria filter = getFilterCriteria();
//    friendWidget->search(ui->searchContactText->text(),
//    filterOffline(filter));

  //  core->getFriendInfo(friendInfo.toString());

  return fw;
}

void FriendListWidget::connectFriendWidget(FriendWidget &fw) {
//  connect(&friendWidget, &FriendWidget::searchCircle, this,
//          &FriendListWidget::searchCircle);
  connect(&fw, &FriendWidget::updateFriendActivity, this,
          &FriendListWidget::updateFriendActivity);

    connect(&fw, &FriendWidget::removeFriend, this,
            &FriendListWidget::removeFriend);

    //  connect(fw, &FriendWidget::friendWidgetRenamed, this,
    //          &FriendListWidget::onFriendWidgetRenamed);
      connect(&fw, &FriendWidget::friendWidgetClicked, this,
              &FriendListWidget::slot_friendClicked);
}

void FriendListWidget::updateFriendActivity(const Friend &frnd) {
  const FriendId &pk = frnd.getPublicKey();
  auto &settings = Settings::getInstance();
  const auto oldTime = settings.getFriendActivity(pk);
  const auto newTime = QDateTime::currentDateTime();
  settings.setFriendActivity(pk, newTime);
  FriendWidget *widget = getFriend(frnd.getPublicKey());
  moveWidget(widget, frnd.getStatus());
  updateActivityTime(oldTime); // update old category widget
}

void FriendListWidget::connectToCore(Core *core)
{

}

void FriendListWidget::setMode(SortingMode mode) {
  if (this->mode == mode)
    return;

  this->mode = mode;
  Settings::getInstance().setFriendSortingMode(mode);

  sortByMode(mode);
}

void FriendListWidget::sortByMode(SortingMode mode) {
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
//    for (int i = 0; i < count; i++) {
//      QWidget *widget = activityLayout->itemAt(i)->widget();
//      CategoryWidget *categoryWidget = qobject_cast<CategoryWidget *>(widget);
//      if (categoryWidget) {
//        categoryWidget->moveFriendWidgets(this);
//      } else {
//        qWarning() << "Unexpected widget";
//      }
//    }

    listLayout->addLayout(listLayout->getLayoutOnline());
    listLayout->addLayout(listLayout->getLayoutOffline());
    //    listLayout->addLayout(circleLayout->getLayout());
    onGroupchatPositionChanged(groupsOnTop);

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
        const QMap<base::ReadableTime, QString> names {
            { base::ReadableTime::Today,     tr("Today",                      COMMENT) },
            { base::ReadableTime::Yesterday, tr("Yesterday",                  COMMENT) },
            { base::ReadableTime::ThisWeek,  tr("Last 7 days",                COMMENT) },
            { base::ReadableTime::ThisMonth, tr("This month",                 COMMENT) },
            { base::ReadableTime::LongAgo,   tr("Older than 6 Months",        COMMENT) },
            { base::ReadableTime::Never,     tr("Never",                      COMMENT) },
            { base::ReadableTime::Month1Ago, ql.monthName(today.addMonths(-1).month()) },
            { base::ReadableTime::Month2Ago, ql.monthName(today.addMonths(-2).month()) },
            { base::ReadableTime::Month3Ago, ql.monthName(today.addMonths(-3).month()) },
            { base::ReadableTime::Month4Ago, ql.monthName(today.addMonths(-4).month()) },
            { base::ReadableTime::Month5Ago, ql.monthName(today.addMonths(-5).month()) },
        };
// clang-format on
#undef COMMENT

    activityLayout = new QVBoxLayout();
//    bool compact = Settings::getInstance().getCompactLayout();
//    for (base::ReadableTime t : names.keys()) {
//      CategoryWidget *category = new CategoryWidget(compact, this);
//      category->setName(names[t]);
//      activityLayout->addWidget(category);
//    }

    moveFriends(listLayout->getLayoutOffline());
    moveFriends(listLayout->getLayoutOnline());
    //    if (circleLayout != nullptr) {
    //      moveFriends(circleLayout->getLayout());
    //    }

//    for (int i = 0; i < activityLayout->count(); ++i) {
//      QWidget *widget = activityLayout->itemAt(i)->widget();
//      CategoryWidget *categoryWidget = qobject_cast<CategoryWidget *>(widget);
//      categoryWidget->setVisible(categoryWidget->hasChatrooms());
//    }

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

void FriendListWidget::moveFriends(QLayout *layout) {
  while (!layout->isEmpty()) {
    QWidget *widget = layout->itemAt(0)->widget();
//    FriendWidget *friendWidget = qobject_cast<FriendWidget *>(widget);
//    CircleWidget *circleWidget = qobject_cast<CircleWidget *>(widget);
//    if (circleWidget) {
//      circleWidget->moveFriendWidgets(this);
//    } else if (friendWidget) {
//      const IMFriend *contact = friendWidget->getFriend();
//      auto *categoryWidget = getTimeCategoryWidget(contact);
//      categoryWidget->addFriendWidget(friendWidget, contact->getStatus());
//    }
  }
}

//CategoryWidget *
//FriendListWidget::getTimeCategoryWidget(const IMFriend *frd) const {
//  const auto activityTime = getActiveTimeFriend(frd);
//  int timeIndex = static_cast<int>(base::getTimeBucket(activityTime));
//  QWidget *widget = activityLayout->itemAt(timeIndex)->widget();
//  return qobject_cast<CategoryWidget *>(widget);
//}

FriendListWidget::SortingMode FriendListWidget::getMode() const { return mode; }

GroupWidget *FriendListWidget::addGroup(const GroupId &groupId,
                                        const QString &groupName) {

  qDebug() << __func__ << groupId.toString();

  Group *g = GroupList::findGroup(groupId);
  if (g) {
    qWarning() << "Group already exist.";
    return groupWidgets.value(groupId.toString());
  }

  auto core = Core::getInstance();
  auto &settings = Settings::getInstance();

//  const bool enabled = core->getGroupAvEnabled(groupId.toString());

  const auto compact = settings.getCompactLayout();
  auto gw = new GroupWidget(m_contentLayout,
                                groupId.toString(),
                                groupId,
                                groupName,
                                compact);
  groupWidgets[groupId.toString()] = gw;

  //  auto notifyReceivedCallback = [this, groupId](const ToxPk &author,
  //                                                const Message &message) {
  //    auto isTargeted =
  //        std::any_of(message.metadata.begin(), message.metadata.end(),
  //                    [](MessageMetadata metadata) {
  //                      return metadata.type ==
  //                      MessageMetadataType::selfMention;
  //                    });
  //    newGroupMessageAlert(groupId, author, message.content,
  //                         isTargeted || settings.getGroupAlwaysNotify());
  //  };
  //
  //  auto notifyReceivedConnection =
  //      connect(messageDispatcher.get(), &IMessageDispatcher::messageReceived,
  //              notifyReceivedCallback);
  //  groupAlertConnections.insert(groupId, notifyReceivedConnection);
  //
  //  auto form =
  //      new GroupChatForm(newgroup, *groupChatLog, *messageDispatcher,
  //      settings);
  //  connect(&settings, &Settings::nameColorsChanged, form,
  //          &GenericChatForm::setColorizedNames);
  //  form->setColorizedNames(settings.getEnableGroupChatsColor());
  //  groupMessageDispatchers[groupId] = messageDispatcher;
  //  groupChatLogs[groupId] = groupChatLog;
  //  groupWidgets[groupId] = widget;
  //  groupChatrooms[groupId] = chatroom;
  //  groupChatForms[groupId] = QSharedPointer<GroupChatForm>(form);
  //
  groupLayout.addSortedWidget(gw);

  connect(g, &Group::subjectChanged,
          [=, this](const QString &author, const QString &name) {
            Q_UNUSED(author);
            renameGroupWidget(gw, name);
          });

  connect(gw, &GroupWidget::chatroomWidgetClicked, this,
          &FriendListWidget::slot_groupClicked);
  connect(gw, &GroupWidget::removeGroup, this, &FriendListWidget::do_groupDeleted);
  //
  //  widget->updateStatusLight();
  //  contactListWidget->activateWindow();
  //
  //  connect(widget, &GroupWidget::chatroomWidgetClicked, this,
  //          &Widget::onChatroomWidgetClicked);
  //  connect(widget, &GroupWidget::newWindowOpened, this,
  //  &Widget::openNewDialog);
  // #if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
  //  auto widgetRemoveGroup = QOverload<const GroupId
  //  &>::of(&Widget::removeGroup); auto widgetDestroyGroup = QOverload<const
  //  GroupId &>::of(&Widget::destroyGroup);
  // #else
  //  auto widgetRemoveGroup =
  //      static_cast<void (Widget::*)(const GroupId &)>(&Widget::removeGroup);
  //  auto widgetDestroyGroup =
  //      static_cast<void (Widget::*)(const GroupId &)>(&Widget::destroyGroup);
  // #endif
  //  connect(widget, &GroupWidget::removeGroup, this, widgetRemoveGroup);
  //  connect(widget, &GroupWidget::destroyGroup, this, widgetDestroyGroup);
  //  //  connect(widget, &GroupWidget::middleMouseClicked, this,
  //  //          [this]() { removeGroup(groupId); });
  //  connect(widget, &GroupWidget::chatroomWidgetClicked, form,
  //          &ChatForm::focusInput);
  //  connect(newgroup, &Group::titleChangedByUser, this,
  //          &Widget::titleChangedByUser);
  //  connect(core, &Core::usernameSet, newgroup, &Group::setSelfName);
  //
  //  FilterCriteria filter = getFilterCriteria();
  //  widget->searchName(ui->searchContactText->text(), filterGroups(filter));

  return gw;
}

void FriendListWidget::do_groupDeleted(const ContactId &cid)
{
    removeGroup(GroupId(cid));
}

void FriendListWidget::removeGroup(const GroupId &cid) {
    auto gw = groupWidgets.value(cid.toString());
    gw->deleteLater();
    groupLayout.removeSortedWidget(gw);
}

void FriendListWidget::setGroupTitle(const GroupId& groupId,
                                     const QString &author,
                                     const QString &title)
{
    auto g = GroupList::findGroup(groupId);
    if(!g){
        qWarning() << "group is no existing."<<groupId.toString();
        return;
    }
    g->setSubject(author, title);
}

void FriendListWidget::setGroupInfo(const GroupId &groupId, const GroupInfo &info)
{
    auto g = GroupList::findGroup(groupId);
    if(!g){
        qWarning() << "group is no existing."<<groupId.toString();
        return;
    }

    g->setName(info.name);
    g->setDesc(info.description);
    g->setSubject("", info.subject);
    g->setPeerCount(info.occupants);

}



//void FriendListWidget::addCircleWidget(int id) { createCircleWidget(id); }

//void FriendListWidget::addCircleWidget(FriendWidget *friendWidget) {
//  CircleWidget *circleWidget = createCircleWidget();
//  if (circleWidget != nullptr) {
//    if (friendWidget != nullptr) {
//      const IMFriend *f = friendWidget->getFriend();
//      ToxPk toxPk = f->getPublicKey();
//      int circleId = Settings::getInstance().getFriendCircleID(toxPk);
//      CircleWidget *circleOriginal = CircleWidget::getFromID(circleId);

//      circleWidget->addFriendWidget(friendWidget, f->getStatus());
//      circleWidget->setExpanded(true);

//      if (circleOriginal != nullptr)
//        emit searchCircle(*circleOriginal);
//    }

//    emit searchCircle(*circleWidget);

//    if (window()->isActiveWindow())
//      circleWidget->editName();
//  }
//  reDraw();
//}

//void FriendListWidget::removeCircleWidget(CircleWidget *widget) {
//  //  circleLayout->removeSortedWidget(widget);
//  widget->deleteLater();
//}

void FriendListWidget::searchChatrooms(const QString &searchString,
                                       bool hideOnline, bool hideOffline,
                                       bool hideGroups) {
  groupLayout.search(searchString, hideGroups);
  listLayout->searchChatrooms(searchString, hideOnline, hideOffline);
  //
  //  if (circleLayout != nullptr) {
  //    for (int i = 0; i != circleLayout->getLayout()->count(); ++i) {
  //      CircleWidget *circleWidget = static_cast<CircleWidget *>(
  //          circleLayout->getLayout()->itemAt(i)->widget());
  //      circleWidget->search(searchString, true, hideOnline, hideOffline);
  //    }
  //  } else if (activityLayout != nullptr) {
  //    for (int i = 0; i != activityLayout->count(); ++i) {
  //      CategoryWidget *categoryWidget =
  //          static_cast<CategoryWidget
  //          *>(activityLayout->itemAt(i)->widget());
  //      categoryWidget->search(searchString, true, hideOnline, hideOffline);
  //      categoryWidget->setVisible(categoryWidget->hasChatrooms());
  //    }
  //  }
}

void FriendListWidget::renameGroupWidget(GroupWidget *groupWidget,
                                         const QString &newName) {
  groupLayout.removeSortedWidget(groupWidget);
  groupLayout.addSortedWidget(groupWidget);
}

//void FriendListWidget::renameCircleWidget(CircleWidget *circleWidget,
//                                          const QString &newName) {
  //  circleLayout->removeSortedWidget(circleWidget);
  //  circleWidget->setName(newName);
  //  circleLayout->addSortedWidget(circleWidget);
//}

void FriendListWidget::onFriendWidgetRenamed(FriendWidget *friendWidget) {
//  const IMFriend *contact = friendWidget->getFriend();
//  auto status = contact->getStatus();
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
//      listLayout->removeFriendWidget(friendWidget, status);
//      listLayout->addFriendWidget(friendWidget, status);
//    }
//  }


}

void FriendListWidget::onGroupchatPositionChanged(bool top) {
  groupsOnTop = top;

  if (mode != SortingMode::Name)
    return;

  listLayout->removeItem(groupLayout.getLayout());

  if (top)
    listLayout->insertLayout(0, groupLayout.getLayout());
  else
    listLayout->insertLayout(1, groupLayout.getLayout());

  reDraw();
}

//void FriendListWidget::cycleContacts(
//    GenericChatroomWidget *activeChatroomWidget, bool forward) {
//  if (!activeChatroomWidget) {
//    return;
//  }

//  int index = -1;
//  FriendWidget *friendWidget =
//      qobject_cast<FriendWidget *>(activeChatroomWidget);

//  if (mode == SortingMode::Activity) {
//    if (!friendWidget) {
//      return;
//    }

//    const auto activityTime = getActiveTimeFriend(friendWidget->getFriend());
//    index = static_cast<int>(base::getTimeBucket(activityTime));
//    QWidget *widget = activityLayout->itemAt(index)->widget();
////    CategoryWidget *categoryWidget = qobject_cast<CategoryWidget *>(widget);

////    if (categoryWidget == nullptr ||
////        categoryWidget->cycleContacts(friendWidget, forward)) {
////      return;
////    }

//    index += forward ? 1 : -1;

//    for (;;) {
//      // Bounds checking.
//      if (index < 0) {
//        index = base::LAST_TIME;
//        continue;
//      } else if (index >base::LAST_TIME) {
//        index = 0;
//        continue;
//      }

////      auto *widget = activityLayout->itemAt(index)->widget();
////      categoryWidget = qobject_cast<CategoryWidget *>(widget);

////      if (categoryWidget != nullptr) {
////        if (!categoryWidget->cycleContacts(forward)) {
////          // Skip empty or finished categories.
////          index += forward ? 1 : -1;
////          continue;
////        }
////      }

//      break;
//    }

//    return;
//  }

//  QLayout *currentLayout = nullptr;
//  CircleWidget *circleWidget = nullptr;

//  if (friendWidget != nullptr) {
//    const ToxPk &pk = friendWidget->getFriend()->getPublicKey();
//    uint32_t circleId = Settings::getInstance().getFriendCircleID(pk);
//    circleWidget = CircleWidget::getFromID(circleId);
//    if (circleWidget != nullptr) {
//      if (circleWidget->cycleContacts(friendWidget, forward)) {
//        return;
//      }

//      //      index = circleLayout->indexOfSortedWidget(circleWidget);
//      //      currentLayout = circleLayout->getLayout();
//    } else {
//      currentLayout = listLayout->getLayoutOnline();
//      index = listLayout->indexOfFriendWidget(friendWidget, true);
//      if (index == -1) {
//        currentLayout = listLayout->getLayoutOffline();
//        index = listLayout->indexOfFriendWidget(friendWidget, false);
//      }
//    }
//  } else {
//    GroupWidget *groupWidget =
//        qobject_cast<GroupWidget *>(activeChatroomWidget);
//    if (groupWidget != nullptr) {
//      currentLayout = groupLayout.getLayout();
//      index = groupLayout.indexOfSortedWidget(groupWidget);
//    } else {
//      return;
//    };
//  }

//  index += forward ? 1 : -1;

//  for (;;) {
//    // Bounds checking.
//    if (index < 0) {
//      currentLayout = nextLayout(currentLayout, forward);
//      index = currentLayout->count() - 1;
//      continue;
//    } else if (index >= currentLayout->count()) {
//      currentLayout = nextLayout(currentLayout, forward);
//      index = 0;
//      continue;
//    }

//    // Go to the actual next index.
//    if (currentLayout == listLayout->getLayoutOnline() ||
//        currentLayout == listLayout->getLayoutOffline() ||
//        currentLayout == groupLayout.getLayout()) {
//      GenericChatroomWidget *chatWidget = qobject_cast<GenericChatroomWidget *>(
//          currentLayout->itemAt(index)->widget());

//      if (chatWidget != nullptr)
//        emit chatWidget->chatroomWidgetClicked(chatWidget);

//      return;
//    }
//    //    else if (currentLayout == circleLayout->getLayout()) {
//    //      circleWidget =
//    //          qobject_cast<CircleWidget
//    //          *>(currentLayout->itemAt(index)->widget());
//    //      if (circleWidget != nullptr) {
//    //        if (!circleWidget->cycleContacts(forward)) {
//    //           Skip empty or finished circles.
//    //          index += forward ? 1 : -1;
//    //          continue;
//    //        }
//    //      }
//    //      return;
//    //    } else {
//    //      return;
//    //    }
//  }
//}

void FriendListWidget::dragEnterEvent(QDragEnterEvent *event) {
  if (!event->mimeData()->hasFormat("toxPk")) {
    return;
  }
  FriendId toxPk(event->mimeData()->data("toxPk"));
  Friend *frnd = FriendList::findFriend(toxPk);
  if (frnd)
    event->acceptProposedAction();
}

void FriendListWidget::dropEvent(QDropEvent *event) {
  // Check, that the element is dropped from qTox
  QObject *o = event->source();
  FriendWidget *widget = qobject_cast<FriendWidget *>(o);
  if (!widget)
    return;

  // Check, that the user has a friend with the same ToxPk
  assert(event->mimeData()->hasFormat("toxPk"));
  const FriendId toxPk{event->mimeData()->data("toxPk")};
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

void FriendListWidget::showEvent(QShowEvent *event) {
  //  auto core = Core::getInstance();
  //  connect(core, &Core::friendAdded, this, &FriendListWidget::addFriend);
  //  for (auto &receiver : core->loadFriendList()) {
  //    qDebug() << "receiver:" << receiver;
    //  };
}

void FriendListWidget::onCoreChanged(Core &core_)
{
    core = &core_;
    connectToCore(core);
}

void FriendListWidget::dayTimeout() {
  if (mode == SortingMode::Activity) {
    setMode(SortingMode::Name);
    setMode(SortingMode::Activity); // Refresh all.
  }

  dayTimer->start(base::Times::timeUntilTomorrow());
}

void FriendListWidget::moveWidget(FriendWidget *widget, Status::Status s,
                                  bool add) {
  if (mode == SortingMode::Name) {
    const Friend *f = widget->getFriend();
//    int circleId = Settings::getInstance().getFriendCircleID(f->getPublicKey());
//    CircleWidget *circleWidget = CircleWidget::getFromID(circleId);

//    if (circleWidget == nullptr || add) {
//      if (circleId != -1)
//        Settings::getInstance().setFriendCircleID(f->getPublicKey(), -1);

//      listLayout->addFriendWidget(widget, s);
//      return;
//    }

//    circleWidget->addFriendWidget(widget, s);
  } else {
//    const IMFriend *contact = widget->getFriend();
//    auto *categoryWidget = getTimeCategoryWidget(contact);
//    categoryWidget->addFriendWidget(widget, contact->getStatus());
//    categoryWidget->show();
  }
}

void FriendListWidget::updateActivityTime(const QDateTime &time) {
  if (mode != SortingMode::Activity)
    return;

  int timeIndex = static_cast<int>(base::getTimeBucket(time));
//  QWidget *widget = activityLayout->itemAt(timeIndex)->widget();
//  CategoryWidget *categoryWidget = static_cast<CategoryWidget *>(widget);
//  categoryWidget->updateStatus();

//  categoryWidget->setVisible(categoryWidget->hasChatrooms());
}

// update widget after add/delete/hide/show
void FriendListWidget::reDraw() {
  hide();
  show();
  resize(QSize()); // lifehack
}

//CircleWidget *FriendListWidget::createCircleWidget(int id) {
//  if (id == -1)
//    id = Settings::getInstance().addCircle();

//  // Stop, after it has been created. Code after this is for displaying.
//  if (mode == SortingMode::Activity)
//    return nullptr;

//  //  assert(circleLayout != nullptr);

//  CircleWidget *circleWidget = new CircleWidget(this, id);
//  emit connectCircleWidget(*circleWidget);
//  //  circleLayout->addSortedWidget(circleWidget);
//  connect(this, &FriendListWidget::onCompactChanged, circleWidget,
//          &CircleWidget::onCompactChanged);
//  connect(circleWidget, &CircleWidget::renameRequested, this,
//          &FriendListWidget::renameCircleWidget);
//  circleWidget->show(); // Avoid flickering.

//  return circleWidget;
//}

QLayout *FriendListWidget::nextLayout(QLayout *layout, bool forward) const {
  if (layout == groupLayout.getLayout()) {
    if (forward) {
      if (groupsOnTop)
        return listLayout->getLayoutOnline();

      return listLayout->getLayoutOffline();
    } else {
      if (groupsOnTop)
        //        return circleLayout->getLayout();

        return listLayout->getLayoutOnline();
    }
  } else if (layout == listLayout->getLayoutOnline()) {
    if (forward) {
      if (groupsOnTop)
        return listLayout->getLayoutOffline();

      return groupLayout.getLayout();
    } else {
      if (groupsOnTop)
        return groupLayout.getLayout();

      //      return circleLayout->getLayout();
    }
  } else if (layout == listLayout->getLayoutOffline()) {
    //    if (forward)
    //      return circleLayout->getLayout();
    //    else if (groupsOnTop)
    //      return listLayout->getLayoutOnline();

    return groupLayout.getLayout();
  }
  //  else if (layout == circleLayout->getLayout()) {
  //    if (forward) {
  //      if (groupsOnTop)
  //        return groupLayout.getLayout();
  //
  //      return listLayout->getLayoutOnline();
  //    } else
  //      return listLayout->getLayoutOffline();
  //  }
  return nullptr;
}

FriendWidget *FriendListWidget::getFriend(const ContactId &friendPk) {
  return friendWidgets.value(friendPk.toString());
}

void FriendListWidget::removeFriend(const FriendId &friendPk) {
  qDebug() << __func__ << friendPk.toString();

  auto fw = friendWidgets.value(friendPk.toString());
  if(!fw){
      qWarning() <<"Unable to find friendWidget";
    return;
  }
    listLayout->removeFriendWidget(fw);
    friendWidgets.remove(friendPk.toString());
    emit deleteFriendWidget(friendPk);
    delete fw;

}

GroupWidget *FriendListWidget::getGroup(const GroupId &id) {
  return groupWidgets.value(id.toString());
}

void FriendListWidget::slot_groupClicked(GenericChatroomWidget *actived) {
  for (auto fw : friendWidgets) {
    fw->setActive(false);
  }
  for (auto gw : groupWidgets) {
    if (gw != actived) {
      gw->setActive(false);
    } else {
      actived->setActive(true);
    }
  }
}

void FriendListWidget::slot_friendClicked(GenericChatroomWidget *actived) {
  for (auto gw : groupWidgets) {
    gw->setActive(false);
  }

  for (auto fw : friendWidgets) {
    if (fw != actived) {
      fw->setActive(false);
    } else {
      fw->setActive(true);
    }
  }
}
void FriendListWidget::setRecvGroupMessage(const GroupMessage& msg) {
//  const GroupId &groupId = msg.groupId;
//  auto gw = getGroup(groupId);
//  if (!gw) {
//      qWarning() <<"group is no existing";
//      return;
//  }
//    gw->setRecvMessage(msg);

}

void FriendListWidget::setFriendStatus(const ContactId &friendPk,
                                       Status::Status status) {
  auto fw = getFriend(friendPk);
  if (!fw) {
    qWarning() << "friend"<< friendPk.toString() << "widget is no existing.";
    return;
  }
  fw->setStatus(status, false);
}

void FriendListWidget::setFriendStatusMsg(const FriendId &friendPk,
                                          const QString &statusMsg) {
  auto fw = getFriend(friendPk);
  if (!fw) {
    qWarning() << "friend widget no exist.";
    return;
  }

  fw->setStatusMsg(statusMsg);
}

void FriendListWidget::setFriendName(const FriendId &friendPk, const QString &name) {
  qDebug() << __func__ << friendPk.toString() << name;
  auto f = FriendList::findFriend(friendPk);
  if(!f){
      qWarning() <<"friend is no existing.";
      return;
  }
  f->setName(name);
}

void FriendListWidget::setFriendAlias(const FriendId &friendPk, const QString &alias)
{
    qDebug() << __func__ << friendPk.toString() << alias;
    auto f = FriendList::findFriend(friendPk);
    if(!f){
        qWarning() <<"friend is no existing.";
        return;
    }
    f->setAlias(alias);
}

void FriendListWidget::setFriendAvatar(const FriendId &friendPk,
                                       const QByteArray &avatar) {
  auto fw = getFriend(friendPk);
  if (!fw) {
    qWarning() << "friend is no exist.";
    return;
  }

  QPixmap p;
  p.loadFromData(avatar);
  fw->setAvatar(p);
}

void FriendListWidget::setFriendTyping(const FriendId &friendId, bool isTyping) {
  auto fw = getFriend(friendId);
  if (fw)
    fw->setTyping(isTyping);
}
void FriendListWidget::reloadTheme() {
  for (auto gw: groupWidgets) {
    gw->reloadTheme();
  }

  for (auto fw: friendWidgets) {
    fw->reloadTheme();
  }
}

void FriendListWidget::do_toShowDetails(const ContactId &cid) {
  qDebug() << __func__  << cid.toString();
  for (auto fw: friendWidgets) {
    if(fw->getContactId()==cid){
      emit fw->chatroomWidgetClicked(fw);
      break;
    }
  }

  for(auto gw: groupWidgets){
      if(gw->getContactId() == cid){
          emit gw->chatroomWidgetClicked(gw);
          break;
      }
  }
}

