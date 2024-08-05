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

#ifndef GENERICCHATITEMWIDGET_H
#define GENERICCHATITEMWIDGET_H

#include <src/friendlist.h>
#include <src/grouplist.h>
#include <src/model/contact.h>
#include <QFrame>
#include <QLabel>
#include "src/model/message.h"

class CroppingLabel;
class MaskablePixmapWidget;

/**
 * 聊天控件基类
 */
class GenericChatItemWidget : public QFrame {
    Q_OBJECT
public:
    enum ItemType { GroupItem, FriendOfflineItem, FriendOnlineItem };

    explicit GenericChatItemWidget(ChatType chatType, const ContactId& cid,
                                   QWidget* parent = nullptr);

    bool isCompact() const { return compact; };
    void setCompact(bool compact_) { compact = compact_; };
    Q_PROPERTY(bool compact READ isCompact WRITE setCompact)

    QString getName() const;
    void setName(const QString& name);

    void searchName(const QString& searchString, bool hideAll);

    ChatType getChatType() const { return chatType; };
    void setChatType(ChatType type) { chatType = type; };
    Q_PROPERTY(ChatType chatType READ getChatType WRITE setChatType)

    inline bool isGroup() const { return chatType == ChatType::GroupChat; };

    void setLastMessage(const QString& msg);

    void updateLastMessage(const Message&);

    virtual void updateStatusLight(Status::Status status, bool event);
    virtual void clearStatusLight();

    bool isActive();
    void setActive(bool active);
    virtual void onActiveSet(bool active) = 0;

    virtual void setAvatar(const QPixmap& pic);
    void clearAvatar();
    void setDefaultAvatar();

    void setContact(const Contact& contact);
    void removeContact();

protected:
    virtual void showEvent(QShowEvent* e) override;

    // 名称
    CroppingLabel* nameLabel;
    CroppingLabel* lastMessageLabel;
    // 信号状态
    QLabel* statusPic;
    // 头像
    MaskablePixmapWidget* avatar;

    bool compact;
    ChatType chatType;
    ContactId contactId;

    /**
     * 联系人
     * 1、好友添加时被设置为friend、group的指针。
     * 2、删除好友时，设置为nullptr。
     */
    const Contact* contact;

    Status::Status prevStatus;
    bool active;
};

#endif  // GENERICCHATITEMWIDGET_H
