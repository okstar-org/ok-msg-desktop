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

#ifndef LOADHISTORYDIALOG_H
#define LOADHISTORYDIALOG_H

#include <QDateTime>
#include <QDialog>
#include "src/core/FriendId.h"

namespace Ui {
class LoadHistoryDialog;
}
class IChatLog;

class LoadHistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoadHistoryDialog(const IChatLog* chatLog, QWidget* parent = nullptr);
    explicit LoadHistoryDialog(QWidget* parent = nullptr);
    ~LoadHistoryDialog();

    QDateTime getFromDate();
    void setTitle(const QString& title);
    void setInfoLabel(const QString& info);

public slots:
    void highlightDates(int year, int month);

private:
    Ui::LoadHistoryDialog* ui;
    const IChatLog* chatLog;
};

#endif  // LOADHISTORYDIALOG_H
