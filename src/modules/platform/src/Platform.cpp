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
// Created by gaojie on 24-7-31.
//

#include "Platform.h"
#include "AppCenterWidget.h"
#include "platformpagecontainer.h"

namespace ok::platform {

Platform::Platform() : name{OK_Platform_MODULE}, m_widget{nullptr} {
    m_widget = std::make_unique<Widget>();

    // todo: 如果后期需要支持页签弹出，看如何重构了
    pageContainter = new PlatformPageContainer(this, m_widget.get());

    AppCenterPage* page = new AppCenterPage(pageContainter);
    page->createContent(m_widget.get());
    pageContainter->addPage(page);
}

Platform::~Platform() {
    // 目前PlatformPage指针绑定到了Widget内部的tab页上
    // 当Widget释放时，会自动释放PlatformPage
    // todo: 是否考虑要调整PlatformPage的所有权
    if (pageContainter) {
        delete pageContainter;
        pageContainter = nullptr;
    }
}

void Platform::init(Profile* p) {}

const QString& Platform::getName() const {
    return name;
}

void Platform::start(std::shared_ptr<lib::session::AuthSession> session) {
    m_widget->start();
}

void Platform::stop() {
    // TODO 一些停止操作
};

bool Platform::isStarted() {
    return false;
}
void Platform::onSave(SavedInfo&) {}
void Platform::cleanup() {}
PlatformPageContainer* Platform::getPageContainer() {
    return pageContainter;
}
void Platform::hide() {}
}  // namespace ok::platform
