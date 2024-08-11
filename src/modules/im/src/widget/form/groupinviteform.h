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

#ifndef GROUPINVITEFORM_H
#define GROUPINVITEFORM_H

#include "src/model/message.h"
#include "src/widget/gui.h"

#include <QScrollArea>
#include <QWidget>

class ContentLayout;
class GroupInvite;
class GroupInviteWidget;

class QGroupBox;
class QLabel;
class QPushButton;
class QScrollArea;
class QSignalMapper;

namespace Ui {
class MainWindow;
}

class GroupInviteForm : public QWidget {
    Q_OBJECT
public:
    GroupInviteForm();
    ~GroupInviteForm();

    void show(ContentLayout* contentLayout);
    bool addGroupInvite(const GroupInvite& inviteInfo);
    bool isShown() const;

signals:
    void groupCreate(ConferenceType type);
    void groupInviteAccepted(const GroupInvite& inviteInfo);
    void groupInvitesSeen();

protected:
    void showEvent(QShowEvent* event) final override;

private:
    void retranslateUi();
    void deleteInviteWidget(const GroupInvite& inviteInfo);

private:
    QWidget* headWidget;
    QLabel* headLabel;
    QPushButton* createButton;
    QGroupBox* inviteBox;
    QList<GroupInviteWidget*> invites;
    QScrollArea* scroll;
};

#endif  // GROUPINVITEFORM_H
