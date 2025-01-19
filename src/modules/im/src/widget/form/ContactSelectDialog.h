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

#ifndef CHATFORWARDIALOG_H
#define CHATFORWARDIALOG_H

#include <QDialog>
#include "src/model/contactid.h"

namespace Ui {
class ContactSelectDialog;
}

namespace module::im {

class Friend;
class FriendWidget;
class ContactListWidget;

class ContactSelectDialog : public QDialog {
    Q_OBJECT

public:
    explicit ContactSelectDialog(QWidget* parent = nullptr, const QString& title = "");
    ~ContactSelectDialog() override;

protected:
    void showEvent(QShowEvent* e) override;

private:
    Ui::ContactSelectDialog* ui;

    ContactListWidget* contactListWidget;

signals:
    void contactClicked(const ContactId& id);

public slots:
    void onFriendClicked(FriendWidget* widget);
    void onFriendDeleted(const Friend* f);
};
}  // namespace module::im
#endif  // CHATFORWARDIALOG_H
