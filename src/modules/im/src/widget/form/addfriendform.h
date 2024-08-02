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

#ifndef ADDFRIENDFORM_H
#define ADDFRIENDFORM_H

#include "src/core/toxid.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include <lib/session/AuthSession.h>

class ContentLayout;

namespace Ui {
class AddFriendForm;
}

namespace ok::backend {
class UserService;
struct OrgStaff;
}  // namespace ok::backend

namespace ok::session {
class SignInInfo;
}

class AddFriendForm : public QWidget {
    Q_OBJECT
public:
    AddFriendForm(QWidget* parent);
    ~AddFriendForm();

    bool isShown() const;
    void showTo(ContentLayout* contentLayout);

    bool addFriendRequest(const QString& friendAddress, const QString& message);

signals:
    void friendRequested(const FriendId& friendAddress, const QString& nick,
                         const QString& message);
    void friendRequestAccepted(const FriendId& friendAddress);
    void friendRequestRejected(const FriendId& friendAddress);
    void friendRequestsSeen();
    void friendReceipts(const QList<ok::backend::OrgStaff*>& qList);

public slots:
    void onUsernameSet(const QString& userName);

protected:
    virtual void showEvent(QShowEvent* e) override;

private slots:
    void onSearchTriggered();
    void onCurrentChanged(int index);
    void onFriendReceipts(const QList<ok::backend::OrgStaff*>& qList);

private:
    void searchFriend(const QString& idText);
    void addFriend(const QString& idText, const QString& nick);
    void retranslateUi();

    void setIdFromClipboard();
    QString getMessage();

    Ui::AddFriendForm* addUi;

    QLabel messageLabel;

    QVBoxLayout* friendLayout;

    QPushButton searchButton;
    QScrollArea friendArea;

    QWidget* main;

    QString lastUsername;
    QTabWidget* tabWidget;
    QVBoxLayout* requestsLayout;
    QList<QPushButton*> acceptButtons;
    QList<QPushButton*> rejectButtons;

    const ok::session::SignInInfo* signIn;
    std::unique_ptr<ok::backend::UserService> userService;
};

#endif  // ADDFRIENDFORM_H
