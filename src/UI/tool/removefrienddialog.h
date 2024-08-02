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

#ifndef DELETEFRIENDDIALOG_H
#define DELETEFRIENDDIALOG_H

#include <QDialog>
#include "lib/lib/messenger/model/friend.h"
#include "ui_removefrienddialog.h"

class RemoveFriendDialog : public QDialog {
    Q_OBJECT
public:
    explicit RemoveFriendDialog(QWidget* parent, const lib::IM::Friend* f);

    inline bool removeHistory() { return ui.removeHistory->isChecked(); }

    inline bool accepted() { return _accepted; }

public slots:
    void onAccepted();

protected:
    Ui_RemoveFriendDialog ui;
    bool _accepted = false;
};

#endif  // DELETEFRIENDDIALOG_H
