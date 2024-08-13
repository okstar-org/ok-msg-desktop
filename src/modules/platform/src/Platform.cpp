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

namespace ok::platform {

Platform::Platform() : m_widget{nullptr} { m_widget = std::make_unique<Widget>(); }

Platform::~Platform() {}

void Platform::init(Profile* p) {}
QString Platform::name() { return {"Platform"}; }

void Platform::start(std::shared_ptr<ok::session::AuthSession> session) { m_widget->start(); }

bool Platform::isStarted() { return false; }
void Platform::onSave(SavedInfo&) {}
void Platform::cleanup() {}
void Platform::destroy() {}
void Platform::hide() {}
}  // namespace ok::platform
