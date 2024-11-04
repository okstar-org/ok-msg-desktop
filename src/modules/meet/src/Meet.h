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

#pragma once

#include "Widget.h"
#include "modules/module.h"

namespace module::meet {

class Meet : public QObject, public Module {
    Q_OBJECT
public:
    Meet();
    virtual ~Meet();
    void init(Profile* p) override;
    QString name() override;
    void start(std::shared_ptr<ok::session::AuthSession> session) override;
    bool isStarted() override;
    void onSave(SavedInfo&) override;
    void cleanup() override;

    QWidget* widget() override { return m_widget.get(); }
    void hide() override;

private:
    std::unique_ptr<Widget> m_widget;
};

}  // namespace module::meet
