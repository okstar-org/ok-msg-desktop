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

#include "ContactSelectDialog.h"

#include "ui_ContactSelectDialog.h"

#include "src/model/friendlist.h"
#include "src/widget/ContactListWidget.h"
#include "src/widget/ContactWidget.h"
#include "src/widget/friendwidget.h"
#include "src/widget/widget.h"

#include <src/nexus.h>

ContactSelectDialog::ContactSelectDialog(QWidget* parent, const QString& title)
        : QDialog(parent), ui(new Ui::ContactSelectDialog) {
    ui->setupUi(this);
    setWindowModality(Qt::WindowModality::ApplicationModal);
    if (title.isEmpty())
        setWindowTitle(tr("Please select contact"));
    else
        setWindowTitle(title);

    setAttribute(Qt::WA_QuitOnClose);
    setFixedWidth(260);

    // 左侧朋友列表
    contactListWidget = new ContactListWidget(this, false);
    contactListWidget->setGeometry(0, 0, 400, 400);
    contactListWidget->layout()->setAlignment(Qt::AlignTop | Qt::AlignVCenter);

    connect(contactListWidget, &ContactListWidget::friendClicked, this,
            &ContactSelectDialog::onFriendClicked);

    auto w = Widget::getInstance();

    connect(w, &Widget::friendRemoved, this, &ContactSelectDialog::onFriendDeleted);

    ui->scrollAreaWidgetContents->layout()->addWidget(contactListWidget);
}

ContactSelectDialog::~ContactSelectDialog() { delete ui; }

void ContactSelectDialog::showEvent(QShowEvent* e) {
    auto& fl = Core::getInstance()->getFriendList();
    for (auto f : fl.getAllFriends()) {
        auto fw = contactListWidget->addFriend(f->getId());
        fw->setShowContextMenu(false);
    }
    QDialog::showEvent(e);
}

void ContactSelectDialog::onFriendClicked(FriendWidget* widget) {
    qDebug() << __func__ << widget;
    emit contactClicked(widget->getContactId());
    close();
}

void ContactSelectDialog::onFriendDeleted(const Friend* widget) {
    qDebug() << __func__ << widget;
    contactListWidget->removeFriend(widget);
}
