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

#pragma once

#include <QFrame>
#include <QPointer>

class QToolButton;
class QHBoxLayout;
class QMenu;

namespace lib::ui {

class PopupMenuComboBox : public QFrame {
    Q_OBJECT

signals:
    void menuRequest();

public:
    explicit PopupMenuComboBox(QWidget* parent = nullptr);
    void setLabel(const QString& text);
    void setWidget(QWidget* widget);
    QToolButton* iconButton();

    // 设置菜单
    void setMenu(QMenu * menu);
    // 手动显示自定义菜单
    void showMenuOnce(QMenu * menu);

private:
    void onMenuButtonClicked();

private:
    QHBoxLayout* mainLayout = nullptr;
    QToolButton* _iconButton = nullptr;
    QToolButton* menuButton = nullptr;
    QPointer<QWidget> content;
    QPointer<QMenu> popMenu;
};
}  // namespace lib::ui
