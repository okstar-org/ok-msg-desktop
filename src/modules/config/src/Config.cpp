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
// Created by gaojie on 25-1-16.
//

#include "Config.h"

namespace module::config {

Config::Config() : m_widget(new ConfigWindow()), m_name(OK_Config_MODULE) {
    OK_RESOURCE_INIT(Config);
}

Config::~Config() {}

void Config::init(Profile* p) {}

const QString& Config::getName() const {
    return m_name;
}

void Config::start(std::shared_ptr<lib::session::AuthSession> session) {}

bool Config::isStarted() {
    return true;
}

void Config::stop() {}

void Config::hide() {}

void Config::onSave(SavedInfo&) {}

void Config::cleanup() {}

QWidget* Config::widget() {
    return m_widget;
}

}  // namespace module::config
