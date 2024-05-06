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

#include "ContentWidget.h"
#include "chatformheader.h"
#include "contentlayout.h"
#include "form/chatform.h"
#include "src/persistence/settings.h"
#include "src/widget/form/groupchatform.h"
#include <QLabel>
#include <QStyleFactory>

ContentWidget::ContentWidget(QWidget* parent): QWidget(parent){

  setLayout(new QVBoxLayout(this));

  layout()->setMargin(0);
  layout()->setSpacing(0);
  init();
}

void ContentWidget::init() {

  mainHead = new QWidget();
  mainHead->setLayout(new QVBoxLayout);
  mainHead->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  mainHead->layout()->setMargin(0);
  mainHead->layout()->setSpacing(0);
  mainHead->setMouseTracking(true);

  mainHLine.setFrameShape(QFrame::HLine);
  mainHLine.setFrameShadow(QFrame::Plain);

  QPalette palette = mainHLine.palette();
  palette.setBrush(QPalette::WindowText, QBrush(QColor(193, 193, 193)));
  mainHLine.setPalette(palette);

  mainContent = new QWidget();
  mainContent->setLayout(new QVBoxLayout);
  mainContent->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  if (QStyleFactory::keys().contains(Settings::getInstance().getStyle())
      && Settings::getInstance().getStyle() != "None") {
    mainHead->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
    mainContent->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
  }

//  reloadTheme();

  mainHLineLayout.addSpacing(4);
  mainHLineLayout.addWidget(&mainHLine);
  mainHLineLayout.addSpacing(5);
  layout()->addItem(&mainHLineLayout);

  layout()->addWidget(mainHead);
  layout()->addWidget(mainContent);

  //    addLayout(&mainHLineLayout);
}

void ContentWidget::showTo(ContentLayout *layout) {
  layout->addWidget(this);
  layout->setCurrentWidget(this);
  show();
}

void ContentWidget::setChatForm(ChatForm * form) {
  auto h = form->getHead();
  mainHead->layout()->addWidget(h);
  mainContent->layout()->addWidget(form);
}

void ContentWidget::setGroupChatForm(GroupChatForm * form) {
  auto h = form->getHead();
  mainHead->layout()->addWidget(h);
  mainContent->layout()->addWidget(form);
}
