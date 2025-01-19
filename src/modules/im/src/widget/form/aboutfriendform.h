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
namespace module::im {
class Widget;
class Profile;
class Friend;

class AboutFriendForm : public QWidget {
    Q_OBJECT

public:
    AboutFriendForm(const Friend* f, QWidget* parent = nullptr);
    ~AboutFriendForm() override;

    void setName(const QString& name);
    const Friend* getFriend() const {
        return m_friend;
    }
    const ContactId& getId();

private:
    void reloadTheme();
    Ui::AboutFriendForm* ui;
    std::unique_ptr<IAboutFriend> about;
    Widget* widget;
    Profile* profile;
    const Friend* m_friend;

signals:
    void histroyRemoved();

private slots:
    void onAliasChanged(const QString& text);
    void onSendMessageClicked();
    void onRemoveHistoryClicked();
};
}  // namespace module::im
#endif  // ABOUT_USER_FORM_H
