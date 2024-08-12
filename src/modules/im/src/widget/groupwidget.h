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

#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include "genericchatroomwidget.h"

#include "src/core/groupid.h"
#include "src/model/chatroom/groupchatroom.h"
#include "src/model/message.h"

#include <src/widget/form/aboutgroupform.h>
#include <memory>
#include "ContentWidget.h"
#include "contentdialog.h"
#include "form/groupchatform.h"
#include "src/model/groupmessagedispatcher.h"
#include "src/model/sessionchatlog.h"

class GroupWidget final : public GenericChatroomWidget {
    Q_OBJECT

public:
    GroupWidget(ContentLayout* layout,
                QString groupnumber,
                const GroupId& groupId,
                const QString& groupName,
                bool compact);

    ~GroupWidget();
    void init();

    void setAsInactiveChatroom() final override;
    void setAsActiveChatroom() final override;
    void updateStatusLight(Status::Status status, bool event) final override;
    void resetEventFlags() final override;
    QString getStatusString() const final override;
    const Group* getGroup() const { return group; };

    void editName();
    ContentDialog* addGroupDialog(Group* group);
    ContentDialog* createContentDialog() const;

    void showDetails();
    void reloadTheme();

signals:
    void groupWidgetClicked(GroupWidget* widget);
    void removeGroup(const GroupId& groupId);
    void destroyGroup(const GroupId& groupId);

protected:
    void contextMenuEvent(QContextMenuEvent* event) final override;
    void mousePressEvent(QMouseEvent* event) final override;
    void mouseMoveEvent(QMouseEvent* event) final override;
    void dragEnterEvent(QDragEnterEvent* ev) override;
    void dragLeaveEvent(QDragLeaveEvent* ev) override;
    void dropEvent(QDropEvent* ev) override;
    void onActiveSet(bool active) override;

private slots:
    void retranslateUi();
    void updateTitle(const QString& author, const QString& newName);
    void updateUserCount(int numPeers);
    void do_widgetClicked(GenericChatroomWidget* w);
    void updateDesc(const QString&);
    void do_removeGroup(const GroupId& groupId);
    void do_destroyGroup(const GroupId& groupId);
    void do_privilegesChanged(
            const Group::Role& role, const Group::Affiliation& aff, const QList<int> codes);

private:
    Group* group;

    ContentLayout* contentLayout;
    AboutGroupForm* about;

    QMenu* menu;
    QAction* destroyGrpAct;
    QAction* quitGroup;
};

#endif  // GROUPWIDGET_H
