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

#ifndef SETPASSWORDDIALOG_H
#define SETPASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class SetPasswordDialog;
}

class SetPasswordDialog : public QDialog {
    Q_OBJECT

public:
    enum ReturnCode { Rejected = QDialog::Rejected, Accepted = QDialog::Accepted, Tertiary };
    explicit SetPasswordDialog(QString body, QString extraButton, QWidget* parent = nullptr);
    ~SetPasswordDialog();
    QString getPassword();
    static int getPasswordStrength(QString pass);

private slots:
    void onPasswordEdit();

private:
    Ui::SetPasswordDialog* ui;
    QString body;
    static const double reasonablePasswordLength;
};

#endif  // SETPASSWORDDIALOG_H
