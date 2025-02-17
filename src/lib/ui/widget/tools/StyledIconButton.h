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

#include <QPushButton>

namespace lib::ui {

class StyledIconButton : public QPushButton {
    Q_OBJECT
public:
    Q_PROPERTY(bool use_indicator READ iconUseIndicator WRITE setIconUseIndicator)

    explicit StyledIconButton(QWidget *parent=nullptr);
    ~StyledIconButton() override=default;

    void setIconUseIndicator(bool set) {
        icon_use_indicator = set;
        update();
    }

    [[nodiscard]] bool iconUseIndicator() const { return icon_use_indicator; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bool icon_use_indicator = false;
};
}

