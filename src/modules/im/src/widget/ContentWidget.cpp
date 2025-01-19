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

namespace module::im {
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
    mainHead->layout()->addWidget(sendWorker->getHeader());
    // 头部信息
    layout()->addWidget(mainHead);

    // 分割条
    seperator = new QWidget(this);
    seperator->setObjectName("ContentSeperator");
    seperator->setFixedHeight(SEPERATOR_WIDTH);
    layout()->addWidget(seperator);

    // 主体内容区
    mainContent = new QWidget(this);
    mainContent->setLayout(new QVBoxLayout);
    mainContent->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mainContent->layout()->setContentsMargins(CONTENT_MARIGN, 0, CONTENT_MARIGN, CONTENT_MARIGN);
    mainContent->layout()->addWidget(sendWorker->getChatForm());
    layout()->addWidget(mainContent);
}

ContentWidget::~ContentWidget() {
    qDebug() << __func__;
}

void ContentWidget::init() {
    //  QPalette palette = mainHLine.palette();
    //  palette.setBrush(QPalette::WindowText, QBrush(QColor(193, 193, 193)));
    //  mainHLine.setPalette(palette);

    //  if (QStyleFactory::keys().contains(Nexus::getProfile()->getSettings()->getStyle())
    //      && Nexus::getProfile()->getSettings()->getStyle() != "None") {
    //    mainHead->setStyle(QStyleFactory::create(Nexus::getProfile()->getSettings()->getStyle()));
    //    mainContent->setStyle(QStyleFactory::create(Nexus::getProfile()->getSettings()->getStyle()));
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

void ContentWidget::showEvent(QShowEvent* event) {}

void ContentWidget::hideEvent(QHideEvent* event) {}
}  // namespace module::im