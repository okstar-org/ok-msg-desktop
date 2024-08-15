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

#include <QBoxLayout>
#include <QDragEnterEvent>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QVariant>

#include <cassert>

#include "circlewidget.h"
#include "contentdialog.h"
#include "friendlistwidget.h"
#include "friendwidget.h"
#include "tool/croppinglabel.h"
#include "widget.h"

#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/persistence/settings.h"
#include "src/widget/form/chatform.h"

QHash<int, CircleWidget*> CircleWidget::circleList;

CircleWidget::CircleWidget(FriendListWidget* parent, int id) : CategoryWidget(parent), id(id) {
    setName(Settings::getInstance().getCircleName(id), false);
    circleList[id] = this;

    connect(nameLabel, &CroppingLabel::editFinished, [this](const QString& newName) {
        if (!newName.isEmpty()) emit renameRequested(this, newName);
    });

    connect(nameLabel, &CroppingLabel::editRemoved, [this]() {
        if (isCompact()) nameLabel->minimizeMaximumWidth();
    });

    setExpanded(Settings::getInstance().getCircleExpanded(id), false);
    updateStatus();
}

CircleWidget::~CircleWidget() {
    if (circleList[id] == this) circleList.remove(id);
}

void CircleWidget::editName() { CategoryWidget::editName(); }

CircleWidget* CircleWidget::getFromID(int id) {
    auto circleIt = circleList.find(id);

    if (circleIt != circleList.end()) return circleIt.value();

    return nullptr;
}

void CircleWidget::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu;
    QAction* renameAction = menu.addAction(tr("Rename circle", "Menu for renaming a circle"));
    QAction* removeAction = menu.addAction(tr("Remove circle", "Menu for removing a circle"));
    QAction* openAction = nullptr;

    if (friendOfflineLayout()->count() + friendOnlineLayout()->count() > 0)
        openAction = menu.addAction(tr("Open all in new window"));

    QAction* selectedItem = menu.exec(mapToGlobal(event->pos()));

    if (selectedItem) {
        if (selectedItem == renameAction) {
            editName();
        } else if (selectedItem == removeAction) {
            FriendListWidget* friendList = static_cast<FriendListWidget*>(parentWidget());
            moveFriendWidgets(friendList);

            friendList->removeCircleWidget(this);

            int replacedCircle = Settings::getInstance().removeCircle(id);

            auto circleReplace = circleList.find(replacedCircle);
            if (circleReplace != circleList.end())
                circleReplace.value()->updateID(id);
            else
                assert(true);  // This should never happen.

            circleList.remove(replacedCircle);
        } else if (selectedItem == openAction) {
            ContentDialog* dialog = new ContentDialog();
            emit newContentDialog(*dialog);
            for (int i = 0; i < friendOnlineLayout()->count(); ++i) {
                QWidget* const widget = friendOnlineLayout()->itemAt(i)->widget();
                FriendWidget* const friendWidget = qobject_cast<FriendWidget*>(widget);

                if (friendWidget != nullptr) {
                    friendWidget->activate();
                }
            }
            for (int i = 0; i < friendOfflineLayout()->count(); ++i) {
                QWidget* const widget = friendOfflineLayout()->itemAt(i)->widget();
                FriendWidget* const friendWidget = qobject_cast<FriendWidget*>(widget);

                if (friendWidget != nullptr) {
                    friendWidget->activate();
                }
            }

            dialog->show();
            dialog->ensureSplitterVisible();
        }
    }

    setContainerAttribute(Qt::WA_UnderMouse, false);
}

void CircleWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (!event->mimeData()->hasFormat("toxPk")) {
        return;
    }
    ToxPk toxPk(event->mimeData()->data("toxPk"));
    Friend* f = Nexus::getCore()->getFriendList().findFriend(toxPk);
    if (f != nullptr) event->acceptProposedAction();

    setContainerAttribute(Qt::WA_UnderMouse, true);  // Simulate hover.
}

void CircleWidget::dragLeaveEvent(QDragLeaveEvent*) {
    setContainerAttribute(Qt::WA_UnderMouse, false);
}

void CircleWidget::dropEvent(QDropEvent* event) {
    setExpanded(true, false);

    // Check, that the element is dropped from qTox
    QObject* o = event->source();
    FriendWidget* widget = qobject_cast<FriendWidget*>(o);
    if (!widget) return;

    if (!event->mimeData()->hasFormat("toxPk")) {
        return;
    }
    // Check, that the user has a friend with the same ToxId
    ToxPk toxPk{event->mimeData()->data("toxPk")};
    Friend* f = Nexus::getCore()->getFriendList().findFriend(toxPk);
    if (!f) return;

    // Save CircleWidget before changing the Id
    int circleId = Settings::getInstance().getFriendCircleID(toxPk);
    CircleWidget* circleWidget = getFromID(circleId);

    addFriendWidget(widget, f->getStatus());
    Settings::getInstance().savePersonal();

    if (circleWidget != nullptr) {
        circleWidget->updateStatus();
        emit searchCircle(*circleWidget);
    }

    setContainerAttribute(Qt::WA_UnderMouse, false);
}

void CircleWidget::onSetName() { Settings::getInstance().setCircleName(id, getName()); }

void CircleWidget::onExpand() {
    Settings::getInstance().setCircleExpanded(id, isExpanded());
    Settings::getInstance().savePersonal();
}

void CircleWidget::onAddFriendWidget(FriendWidget* w) {
    const Friend* f = w->getFriend();
    ToxPk toxId = f->getPublicKey();
    Settings::getInstance().setFriendCircleID(toxId, id);
}

void CircleWidget::updateID(int index) {
    // For when a circle gets destroyed, another takes its id.
    // This function updates all friends widgets for this new id.

    if (id == index) {
        return;
    }

    id = index;
    circleList[id] = this;

    for (int i = 0; i < friendOnlineLayout()->count(); ++i) {
        const QWidget* w = friendOnlineLayout()->itemAt(i)->widget();
        const FriendWidget* friendWidget = qobject_cast<const FriendWidget*>(w);

        if (friendWidget) {
            const Friend* f = friendWidget->getFriend();
            Settings::getInstance().setFriendCircleID(f->getPublicKey(), id);
        }
    }

    for (int i = 0; i < friendOfflineLayout()->count(); ++i) {
        const QWidget* w = friendOfflineLayout()->itemAt(i)->widget();
        const FriendWidget* friendWidget = qobject_cast<const FriendWidget*>(w);

        if (friendWidget) {
            const Friend* f = friendWidget->getFriend();
            Settings::getInstance().setFriendCircleID(f->getPublicKey(), id);
        }
    }
}
