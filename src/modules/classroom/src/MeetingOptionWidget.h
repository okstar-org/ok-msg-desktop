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
#include "Defines.h"
#include "lib/ortc/ok_rtc.h"

class QPushButton;
class RoundedPixmapLabel;
class PopupMenuComboBox;
class QSlider;
class QHBoxLayout;

namespace module::classroom {

/**
 * 选项配置控件
 */
class MeetingOptionWidget : public QWidget {
    Q_OBJECT
public:
    explicit MeetingOptionWidget(QWidget* parent = nullptr);
    ~MeetingOptionWidget();

    void addFooterButton(QPushButton* button);

    void retranslateUi();

    inline const lib::ortc::CtrlState& getCtrlState() const {
        return ctrlState;
    }

protected:
    void showEvent(QShowEvent* event) override;

private:
    void updateAudioVideoIcon(bool audio, bool video, bool spk);

    RoundedPixmapLabel* avatarLabel = nullptr;

    PopupMenuComboBox* micSpeakSetting = nullptr;
    PopupMenuComboBox* cameraSetting = nullptr;
    PopupMenuComboBox* volumeSetting = nullptr;
    QSlider* volumeSlider = nullptr;

    QHBoxLayout* buttonLayout = nullptr;

    lib::ortc::CtrlState ctrlState;
signals:
    // 状态改变事件
    void stateChanged();
};
}  // namespace module::classroom
