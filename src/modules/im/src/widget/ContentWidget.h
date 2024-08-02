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

//
// Created by gaojie on 24-5-5.
//

#ifndef CONTENTWIDGET_H
#define CONTENTWIDGET_H

#include <QFrame>
#include <QHBoxLayout>

#include "src/widget/form/groupchatform.h"
#include "src/worker/SendWorker.h"

class ContentLayout;
class ChatForm;

class ContentWidget : public QWidget {
    Q_OBJECT
public:
    explicit ContentWidget(SendWorker* sendWorker, QWidget* parent = nullptr);
    ~ContentWidget();

    void showTo(ContentLayout*);
    void setChatForm(GenericChatForm*);

    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;

private:
    void init();
    QFrame* mainHLine;
    QHBoxLayout* mainHLineLayout;
    QWidget* mainHead;
    QWidget* seperator;
    QWidget* mainContent;
};

#endif  // OKMSG_PROJECT_CONTENTWIDGET_H
