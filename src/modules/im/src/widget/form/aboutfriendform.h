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

#ifndef ABOUT_USER_FORM_H
#define ABOUT_USER_FORM_H

#include "src/model/aboutfriend.h"

#include <QDialog>
#include <QPointer>

#include <memory>

namespace Ui {
class AboutFriendForm;
}

class Widget;
class Profile;

class AboutFriendForm : public QWidget {
    Q_OBJECT

public:
    AboutFriendForm(std::unique_ptr<IAboutFriend> about, QWidget* parent = nullptr);
    ~AboutFriendForm();

    void setName(const QString& name);

private:
    void reloadTheme();
    Ui::AboutFriendForm* ui;
    const std::unique_ptr<IAboutFriend> about;
    Widget* widget;
    Profile* profile;
signals:
    void histroyRemoved();

private slots:
    void onAliasChanged(const QString& text);
    void onAutoAcceptDirChanged(const QString& path);
    void onSendMessageClicked();
    void onAutoAcceptDirClicked();
    void onAutoAcceptCallClicked();
    void onSelectDirClicked();
    void onRemoveHistoryClicked();
};

#endif  // ABOUT_USER_FORM_H
