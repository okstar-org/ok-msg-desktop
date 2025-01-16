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
// Created by gaojie on 23-10-7.
//

#include "Painter.h"
#include "PageClassing.h"

namespace module::painter {

static Painter* Instance;

Painter::Painter() : m_widget(new PageClassing()), m_name(OK_Painter_MODULE) {}
Painter::~Painter() {}

const QString& Painter::getName() const {
    return m_name;
}

void Painter::init(Profile* p) {}

void Painter::start(std::shared_ptr<lib::session::AuthSession> session) {}

bool Painter::isStarted() {
    return true;
}

void Painter::hide() {}
void Painter::onSave(SavedInfo&) {}
void Painter::cleanup() {}
void Painter::stop() {}

QWidget* Painter::widget() {
    return m_widget;
}

}  // namespace module::painter