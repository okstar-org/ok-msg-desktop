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
#pragma once

#include "base/resources.h"
#include "modules/module.h"

namespace module::classroom {

class Widget;

class Classroom : public Module {
public:
    Classroom();
    ~Classroom();

    void init(lib::session::Profile* p) override;
    const QString& getName() const override;
    void start(std::shared_ptr<lib::session::AuthSession> session) override;
    [[nodiscard]] bool isStarted() override;
    void stop() override;
    void hide() override;

    void onSave(SavedInfo&) override;
    void cleanup() override;
    QWidget* widget() override;

private:
    QString m_name;
    Widget* m_widget;
};
}  // namespace module::classroom
