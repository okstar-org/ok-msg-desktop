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

#include "MessageSessionListWidget.h"
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QGridLayout>
#include <QMimeData>
#include <QTimer>
#include <cassert>
#include "Bus.h"
#include "ChatWidget.h"
#include "ContactListLayout.h"
#include "application.h"
#include "base/times.h"
#include "circlewidget.h"
#include "contentdialogmanager.h"
#include "friendwidget.h"
#include "groupwidget.h"

#include "src/model/status.h"
#include "src/nexus.h"
#include "widget.h"
#include "Bus.h"

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

inline QDateTime getActiveTimeFriend(const Friend* contact) {
    return Nexus::getProfile()->getSettings()->getFriendActivity(contact->getPublicKey());
}

MessageSessionListWidget::MessageSessionListWidget(MainLayout* parent,
                                                   ContentLayout* contentBox,
                                                   bool groupsOnTop)
        : QWidget(parent), m_contentLayout(contentBox), groupsOnTop(groupsOnTop) {
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setAcceptDrops(true);

    // Prevent QLayout's add child warning before setting the mode.
    listLayout = new ContactListLayout(this);
    setLayout(listLayout);

    //    mode = Nexus::getProfile()->getSettings()->getFriendSortingMode();
    //    sortByMode(mode);

    auto w = Widget::getInstance();
    connect(w, &Widget::toDeleteChat, this, &MessageSessionListWidget::do_deleteSession);
    connect(w, &Widget::toClearHistory, this, &MessageSessionListWidget::do_clearHistory);

    auto bus = ok::Application::Instance()->bus();
    connect(bus, &ok::Bus::profileChanged, [this](Profile* profile) {
        auto settings = profile->getSettings();
        connect(settings, &Settings::compactLayoutChanged, this,
                &MessageSessionListWidget::onCompactChanged);
    });
}

MessageSessionListWidget::~MessageSessionListWidget() {
    auto w = Widget::getInstance();
    disconnect(w, &Widget::toDeleteChat, this, &MessageSessionListWidget::do_deleteSession);
    disconnect(w, &Widget::toClearHistory, this, &MessageSessionListWidget::do_clearHistory);

    if (activityLayout != nullptr) {
        QLayoutItem* item;
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

MessageSessionWidget* MessageSessionListWidget::createMessageSession(const ContactId& contactId,
                                                                     const QString& sid,
                                                                     ChatType type) {
    qDebug() << __func__ << "contactId:" << contactId.toString();

    auto sw = getMessageSession(contactId.toString());
    if (sw) {
        qWarning() << "Message session is existing.";
        return sw;
    }

    sw = new MessageSessionWidget(m_contentLayout, contactId, type);
    qDebug() << "create session for:" << contactId.toString() << "=>" << sw;
    connectSessionWidget(*sw);

    listLayout->addWidget(sw);
    sessionWidgets.insert(sw->getContactId().toString(), sw);

    emit sessionAdded(sw);
    return sw;
}

void MessageSessionListWidget::connectSessionWidget(MessageSessionWidget& sw) {
    connect(&sw, &MessageSessionWidget::updateFriendActivity, this,
            &MessageSessionListWidget::updateFriendActivity);
    connect(&sw, &MessageSessionWidget::widgetClicked, this,
            &MessageSessionListWidget::slot_sessionClicked);
    connect(&sw, &MessageSessionWidget::deleteSession, this,
            &MessageSessionListWidget::do_deleteSession);
}

void MessageSessionListWidget::updateFriendActivity(const Friend& frnd) {
    const FriendId& pk = frnd.getPublicKey();
    auto settings = Nexus::getProfile()->getSettings();
    const auto oldTime = settings->getFriendActivity(pk);
    const auto newTime = QDateTime::currentDateTime();
    settings->setFriendActivity(pk, newTime);
    MessageSessionWidget* widget = getMessageSession(frnd.getPublicKey().toString());
    moveWidget(widget, frnd.getStatus());
    updateActivityTime(oldTime);  // update old category widget
}

void MessageSessionListWidget::setMode(SortingMode mode) {
    if (this->mode == mode) return;

    this->mode = mode;
    Nexus::getProfile()->getSettings()->setFriendSortingMode(mode);

    sortByMode(mode);
}

void MessageSessionListWidget::sortByMode(SortingMode mode) {
    if (mode == SortingMode::Name) {
        //    circleLayout = new GenericChatItemLayout;
        //    circleLayout->getLayout()->setSpacing(0);
        //    circleLayout->getLayout()->setMargin(0);

        //    for (int i = 0; i < Nexus::getProfile()->getSettings()->getCircleCount(); ++i) {
        //      addCircleWidget(i);
        //      CircleWidget::getFromID(i)->setVisible(false);
        //    }

        // Only display circles once all created to avoid artifacts.
        //    for (int i = 0; i < Nexus::getProfile()->getSettings()->getCircleCount(); ++i)
        //      CircleWidget::getFromID(i)->setVisible(true);

        int count = activityLayout ? activityLayout->count() : 0;
        for (int i = 0; i < count; i++) {
            QWidget* widget = activityLayout->itemAt(i)->widget();
            //      CategoryWidget *categoryWidget = qobject_cast<CategoryWidget *>(widget);
            //      if (categoryWidget) {
            //        categoryWidget->moveFriendWidgets(this);
            //      } else {
            //        qWarning() << "Unexpected widget";
            //      }
        }

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
        QLocale ql(Nexus::getProfile()->getSettings()->getTranslation());
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
        //    bool compact = Nexus::getProfile()->getSettings()->getCompactLayout();
        //    for (Time t : names.keys()) {
        //      CategoryWidget *category = new CategoryWidget(compact, this);
        //      category->setName(names[t]);
        //      activityLayout->addWidget(category);
        //    }

        moveFriends(listLayout->getLayoutOnline());
        //    if (circleLayout != nullptr) {
        //      moveFriends(circleLayout->getLayout());
        //    }

        for (int i = 0; i < activityLayout->count(); ++i) {
            QWidget* widget = activityLayout->itemAt(i)->widget();
            //      CategoryWidget *categoryWidget = qobject_cast<CategoryWidget *>(widget);
            //      categoryWidget->setVisible(categoryWidget->hasChatrooms());
        }

        listLayout->removeItem(listLayout->getLayoutOnline());

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

void MessageSessionListWidget::moveFriends(QLayout* layout) {
    while (!layout->isEmpty()) {
        QWidget* widget = layout->itemAt(0)->widget();
        FriendWidget* friendWidget = qobject_cast<FriendWidget*>(widget);
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

MessageSessionListWidget::SortingMode MessageSessionListWidget::getMode() const {
    return mode;
}

void MessageSessionListWidget::search(const QString& searchString) {
    listLayout->search(searchString);
}

void MessageSessionListWidget::onFriendWidgetRenamed(FriendWidget* friendWidget) {
    //    const Friend* contact = friendWidget->getFriend();
    //    auto status = contact->getStatus();
    //    listLayout->removeFriendWidget(friendWidget);
    //    listLayout->addFriendWidget(friendWidget, status);
}

void MessageSessionListWidget::cycleContacts(GenericChatroomWidget* activeChatroomWidget,
                                             bool forward) {
    if (!activeChatroomWidget) {
        return;
    }

    int index = -1;
    FriendWidget* friendWidget = qobject_cast<FriendWidget*>(activeChatroomWidget);

    if (mode == SortingMode::Activity) {
        if (!friendWidget) {
            return;
        }

        const auto activityTime = getActiveTimeFriend(friendWidget->getFriend());
        index = static_cast<int>(ok::base::getTimeBucket(activityTime));
        QWidget* widget = activityLayout->itemAt(index)->widget();
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

            auto* widget = activityLayout->itemAt(index)->widget();
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

    QLayout* currentLayout = nullptr;
    //  CircleWidget *circleWidget = nullptr;

    if (friendWidget != nullptr) {
        const FriendId& pk = friendWidget->getFriend()->getPublicKey();
        uint32_t circleId = Nexus::getProfile()->getSettings()->getFriendCircleID(pk);
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
        if (currentLayout == listLayout->getLayoutOnline()) {
            GenericChatroomWidget* chatWidget =
                    qobject_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());

            if (chatWidget != nullptr) emit chatWidget->chatroomWidgetClicked(chatWidget);

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

void MessageSessionListWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (!event->mimeData()->hasFormat("toxPk")) {
        return;
    }
    FriendId toxPk(event->mimeData()->data("toxPk"));
    Friend* frnd = Nexus::getCore()->getFriendList().findFriend(toxPk);
    if (frnd) event->acceptProposedAction();
}

void MessageSessionListWidget::dropEvent(QDropEvent* event) {
    // Check, that the element is dropped from OkMsg
    QObject* o = event->source();
    auto widget = qobject_cast<MessageSessionWidget*>(o);
    if (!widget) return;

    // Check, that the user has a friend with the same ToxPk
    assert(event->mimeData()->hasFormat("toxPk"));
    const FriendId toxPk{event->mimeData()->data("toxPk")};
    Friend* f = Nexus::getCore()->getFriendList().findFriend(toxPk);
    if (!f) return;

    // Save CircleWidget before changing the Id
    //  int circleId = Nexus::getProfile()->getSettings()->getFriendCircleID(f->getPublicKey());
    //  CircleWidget *circleWidget = CircleWidget::getFromID(circleId);

    moveWidget(widget, f->getStatus(), true);

    //  if (circleWidget)
    //    circleWidget->updateStatus();
}

void MessageSessionListWidget::showEvent(QShowEvent* event) {
    //  auto core = Core::getInstance();
    //  connect(core, &Core::friendAdded, this, &MessageSessionListWidget::addFriend);
    //  for (auto &receiver : core->loadFriendList()) {
    //    qDebug() << "receiver:" << receiver;
    //  };
}

void MessageSessionListWidget::dayTimeout() {
    if (mode == SortingMode::Activity) {
        setMode(SortingMode::Name);
        setMode(SortingMode::Activity);  // Refresh all.
    }

    // dayTimer->start(ok::base::Times::timeUntilTomorrow());
}

void MessageSessionListWidget::moveWidget(MessageSessionWidget* widget, Status::Status s,
                                          bool add) {
    //  if (mode == SortingMode::Name) {
    //    const IMFriend *f = widget->getFriend();
    //    int circleId = Nexus::getProfile()->getSettings()->getFriendCircleID(f->getPublicKey());
    //    CircleWidget *circleWidget = CircleWidget::getFromID(circleId);

    //    if (circleWidget == nullptr || add) {
    //      if (circleId != -1)
    //        Nexus::getProfile()->getSettings()->setFriendCircleID(f->getPublicKey(), -1);
    //      listLayout->addWidget(widget);
    //      return;
    //    }

    //    circleWidget->addFriendWidget(widget, s);
    //  } else {
    //    const IMFriend *contact = widget->getFriend();
    //    auto *categoryWidget = getTimeCategoryWidget(contact);
    //    categoryWidget->addFriendWidget(widget, contact->getStatus());
    //    categoryWidget->show();
    //  }
}

void MessageSessionListWidget::updateActivityTime(const QDateTime& time) {
    if (mode != SortingMode::Activity) return;

    //  int timeIndex = static_cast<int>(base::getTimeBucket(time));
    //  QWidget *widget = activityLayout->itemAt(timeIndex)->widget();
    //  CategoryWidget *categoryWidget = static_cast<CategoryWidget *>(widget);
    //  categoryWidget->updateStatus();
    //  categoryWidget->setVisible(categoryWidget->hasChatrooms());
}

// update widget after add/delete/hide/show
void MessageSessionListWidget::reDraw() {
    hide();
    show();
    resize(QSize());  // lifehack
}

void MessageSessionListWidget::setRecvGroupMessage(const GroupId& groupId,
                                                   const GroupMessage& msg) {
    auto ms = getMessageSession(groupId.toString());
    if (!ms) {
        ms = createMessageSession(groupId, "", ChatType::GroupChat);
    }

    ms->setRecvGroupMessage(msg);
}

void MessageSessionListWidget::toSendMessage(const ContactId& pk, bool isGroup) {
    qDebug() << __func__ << pk.toString();
    auto w = sessionWidgets.value(pk.toString());
    if (!w) {
        qDebug() << "Create session for" << pk.toString();
        w = createMessageSession(pk, "", isGroup ? ChatType::GroupChat : ChatType::Chat);
    }
    emit w->chatroomWidgetClicked(w);
}

void MessageSessionListWidget::toForwardMessage(const ContactId& pk, const MsgId& id) {
    qDebug() << __func__ << pk.toString();
    auto w = sessionWidgets.value(pk.toString());
    if (!w) {
        qDebug() << "Create session for" << pk.toString();
        w = createMessageSession(pk, "", pk.isGroup() ? ChatType::GroupChat : ChatType::Chat);
    }

    w->doForwardMessage(pk, id);
}

void MessageSessionListWidget::setFriendAvInvite(const ToxPeer& peerId, bool video) {
    auto friendId = peerId.toFriendId();
    auto w = sessionWidgets.value(friendId.toString());
    if (!w) {
        qDebug() << "Create session for friend" << friendId;
        w = createMessageSession(friendId, "", ChatType::Chat);
    }
    w->setAvInvite(peerId, video);
}

void MessageSessionListWidget::setFriendAvStart(const FriendId& friendId, bool video) {
    auto w = sessionWidgets.value(friendId.toString());
    if (!w) {
        qWarning() << "The message session is no existing!";
        return;
    }
    w->setAvStart(video);
}

void MessageSessionListWidget::setFriendAvPeerConnectedState(const FriendId& friendId,
                                                             lib::ortc::PeerConnectionState state) {
    auto w = sessionWidgets.value(friendId.toString());
    if (!w) {
        qWarning() << "The message session is no existing!";
        return;
    }

    w->setAvPeerConnectedState(state);
}

void MessageSessionListWidget::setFriendAvEnd(const FriendId& friendId, bool error) {
    auto w = sessionWidgets.value(friendId.toString());
    if (!w) {
        qWarning() << "The message session is no existing!";
        return;
    }
    w->setAvEnd(error);
}

void MessageSessionListWidget::addGroup(const Group* g) {
    auto ms = getMessageSession(g->getIdAsString());
    if (!ms) {
        qWarning() << "Unable to find message session" << g->getIdAsString();
        return;
    }
    ms->setGroup(g);
}

void MessageSessionListWidget::removeGroup(const Group* g) {
    auto ms = getMessageSession(g->getIdAsString());
    if (!ms) {
        qWarning() << "Unable to find message session" << g->getIdAsString();
        return;
    }
    ms->removeGroup();
}

void MessageSessionListWidget::clearAllReceipts() {
    for (auto sw : sessionWidgets) {
        sw->clearReceipts();
    }
}

QLayout* MessageSessionListWidget::nextLayout(QLayout* layout, bool forward) const {
    qDebug() << "nextLayout:" << layout << "forward:" << forward;
    if (forward) {
        if (groupsOnTop) return listLayout->getLayoutOnline();

    } else {
        if (groupsOnTop) return listLayout->getLayoutOnline();
    }

    return nullptr;
}

MessageSessionWidget* MessageSessionListWidget::getMessageSession(const QString& contactId) {
    return sessionWidgets.value(contactId);
}

void MessageSessionListWidget::addFriend(const Friend* f) {
    auto ms = getMessageSession(f->getId().toString());
    if (!ms) {
        qWarning() << "Unable to find message session" << f->getId();
        return;
    }
    ms->setFriend(f);
}

void MessageSessionListWidget::removeFriend(const Friend* f) {
    auto ms = getMessageSession(f->getId().toString());
    if (!ms) {
        qWarning() << "Unable to find message session" << f->getId();
        return;
    }
    ms->removeFriend();
}

void MessageSessionListWidget::slot_sessionClicked(MessageSessionWidget* actived) {
    for (auto w : sessionWidgets) {
        if (w != actived) {
            w->setActive(false);
        } else {
            w->setActive(true);
        }
    }
}

void MessageSessionListWidget::do_deleteSession(const QString& cid) {
    qDebug() << __func__ << cid;
    auto w = sessionWidgets.value(cid);
    if (w) {
        qDebug() << "delete" << w;
        sessionWidgets.remove(cid);
        w->deleteLater();
    }
}

void MessageSessionListWidget::do_clearHistory(const QString& cid) {
    qDebug() << __func__ << cid;
    auto ms = getMessageSession(cid);
    if (ms) {
        ms->clearHistory();
    }
}

void MessageSessionListWidget::setRecvFriendMessage(FriendId friendnumber,
                                                    const FriendMessage& message, bool isAction) {
    auto fw = getMessageSession(friendnumber.toString());
    if (!fw) {
        /**
         * 陌生人消息（可能是已经将对方删除，通讯录没有对方记录）
         */
        qWarning() << "Can not find friend:" << friendnumber.toString()
                   << ", so add it to contact.";
        fw = createMessageSession(friendnumber, message.id, ChatType::Chat);
    }

    fw->setRecvMessage(message, isAction);
}

void MessageSessionListWidget::setFriendMessageReceipt(const FriendId& friendId,
                                                       const MsgId& msgId) {
    auto fw = getMessageSession(friendId.toString());
    if (!fw) {
        return;
    }
    fw->setMessageReceipt(msgId);
}

void MessageSessionListWidget::setFriendStatus(const FriendId& friendPk, Status::Status status) {
    auto fw = getMessageSession(friendPk.toString());
    if (!fw) {
        qWarning() << "friend widget is no existing.";
        return;
    }
    fw->setStatus(status, false);
}

void MessageSessionListWidget::setFriendStatusMsg(const FriendId& friendPk,
                                                  const QString& statusMsg) {
    auto fw = getMessageSession(friendPk.toString());
    if (!fw) {
        qWarning() << "friend widget no exist.";
        return;
    }
    fw->setStatusMsg(statusMsg);
}

void MessageSessionListWidget::setFriendName(const FriendId& friendPk, const QString& name) {
    auto fw = getMessageSession(friendPk.toString());
    if (!fw) {
        qWarning() << "friend is no exist.";
        return;
    }
    fw->setName(name);
}

void MessageSessionListWidget::setFriendAvatar(const FriendId& friendPk, const QByteArray& avatar) {
    auto fw = getMessageSession(friendPk.toString());
    if (!fw) {
        qWarning() << "message session is no exist.";
        return;
    }

    QPixmap p;
    p.loadFromData(avatar);
    fw->setAvatar(p);
}

void MessageSessionListWidget::setFriendTyping(const ContactId& f, bool isTyping) {
    auto fw = getMessageSession(f.toString());
    if (fw) fw->setTyping(isTyping);
}

void MessageSessionListWidget::setFriendFileReceived(const ContactId& f, const ToxFile& file) {
    auto ms = getMessageSession(f.toString());
    if (ms) {
        ms->setFileReceived(file);
    }
}

void MessageSessionListWidget::setFriendFileCancelled(const ContactId& f, const QString& fileId) {
    auto ms = getMessageSession(f.toString());
    if (ms) {
        ms->setFileCancelled(fileId);
    }
}

void MessageSessionListWidget::reloadTheme() {
    auto p = palette();
    p.setColor(QPalette::Window,
               lib::settings::Style::getColor(
                       lib::settings::Style::ColorPalette::ThemeMedium));  // Base background color
    p.setColor(QPalette::Highlight,
               lib::settings::Style::getColor(
                       lib::settings::Style::ColorPalette::ThemeHighlight));  // On mouse over
    p.setColor(QPalette::Light,
               lib::settings::Style::getColor(
                       lib::settings::Style::ColorPalette::ThemeLight));  // When active
    setPalette(p);

    for (auto fw : sessionWidgets) {
        fw->reloadTheme();
    }
}
