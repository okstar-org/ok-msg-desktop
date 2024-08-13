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
#include <QLabel>
#include <QStyleFactory>
#include "chatformheader.h"
#include "contentlayout.h"
#include "form/chatform.h"
#include "src/persistence/settings.h"
#include "src/widget/form/groupchatform.h"
#include "src/worker/SendWorker.h"

static constexpr int HEADER_MARIGN = 8;
static constexpr int CONTENT_MARIGN = 8;
static constexpr int SEPERATOR_WIDTH = 2;

ContentWidget::ContentWidget(SendWorker* sendWorker, QWidget* parent) : QWidget(parent) {
    setLayout(new QVBoxLayout(this));

    layout()->setMargin(0);
    layout()->setSpacing(0);

    mainHead = new QWidget(this);
    mainHead->setLayout(new QVBoxLayout);
    mainHead->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    mainHead->layout()->setContentsMargins(HEADER_MARIGN, HEADER_MARIGN, HEADER_MARIGN,
                                           HEADER_MARIGN);
    //  mainHead->layout()->setSpacing(0);
    //  mainHead->setMouseTracking(true);

    //  mainHLine = new QFrame(this);
    //  mainHLine->setFrameShape(QFrame::HLine);
    //  mainHLine->setFrameShadow(QFrame::Plain);

    //  mainHLineLayout=new QHBoxLayout(this);
    //  mainHLineLayout->addWidget(mainHLine);
    //  mainHLineLayout->addSpacing(4);
    //  mainHLineLayout->addSpacing(5);
    //  layout()->addItem(mainHLineLayout);

    mainContent = new QWidget(this);
    mainContent->setLayout(new QVBoxLayout);
    mainContent->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mainContent->layout()->setContentsMargins(CONTENT_MARIGN, 0, CONTENT_MARIGN, CONTENT_MARIGN);

    seperator = new QWidget(this);
    seperator->setObjectName("ContentSeperator");
    seperator->setFixedHeight(SEPERATOR_WIDTH);

    layout()->addWidget(mainHead);
    layout()->addWidget(seperator);
    layout()->addWidget(mainContent);

    mainHead->layout()->addWidget(sendWorker->getHeader());
    mainContent->layout()->addWidget(sendWorker->getChatForm());
}

ContentWidget::~ContentWidget() { qDebug() << __func__; }

void ContentWidget::init() {
    //  QPalette palette = mainHLine.palette();
    //  palette.setBrush(QPalette::WindowText, QBrush(QColor(193, 193, 193)));
    //  mainHLine.setPalette(palette);

    //  if (QStyleFactory::keys().contains(Settings::getInstance().getStyle())
    //      && Settings::getInstance().getStyle() != "None") {
    //    mainHead->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
    //    mainContent->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
    //  }

    //  reloadTheme();
}

void ContentWidget::showTo(ContentLayout* layout) {
    //  auto contentIndex = layout->indexOf(this);
    //    if(contentIndex < 0 ){
    //     contentIndex = layout->addWidget(this);
    //    }
    layout->setCurrentWidget(this);
    this->show();
}

void ContentWidget::setChatForm(GenericChatForm* form) {
    //  auto h = form->getHead();
    //  mainHead->layout()->addWidget(form->getHead());
    //  mainContent->layout()->addWidget(form);
}

void ContentWidget::showEvent(QShowEvent* event) {}

void ContentWidget::hideEvent(QHideEvent* event) {}
