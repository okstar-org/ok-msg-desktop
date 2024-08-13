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

#include "categorywidget.h"
#include <QBoxLayout>
#include <QMouseEvent>
#include "friendlistlayout.h"
#include "friendlistwidget.h"
#include "friendwidget.h"
#include "src/lib/settings/style.h"
#include "src/model/status.h"
#include "tool/croppinglabel.h"

#include <QApplication>

void CategoryWidget::emitChatroomWidget(QLayout* layout, int index) {
    QWidget* widget = layout->itemAt(index)->widget();
    GenericChatroomWidget* chatWidget = qobject_cast<GenericChatroomWidget*>(widget);
    if (chatWidget != nullptr) {
        emit chatWidget->chatroomWidgetClicked(chatWidget);
    }
}

CategoryWidget::CategoryWidget(bool compact, QWidget* parent)
        : GenericChatItemWidget(compact, parent) {
    container = new QWidget(this);
    container->setObjectName("circleWidgetContainer");
    container->setLayoutDirection(Qt::LeftToRight);

    statusLabel = new QLabel(this);
    statusLabel->setObjectName("status");
    statusLabel->setTextFormat(Qt::PlainText);

    statusPic.setPixmap(QPixmap(Style::getImagePath("chatArea/scrollBarRightArrow.svg")));

    fullLayout = new QVBoxLayout(this);
    fullLayout->setSpacing(0);
    fullLayout->setMargin(0);
    fullLayout->addWidget(container);

    lineFrame = new QFrame(container);
    lineFrame->setObjectName("line");
    lineFrame->setFrameShape(QFrame::HLine);
    lineFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    lineFrame->resize(0, 0);

    listLayout = new FriendListLayout();
    listWidget = new QWidget(this);
    listWidget->setLayout(listLayout);
    fullLayout->addWidget(listWidget);

    setAcceptDrops(true);

    onCompactChanged(isCompact());

    setExpanded(true, false);
    updateStatus();
}

bool CategoryWidget::isExpanded() const { return expanded; }

void CategoryWidget::setExpanded(bool isExpanded, bool save) {
    if (expanded == isExpanded) {
        return;
    }
    expanded = isExpanded;
    setMouseTracking(true);
    listWidget->setVisible(isExpanded);

    QString pixmapPath;
    if (isExpanded)
        pixmapPath = Style::getImagePath("chatArea/scrollBarDownArrow.svg");
    else
        pixmapPath = Style::getImagePath("chatArea/scrollBarRightArrow.svg");
    statusPic.setPixmap(QPixmap(pixmapPath));
    // The listWidget will recieve a enterEvent for some reason if now visible.
    // Using the following, we prevent that.
    QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
    container->hide();
    container->show();

    if (save) onExpand();
}

void CategoryWidget::leaveEvent(QEvent* event) { event->ignore(); }

void CategoryWidget::setName(const QString& name, bool save) {
    nameLabel->setText(name);

    if (isCompact()) nameLabel->minimizeMaximumWidth();

    if (save) onSetName();
}

void CategoryWidget::editName() {
    nameLabel->editBegin();
    nameLabel->setMaximumWidth(QWIDGETSIZE_MAX);
}

void CategoryWidget::addFriendWidget(FriendWidget* w, Status::Status s) {
    listLayout->addFriendWidget(w, s);
    updateStatus();
    onAddFriendWidget(w);
    w->reloadTheme();  // Otherwise theme will change when moving to another circle.
}

void CategoryWidget::removeFriendWidget(FriendWidget* w, Status::Status s) {
    listLayout->removeFriendWidget(w, s);
    updateStatus();
}

void CategoryWidget::updateStatus() {
    QString online = QString::number(listLayout->friendOnlineCount());
    QString offline = QString::number(listLayout->friendTotalCount());
    QString text = online + QStringLiteral(" / ") + offline;
    statusLabel->setText(text);
}

bool CategoryWidget::hasChatrooms() const { return listLayout->hasChatrooms(); }

void CategoryWidget::search(const QString& searchString, bool updateAll, bool hideOnline,
                            bool hideOffline) {
    if (updateAll) {
        listLayout->searchChatrooms(searchString, hideOnline, hideOffline);
    }
    bool inCategory = searchString.isEmpty() && !(hideOnline && hideOffline);
    setVisible(inCategory || listLayout->hasChatrooms());
}

bool CategoryWidget::cycleContacts(bool forward) {
    if (listLayout->friendTotalCount() == 0) {
        return false;
    }
    if (forward) {
        if (!listLayout->getLayoutOnline()->isEmpty()) {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOnline(), 0);
            return true;
        } else if (!listLayout->getLayoutOffline()->isEmpty()) {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOffline(), 0);
            return true;
        }
    } else {
        if (!listLayout->getLayoutOffline()->isEmpty()) {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOffline(),
                               listLayout->getLayoutOffline()->count() - 1);
            return true;
        } else if (!listLayout->getLayoutOnline()->isEmpty()) {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOnline(),
                               listLayout->getLayoutOnline()->count() - 1);
            return true;
        }
    }
    return false;
}

bool CategoryWidget::cycleContacts(FriendWidget* activeChatroomWidget, bool forward) {
    int index = -1;
    QLayout* currentLayout = nullptr;

    FriendWidget* friendWidget = qobject_cast<FriendWidget*>(activeChatroomWidget);
    if (friendWidget == nullptr) return false;

    currentLayout = listLayout->getLayoutOnline();
    index = listLayout->indexOfFriendWidget(friendWidget, true);
    if (index == -1) {
        currentLayout = listLayout->getLayoutOffline();
        index = listLayout->indexOfFriendWidget(friendWidget, false);
    }

    index += forward ? 1 : -1;
    for (;;) {
        // Bounds checking.
        if (index < 0) {
            if (currentLayout == listLayout->getLayoutOffline())
                currentLayout = listLayout->getLayoutOnline();
            else
                return false;

            index = currentLayout->count() - 1;
            continue;
        } else if (index >= currentLayout->count()) {
            if (currentLayout == listLayout->getLayoutOnline())
                currentLayout = listLayout->getLayoutOffline();
            else
                return false;

            index = 0;
            continue;
        }

        GenericChatroomWidget* chatWidget =
                qobject_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());
        if (chatWidget != nullptr) emit chatWidget->chatroomWidgetClicked(chatWidget);
        return true;
    }

    return false;
}

void CategoryWidget::onCompactChanged(bool _compact) {
    delete topLayout;
    delete mainLayout;

    topLayout = new QHBoxLayout;
    topLayout->setSpacing(0);
    topLayout->setMargin(0);

    Q_UNUSED(_compact);
    setCompact(true);

    nameLabel->minimizeMaximumWidth();

    mainLayout = nullptr;

    container->setFixedHeight(25);
    container->setLayout(topLayout);

    topLayout->addSpacing(18);
    topLayout->addWidget(&statusPic);
    topLayout->addSpacing(5);
    topLayout->addWidget(nameLabel, 100);
    topLayout->addWidget(lineFrame, 1);
    topLayout->addSpacing(5);
    topLayout->addWidget(statusLabel);
    topLayout->addSpacing(5);
    topLayout->activate();

    Style::repolish(this);
}

void CategoryWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) setExpanded(!expanded);
}

void CategoryWidget::setContainerAttribute(Qt::WidgetAttribute attribute, bool enabled) {
    container->setAttribute(attribute, enabled);
    Style::repolish(container);
}

QLayout* CategoryWidget::friendOfflineLayout() const { return listLayout->getLayoutOffline(); }

QLayout* CategoryWidget::friendOnlineLayout() const { return listLayout->getLayoutOnline(); }

void CategoryWidget::moveFriendWidgets(FriendListWidget* friendList) {
    listLayout->moveFriendWidgets(friendList);
}
