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

#include "ChatForwardDialog.h"

#include "ui_ChatForwardDialog.h"

#include "src/friendlist.h"
#include "src/widget/ContactListWidget.h"
#include "src/widget/ContactWidget.h"
#include "src/widget/friendwidget.h"
#include "src/widget/widget.h"

#include <src/nexus.h>

ChatForwardDialog::ChatForwardDialog(const MsgId& msgId, QWidget* parent)
        : QDialog(parent), ui(new Ui::ChatForwardDialog), msgId{msgId} {
    ui->setupUi(this);
    setWindowModality(Qt::WindowModality::ApplicationModal);
    setWindowTitle(tr("Forward message to"));
    setAttribute(Qt::WA_QuitOnClose);
    setFixedWidth(260);

    // 左侧朋友列表
    contactListWidget = new ContactListWidget(this, false);
    contactListWidget->setGeometry(0, 0, 400, 400);
    contactListWidget->layout()->setAlignment(Qt::AlignTop | Qt::AlignVCenter);

    connect(contactListWidget, &ContactListWidget::friendClicked, this,
            &ChatForwardDialog::onFriendClicked);

    auto w = Widget::getInstance();

    connect(w, &Widget::friendRemoved, this, &ChatForwardDialog::onFriendDeleted);

    ui->scrollAreaWidgetContents->layout()->addWidget(contactListWidget);
}

ChatForwardDialog::~ChatForwardDialog() { delete ui; }

void ChatForwardDialog::showEvent(QShowEvent* e) {
    auto& fl = Core::getInstance()->getFriendList();
    for (auto f : fl.getAllFriends()) {
        auto fw = contactListWidget->addFriend(f->getId());
        fw->setShowContextMenu(false);
    }
    QDialog::showEvent(e);
}

void ChatForwardDialog::onFriendClicked(FriendWidget* widget) {
    qDebug() << __func__ << widget;
    emit Widget::getInstance() -> forwardMessage(widget->getContactId(), msgId);
}

void ChatForwardDialog::onFriendDeleted(const Friend* widget) {
    qDebug() << __func__ << widget;
    contactListWidget->removeFriend(widget);
}
