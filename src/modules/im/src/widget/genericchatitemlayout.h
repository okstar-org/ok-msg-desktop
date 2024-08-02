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

#ifndef GENERICCHATITEMLAYOUT_H
#define GENERICCHATITEMLAYOUT_H

#include <Qt>

class QLayout;
class QVBoxLayout;
class GenericChatItemWidget;

class GenericChatItemLayout {
public:
    GenericChatItemLayout();
    GenericChatItemLayout(const GenericChatItemLayout& layout) = delete;
    ~GenericChatItemLayout();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    void addSortedWidget(GenericChatItemWidget* widget, int stretch = 0,
                         Qt::Alignment alignment = Qt::Alignment());
#else
    void addSortedWidget(GenericChatItemWidget* widget, int stretch = 0,
                         Qt::Alignment alignment = nullptr);
#endif
    int indexOfSortedWidget(GenericChatItemWidget* widget) const;
    bool existsSortedWidget(GenericChatItemWidget* widget) const;
    void removeSortedWidget(GenericChatItemWidget* widget);
    void search(const QString& searchString, bool hideAll = false);

    QLayout* getLayout() const;

private:
    int indexOfClosestSortedWidget(GenericChatItemWidget* widget) const;
    QVBoxLayout* layout;
};

#endif  // GENERICCHATITEMLAYOUT_H
