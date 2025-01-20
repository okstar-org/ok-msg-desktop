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
// Created by gaojie on 25-1-20.
//

#pragma once

#include "lib/ui/widget/OWidget.h"

namespace lib::ui {

class WebWidget : public OWidget {
    Q_OBJECT
public:
    explicit WebWidget(const QUrl& url, QWidget* parent = nullptr);
    ~WebWidget() override;
private slots:
    static void onLoadFinished(bool ok);
};

}  // namespace lib::ui
