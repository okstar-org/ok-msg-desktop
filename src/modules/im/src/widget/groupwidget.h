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

#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include "genericchatroomwidget.h"

#include "src/model/chatroom/groupchatroom.h"
#include "src/core/groupid.h"

#include <memory>

class GroupWidget final : public GenericChatroomWidget
{
    Q_OBJECT
public:
    GroupWidget(std::shared_ptr<GroupChatroom> chatroom, bool compact);
    ~GroupWidget();
    void setAsInactiveChatroom() final override;
    void setAsActiveChatroom() final override;
    void updateStatusLight() final override;
    void resetEventFlags() final override;
    QString getStatusString() const final override;
    Group* getGroup() const final override;
    const Contact* getContact() const final override;
    void setName(const QString& name);
    void editName();

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

private slots:
    void retranslateUi();
    void updateTitle(const QString& author, const QString& newName);
    void updateUserCount(int numPeers);

public:
    GroupId groupId;

private:
    std::shared_ptr<GroupChatroom> chatroom;
};

#endif // GROUPWIDGET_H
