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

#ifndef CONTENTDIALOG_H
#define CONTENTDIALOG_H

#include "src/core/FriendId.h"
#include "src/core/groupid.h"
#include "src/model/chatroom/chatroom.h"
#include "src/model/dialogs/idialogs.h"
#include "src/model/status.h"
#include "src/widget/genericchatitemlayout.h"
#include "src/widget/tool/activatedialog.h"

#include <memory>

template <typename K, typename V> class QHash;

class ContentLayout;
class Friend;
class FriendChatroom;
class FriendListLayout;
class FriendWidget;
class GenericChatForm;
class GenericChatroomWidget;
class Group;
class GroupChatroom;
class GroupWidget;
class QCloseEvent;
class QSplitter;

class ContentDialog : public ActivateDialog, public IDialogs {
    Q_OBJECT
public:
    explicit ContentDialog(QWidget* parent = nullptr);
    ~ContentDialog() override;

    void addFriend(FriendChatroom* chatroom, GenericChatForm* form);
    void addGroup(GroupChatroom* chatroom, GenericChatForm* form);
    void removeFriend(const FriendId& friendPk) override;
    void removeGroup(const GroupId& groupId) override;
    int chatroomCount() const override;
    void ensureSplitterVisible();
    void updateTitleAndStatusIcon();

    void cycleContacts(bool forward, bool loop = true);
    void onVideoShow(QSize size);
    void onVideoHide();

    void addFriendWidget(FriendWidget* widget, Status::Status status);
    bool isActiveWidget(GenericChatroomWidget* widget);

    bool hasContact(const ContactId& contactId) const override;
    bool isContactActive(const ContactId& contactId) const override;

    void focusContact(const ContactId& friendPk);
    void updateFriendStatus(const FriendId& friendPk, Status::Status status);
    void updateContactStatusLight(const ContactId& contactId);

    void setStatusMessage(const FriendId& friendPk, const QString& message);

signals:
    void friendDialogShown(const Friend* f);
    void groupDialogShown(const Group* g);
    void addFriendDialog(Friend* frnd, ContentDialog* contentDialog);
    void addGroupDialog(Group* group, ContentDialog* contentDialog);
    void activated();
    void willClose();
    void connectFriendWidget(FriendWidget& friendWidget);

public slots:
    void reorderLayouts(bool newGroupOnTop);
    void previousContact();
    void nextContact();
    void setUsername(const QString& newName);

protected:
    bool event(QEvent* event) final override;
    void dragEnterEvent(QDragEnterEvent* event) final override;
    void dropEvent(QDropEvent* event) final override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public slots:
    void activate(GenericChatroomWidget* widget);

private slots:
    void updateFriendWidget(const FriendId& friendPk, QString alias);
    void onGroupchatPositionChanged(bool top);

private:
    void closeIfEmpty();
    void closeEvent(QCloseEvent* event) override;

    void retranslateUi();
    void saveDialogGeometry();
    void saveSplitterState();
    QLayout* nextLayout(QLayout* layout, bool forward) const;
    int getCurrentLayout(QLayout*& layout);
    void focusCommon(const ContactId& id, QHash<const ContactId&, GenericChatroomWidget*> list);

private:
    QList<QLayout*> layouts;
    QSplitter* splitter;
    FriendListLayout* friendLayout;
    GenericChatItemLayout groupLayout;
    ContentLayout* contentLayout;
    GenericChatroomWidget* activeChatroomWidget;
    QSize videoSurfaceSize;
    int videoCount;

    GenericChatroomWidget* contactWidget;

    GenericChatForm* m_chatForm;
    Chatroom* m_chatroom;

    QString username;
};

#endif  // CONTENTDIALOG_H
