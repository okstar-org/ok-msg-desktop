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

#include <QFrame>
#include <QPointer>

class QPushButton;
class RoundedPixmapLabel;
class PopupMenuComboBox;
class QSlider;
class QHBoxLayout;

namespace module::meet {

/**
 * 选项配置控件
 */
class MeetingOptionWidget : public QWidget {
    Q_OBJECT

public:
    MeetingOptionWidget(QWidget* parent = nullptr);
    void addFooterButton(QPushButton* button);

    void retranslateUi();

protected:
    void showEvent(QShowEvent* event) override;

private:
    RoundedPixmapLabel* avatarLabel = nullptr;

    PopupMenuComboBox* micSpeakSetting = nullptr;
    PopupMenuComboBox* cameraSetting = nullptr;
    PopupMenuComboBox* volumnSetting = nullptr;
    QSlider* volumnSlider = nullptr;

    QHBoxLayout* buttonLayout = nullptr;
};
}  // namespace module::meet
