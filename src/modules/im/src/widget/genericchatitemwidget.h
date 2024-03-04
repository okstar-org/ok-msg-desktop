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

class CroppingLabel;

/**
 * 聊天框
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

    explicit GenericChatItemWidget(bool compact, QWidget* parent = nullptr);

    bool isCompact() const;
    void setCompact(bool compact);

    QString getName() const;

    void searchName(const QString& searchString, bool hideAll);

    Q_PROPERTY(bool compact READ isCompact WRITE setCompact)

protected:
    CroppingLabel* nameLabel;
    QLabel statusPic;

private:
    bool compact;
};

#endif // GENERICCHATITEMWIDGET_H
