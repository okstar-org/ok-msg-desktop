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

#ifndef FRIENDFORM_H
#define FRIENDFORM_H

#include <QFrame>

#include <src/core/contactid.h>
#include "lib/backend/UserService.h"

namespace Ui {
class FriendForm;
}

namespace ok::backend {
struct OrgStaff;
}

class FriendForm : public QFrame {
    Q_OBJECT

public:
    explicit FriendForm(const ok::backend::OrgStaff& staff, QWidget* parent = nullptr);
    ~FriendForm();

private:
    Ui::FriendForm* ui;

    ok::backend::OrgStaff staff;

signals:
    void add(const QString& cId, QString& nick);
};

#endif  // FRIENDFORM_H
