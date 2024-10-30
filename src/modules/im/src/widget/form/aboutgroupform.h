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

#ifndef ABOUTGROUPFORM_H
#define ABOUTGROUPFORM_H

#include <QWidget>
#include "src/model/group.h"

namespace Ui {
class AboutGroupForm;
}

class AboutGroupForm : public QWidget {
    Q_OBJECT

public:
    explicit AboutGroupForm(const Group* gId, QWidget* parent = nullptr);
    ~AboutGroupForm();
    void init();
    const ContactId& getId();

    void updateUI();

private:
    Ui::AboutGroupForm* ui;
    const Group* group;

private slots:
    void onSendMessageClicked();
    void doNameChanged(const QString& text);
    void doAliasChanged(const QString& text);
    void doSubjectChanged(const QString& text);
    void doDescChanged(const QString& text);
};

#endif  // ABOUTGROUPFORM_H
