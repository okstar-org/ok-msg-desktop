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

#include "src/widget/form/groupchatform.h"
#include <QFrame>
#include <QHBoxLayout>

class ContentLayout;
class ChatForm;

class ContentWidget : public QWidget {
  Q_OBJECT
public:
  explicit ContentWidget(QWidget *parent= nullptr);
  void showTo(ContentLayout*);
  void setChatForm(GenericChatForm *);

private:
  void init();
  QFrame mainHLine;
  QHBoxLayout mainHLineLayout;
  QWidget* mainHead;
  QWidget* mainContent;

  int contentIndex;
};

#endif // OKMSG_PROJECT_CONTENTWIDGET_H
