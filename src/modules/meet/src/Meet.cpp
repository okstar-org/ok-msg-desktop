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

#include "Meet.h"

namespace module::meet {

Meet::Meet() : m_widget{nullptr} { m_widget = std::make_unique<Widget>(); }

Meet::~Meet() {}

void Meet::init(Profile* p) {}

QString Meet::name() { return {"Meet"}; }

void Meet::start(std::shared_ptr<lib::session::AuthSession> session) {
    m_widget->start();
}

bool Meet::isStarted() { return false; }
void Meet::onSave(SavedInfo&) {}
void Meet::cleanup() {}
void Meet::hide() {}
}  // namespace module::meet
