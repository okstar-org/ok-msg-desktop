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
// #include "src/widget/categorywidget.h"
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QGridLayout>
#include <QMimeData>
#include <QTimer>
#include <cassert>
#include "widget.h"

inline QDateTime getActiveTimeFriend(const Friend* contact) {
    return Settings::getInstance().getFriendActivity(contact->getPublicKey());
}

FriendListWidget::FriendListWidget(MainLayout* parent, ContentLayout* contentLayout,
                                   bool groupsOnTop)
        : QWidget(parent), m_contentLayout{contentLayout}, groupsOnTop(groupsOnTop) {
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    groupLayout.getLayout()->setSpacing(0);
    groupLayout.getLayout()->setMargin(0);

    // Prevent QLayout's add child warning before setting the mode.
    listLayout = new FriendListLayout(this);
    setLayout(listLayout);

    mode = Settings::getInstance().getFriendSortingMode();
    sortByMode(mode);

    onGroupchatPositionChanged(groupsOnTop);
    dayTimer = new QTimer(this);
    dayTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(dayTimer, &QTimer::timeout, this, &FriendListWidget::dayTimeout);
    dayTimer->start(ok::base::Times::timeUntilTomorrow());

    setAcceptDrops(true);

    auto& settings = Settings::getInstance();

    connect(&settings, &Settings::groupchatPositionChanged, this,
            &FriendListWidget::onGroupchatPositionChanged);

    auto widget = Widget::getInstance();
    connect(widget, &Widget::toShowDetails, this, &FriendListWidget::do_toShowDetails);

    //  connect(Nexus::getProfile(), &Profile::coreChanged,
    //          this, &FriendListWidget::onCoreChanged);
}

FriendListWidget::~FriendListWidget() {
    if (activityLayout != nullptr) {
        QLayoutItem* item;
        while ((item = activityLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete activityLayout;
    }
}

FriendWidget* FriendListWidget::addFriend(const FriendInfo& friendInfo) {
    qDebug() << __func__ << friendInfo;
    auto exist = getFriend(friendInfo.id);
    if (exist) {
        qWarning() << "Exist friend" << friendInfo.id;
        return exist;
    }

    auto core = Core::getInstance();
    auto profile = Nexus::getProfile();

    auto& settings = Settings::getInstance();
    const auto compact = settings.getCompactLayout();

    auto fw = new FriendWidget(m_contentLayout, friendInfo, this);
    connectFriendWidget(*fw);
    friendWidgets.insert(friendInfo.getId().toString(), fw);

    auto frid = Nexus::getCore()->getFriendList().findFriend(friendInfo.getId());
    emit Widget::getInstance() -> friendAdded(frid);

    //  CircleWidget *circleWidget = CircleWidget::getFromID(circleIndex);
    //  if (circleWidget == nullptr)
    //    moveWidget(fw, s, true);
    //  else
    //    circleWidget->addFriendWidget(fw, s);
    auto status = core->getFriendStatus(friendInfo.toString());
    listLayout->addFriendWidget(fw, status);
    setFriendStatus(friendInfo.getId(), status);
    return fw;
}

void FriendListWidget::connectFriendWidget(FriendWidget& fw) {
    //  connect(&friendWidget, &FriendWidget::searchCircle, this,
    //          &FriendListWidget::searchCircle);
    connect(&fw, &FriendWidget::updateFriendActivity, this,
            &FriendListWidget::updateFriendActivity);

    connect(&fw, &FriendWidget::removeFriend, this, &FriendListWidget::removeFriend);

    //  connect(fw, &FriendWidget::friendWidgetRenamed, this,
    //          &FriendListWidget::onFriendWidgetRenamed);
    connect(&fw, &FriendWidget::friendWidgetClicked, this, &FriendListWidget::slot_friendClicked);
}

void FriendListWidget::updateFriendActivity(const Friend& frnd) {
    const FriendId& pk = frnd.getPublicKey();
    auto& settings = Settings::getInstance();
    const auto oldTime = settings.getFriendActivity(pk);
    const auto newTime = QDateTime::currentDateTime();
    settings.setFriendActivity(pk, newTime);
    FriendWidget* widget = getFriend(frnd.getPublicKey());
    moveWidget(widget, frnd.getStatus());
    updateActivityTime(oldTime);  // update old category widget
}

void FriendListWidget::connectToCore(Core* core) {}

void FriendListWidget::setMode(SortingMode mode) {
    if (this->mode == mode) return;

    this->mode = mode;
    Settings::getInstance().setFriendSortingMode(mode);

    sortByMode(mode);
}

void FriendListWidget::sortByMode(SortingMode mode) {
    if (mode == SortingMode::Name) {
        int count = activityLayout ? activityLayout->count() : 0;

        listLayout->addLayout(listLayout->getLayoutOnline());
        listLayout->addLayout(listLayout->getLayoutOffline());
        //    listLayout->addLayout(circleLayout->getLayout());
        onGroupchatPositionChanged(groupsOnTop);

        if (activityLayout != nullptr) {
            QLayoutItem* item;
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
        const QMap<ok::base::ReadableTime, QString> names {
            { ok::base::ReadableTime::Today,     tr("Today",                      COMMENT) },
            { ok::base::ReadableTime::Yesterday, tr("Yesterday",                  COMMENT) },
            { ok::base::ReadableTime::ThisWeek,  tr("Last 7 days",                COMMENT) },
            { ok::base::ReadableTime::ThisMonth, tr("This month",                 COMMENT) },
            { ok::base::ReadableTime::LongAgo,   tr("Older than 6 Months",        COMMENT) },
            { ok::base::ReadableTime::Never,     tr("Never",                      COMMENT) },
            { ok::base::ReadableTime::Month1Ago, ql.monthName(today.addMonths(-1).month()) },
            { ok::base::ReadableTime::Month2Ago, ql.monthName(today.addMonths(-2).month()) },
            { ok::base::ReadableTime::Month3Ago, ql.monthName(today.addMonths(-3).month()) },
            { ok::base::ReadableTime::Month4Ago, ql.monthName(today.addMonths(-4).month()) },
            { ok::base::ReadableTime::Month5Ago, ql.monthName(today.addMonths(-5).month()) },
        };
// clang-format on
#undef COMMENT

        activityLayout = new QVBoxLayout();

        listLayout->removeItem(listLayout->getLayoutOnline());
        listLayout->removeItem(listLayout->getLayoutOffline());

        listLayout->insertLayout(1, activityLayout);

        reDraw();
    }
}

FriendListWidget::SortingMode FriendListWidget::getMode() const { return mode; }

GroupWidget* FriendListWidget::addGroup(const GroupId& groupId, const QString& groupName) {
    qDebug() << __func__ << groupId.toString();

    Group* g = GroupList::findGroup(groupId);
    if (g) {
        qWarning() << "Group already exist.";
        return groupWidgets.value(groupId.toString());
    }
    g = GroupList::addGroup(groupId, groupName);

    auto& settings = Settings::getInstance();

    //  const bool enabled = core->getGroupAvEnabled(groupId.toString());

    const auto compact = settings.getCompactLayout();
    auto gw = new GroupWidget(m_contentLayout, groupId.toString(), groupId, groupName, compact);
    groupWidgets[groupId.toString()] = gw;
    groupLayout.addSortedWidget(gw);

    connect(g, &Group::subjectChanged, [=, this](const QString& author, const QString& name) {
        Q_UNUSED(author);
        renameGroupWidget(gw, name);
    });

    connect(gw, &GroupWidget::chatroomWidgetClicked, this, &FriendListWidget::slot_groupClicked);
    connect(gw, &GroupWidget::removeGroup, this, &FriendListWidget::do_groupDeleted);

    return gw;
}

void FriendListWidget::do_groupDeleted(const ContactId& cid) {
    qDebug() << __func__ << cid.toString();
    removeGroup(GroupId(cid));
}

void FriendListWidget::removeGroup(const GroupId& cid) {
    qDebug() << __func__ << cid.toString();
    auto gw = groupWidgets.value(cid.toString());
    gw->deleteLater();
    groupWidgets.remove(cid.getId());
    groupLayout.removeSortedWidget(gw);
}

void FriendListWidget::setGroupTitle(const GroupId& groupId,
                                     const QString& author,
                                     const QString& title) {
    auto g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "group is no existing." << groupId.toString();
        return;
    }
    g->setSubject(author, title);
}

void FriendListWidget::setGroupInfo(const GroupId& groupId, const GroupInfo& info) {
    auto g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "group is no existing." << groupId.toString();
        return;
    }

    g->setName(info.name);
    g->setDesc(info.description);
    g->setSubject("", info.subject);
    g->setPeerCount(info.occupants);
}

void FriendListWidget::searchChatrooms(const QString& searchString, bool hideOnline,
                                       bool hideOffline, bool hideGroups) {
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

void FriendListWidget::renameGroupWidget(GroupWidget* groupWidget, const QString& newName) {
    //  groupLayout.removeSortedWidget(groupWidget);
    //  groupLayout.addSortedWidget(groupWidget);

    groupWidget->setName(newName);
}

void FriendListWidget::onGroupchatPositionChanged(bool top) {
    groupsOnTop = top;

    if (mode != SortingMode::Name) return;

    listLayout->removeItem(groupLayout.getLayout());

    if (top)
        listLayout->insertLayout(0, groupLayout.getLayout());
    else
        listLayout->insertLayout(1, groupLayout.getLayout());

    reDraw();
}

void FriendListWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (!event->mimeData()->hasFormat("toxPk")) {
        return;
    }
    FriendId toxPk(event->mimeData()->data("toxPk"));
    Friend* frnd = Nexus::getCore()->getFriendList().findFriend(toxPk);
    if (frnd) event->acceptProposedAction();
}

void FriendListWidget::dropEvent(QDropEvent* event) {
    // Check, that the element is dropped from qTox
    QObject* o = event->source();
    FriendWidget* widget = qobject_cast<FriendWidget*>(o);
    if (!widget) return;

    // Check, that the user has a friend with the same ToxPk
    assert(event->mimeData()->hasFormat("toxPk"));
    const FriendId toxPk{event->mimeData()->data("toxPk")};
    Friend* f = Nexus::getCore()->getFriendList().findFriend(toxPk);
    if (!f) return;

    // Save CircleWidget before changing the Id
    //  int circleId = Settings::getInstance().getFriendCircleID(f->getPublicKey());
    //  CircleWidget *circleWidget = CircleWidget::getFromID(circleId);

    moveWidget(widget, f->getStatus(), true);

    //  if (circleWidget)
    //    circleWidget->updateStatus();
}

void FriendListWidget::showEvent(QShowEvent* event) {
    //  auto core = Core::getInstance();
    //  connect(core, &Core::friendAdded, this, &FriendListWidget::addFriend);
    //  for (auto &receiver : core->loadFriendList()) {
    //    qDebug() << "receiver:" << receiver;
    //  };
}

void FriendListWidget::onCoreChanged(Core& core_) {
    core = &core_;
    connectToCore(core);
}

void FriendListWidget::dayTimeout() {
    if (mode == SortingMode::Activity) {
        setMode(SortingMode::Name);
        setMode(SortingMode::Activity);  // Refresh all.
    }

    dayTimer->start(ok::base::Times::timeUntilTomorrow());
}

void FriendListWidget::moveWidget(FriendWidget* widget, Status::Status s, bool add) {
    if (mode == SortingMode::Name) {
        const Friend* f = widget->getFriend();
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

void FriendListWidget::updateActivityTime(const QDateTime& time) {
    if (mode != SortingMode::Activity) return;

    int timeIndex = static_cast<int>(ok::base::getTimeBucket(time));
    //  QWidget *widget = activityLayout->itemAt(timeIndex)->widget();
    //  CategoryWidget *categoryWidget = static_cast<CategoryWidget *>(widget);
    //  categoryWidget->updateStatus();

    //  categoryWidget->setVisible(categoryWidget->hasChatrooms());
}

// update widget after add/delete/hide/show
void FriendListWidget::reDraw() {
    hide();
    show();
    resize(QSize());  // lifehack
}

QLayout* FriendListWidget::nextLayout(QLayout* layout, bool forward) const {
    if (layout == groupLayout.getLayout()) {
        if (forward) {
            if (groupsOnTop) return listLayout->getLayoutOnline();

            return listLayout->getLayoutOffline();
        } else {
            if (groupsOnTop)
                //        return circleLayout->getLayout();

                return listLayout->getLayoutOnline();
        }
    } else if (layout == listLayout->getLayoutOnline()) {
        if (forward) {
            if (groupsOnTop) return listLayout->getLayoutOffline();

            return groupLayout.getLayout();
        } else {
            if (groupsOnTop) return groupLayout.getLayout();

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

FriendWidget* FriendListWidget::getFriend(const ContactId& friendPk) {
    return friendWidgets.value(friendPk.toString());
}

void FriendListWidget::removeFriend(const FriendId& friendPk) {
    qDebug() << __func__ << friendPk.toString();

    auto fw = friendWidgets.value(friendPk.toString());
    if (!fw) {
        qWarning() << "Unable to find friendWidget";
        return;
    }
    listLayout->removeFriendWidget(fw);
    friendWidgets.remove(friendPk.toString());

    emit deleteFriendWidget(friendPk);

    auto frid = Nexus::getCore()->getFriendList().findFriend(friendPk);
    emit Widget::getInstance() -> friendRemoved(frid);
    delete fw;
}

GroupWidget* FriendListWidget::getGroup(const GroupId& id) {
    return groupWidgets.value(id.toString());
}

void FriendListWidget::slot_groupClicked(GenericChatroomWidget* actived) {
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

void FriendListWidget::slot_friendClicked(GenericChatroomWidget* actived) {
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

void FriendListWidget::setFriendStatus(const ContactId& friendPk, Status::Status status) {
    auto fw = getFriend(friendPk);
    if (!fw) {
        qWarning() << "friend" << friendPk.toString() << "widget is no existing.";
        return;
    }
    fw->setStatus(status, false);
}

void FriendListWidget::setFriendStatusMsg(const FriendId& friendPk, const QString& statusMsg) {
    auto fw = getFriend(friendPk);
    if (!fw) {
        qWarning() << "friend widget no exist.";
        return;
    }

    fw->setStatusMsg(statusMsg);
}

void FriendListWidget::setFriendName(const FriendId& friendPk, const QString& name) {
    qDebug() << __func__ << friendPk.toString() << name;
    auto f = Nexus::getCore()->getFriendList().findFriend(friendPk);
    if (!f) {
        qWarning() << "friend is no existing.";
        return;
    }
    f->setName(name);
}

void FriendListWidget::setFriendAlias(const FriendId& friendPk, const QString& alias) {
    qDebug() << __func__ << friendPk.toString() << alias;
    auto f = Nexus::getCore()->getFriendList().findFriend(friendPk);
    if (!f) {
        qWarning() << "friend is no existing.";
        return;
    }
    f->setAlias(alias);
}

void FriendListWidget::setFriendAvatar(const FriendId& friendPk, const QByteArray& avatar) {
    auto fw = getFriend(friendPk);
    if (!fw) {
        qWarning() << "friend is no exist.";
        return;
    }

    QPixmap p;
    p.loadFromData(avatar);
    fw->setAvatar(p);
}

void FriendListWidget::setFriendTyping(const FriendId& friendId, bool isTyping) {
    auto fw = getFriend(friendId);
    if (fw) fw->setTyping(isTyping);
}
void FriendListWidget::reloadTheme() {
    for (auto gw : groupWidgets) {
        gw->reloadTheme();
    }

    for (auto fw : friendWidgets) {
        fw->reloadTheme();
    }
}

void FriendListWidget::do_toShowDetails(const ContactId& cid) {
    qDebug() << __func__ << cid.toString();
    for (auto fw : friendWidgets) {
        if (fw->getContactId() == cid) {
            emit fw->chatroomWidgetClicked(fw);
            break;
        }
    }

    for (auto gw : groupWidgets) {
        if (gw->getContactId() == cid) {
            emit gw->chatroomWidgetClicked(gw);
            break;
        }
    }
}
