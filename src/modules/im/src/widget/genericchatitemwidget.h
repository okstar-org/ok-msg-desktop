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

#include <QFrame>
#include <QLabel>
#include "src/model/message.h"

class CroppingLabel;

/**
 * 聊天控件
 */
class GenericChatItemWidget : public QFrame
{
    Q_OBJECT
public:
    enum ItemType
    {
        GroupItem,
        FriendOfflineItem,
        FriendOnlineItem
    };

    explicit GenericChatItemWidget(ChatType chatType=ChatType::Chat, QWidget* parent = nullptr);

    bool isCompact() const{return compact;};
    void setCompact(bool compact_){compact = compact_;};
    Q_PROPERTY(bool compact READ isCompact WRITE setCompact)

    QString getName() const;

    void searchName(const QString& searchString, bool hideAll);


    ChatType getChatType() const{return chatType;};
    void setChatType(ChatType type){chatType = type;};
    Q_PROPERTY(ChatType chatType READ getChatType WRITE setChatType)

    bool isGroup() const {return chatType == ChatType::GroupChat;};
protected:
    CroppingLabel* nameLabel;
    QLabel statusPic;
    bool compact;
    ChatType chatType;
};

#endif // GENERICCHATITEMWIDGET_H
