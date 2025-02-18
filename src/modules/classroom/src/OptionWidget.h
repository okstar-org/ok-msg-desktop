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

#pragma once

#include <QPointer>
#include <QWidget>
#include "lib/ui/widget/OMediaConfigWidget.h"


namespace module::classroom {

/**
 * 选项配置控件
 */
class OptionWidget : public lib::ui::OMediaConfigWidget {
    Q_OBJECT
public:
    explicit OptionWidget(QWidget* parent = nullptr);
    ~OptionWidget() override = default;

    void retranslateUi() override;

signals:
    // 状态改变事件
    void stateChanged();
};
}  // namespace module::classroom
