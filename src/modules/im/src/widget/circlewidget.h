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

#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include "categorywidget.h"

class ContentDialog;

class CircleWidget : public CategoryWidget {
    Q_OBJECT
public:
    explicit CircleWidget(FriendListWidget* parent, int id);
    ~CircleWidget();

    void editName();
    static CircleWidget* getFromID(int id);

signals:
    void renameRequested(CircleWidget* circleWidget, const QString& newName);
    void searchCircle(CircleWidget& circletWidget);
    void newContentDialog(ContentDialog& contentDialog);

protected:
    void contextMenuEvent(QContextMenuEvent* event) final override;
    void dragEnterEvent(QDragEnterEvent* event) final override;
    void dragLeaveEvent(QDragLeaveEvent* event) final override;
    void dropEvent(QDropEvent* event) final override;

private:
    void onSetName() final override;
    void onExpand() final override;
    void onAddFriendWidget(FriendWidget* w) final override;
    void updateID(int index);

    static QHash<int, CircleWidget*> circleList;
    int id;
};

#endif  // CIRCLEWIDGET_H
