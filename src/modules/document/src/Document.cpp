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
// Created by gaojie on 25-1-14.
//

#include "Document.h"

namespace module::doc {

Document::Document() : name{OK_Document_MODULE}, m_widget(std::make_unique<Widget>()) {}

Document::~Document() {}

void Document::init(lib::session::Profile* p) {}

QWidget* Document::widget() {
    return m_widget.get();
}

const QString& Document::getName() const {
    return name;
}

void Document::start(std::shared_ptr<lib::session::AuthSession> session) {}

void Document::stop() {}

bool Document::isStarted() {
    return false;
}

void Document::onSave(SavedInfo&) {}

void Document::cleanup() {}

void Document::hide() {}

}  // namespace module::doc
