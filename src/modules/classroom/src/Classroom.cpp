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

#include "Classroom.h"
#include "PageClassing.h"

namespace module::classroom {

static Classroom* Instance;

Classroom::Classroom() : m_widget(new PageClassing()), m_name(OK_Classroom_MODULE) {
    OK_RESOURCE_INIT(ClassroomRes);
}

Classroom::~Classroom() {}

const QString& Classroom::getName() const {
    return m_name;
}

void Classroom::init(Profile* p) {}

void Classroom::start(std::shared_ptr<lib::session::AuthSession> session) {}

bool Classroom::isStarted() {
    return true;
}

void Classroom::hide() {}
void Classroom::onSave(SavedInfo&) {}
void Classroom::cleanup() {}
void Classroom::stop() {}

QWidget* Classroom::widget() {
    return m_widget;
}

}  // namespace module::classroom
