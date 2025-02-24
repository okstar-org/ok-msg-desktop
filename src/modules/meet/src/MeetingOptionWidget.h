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
#include <QMenu>
#include <QPointer>

#include <lib/ui/widget/OMediaConfigWidget.h>


#include "Defines.h"
#include "base/compatiblerecursivemutex.h"
#include "lib/video/videomode.h"
#include "lib/ortc/ok_rtc.h"
#include "lib/audio/iaudiosource.h"
#include "lib/audio/iaudiosink.h"

class QPushButton;
class QSlider;
class QHBoxLayout;
class QStackedLayout;


namespace lib::ui {
class PopupMenuComboBox;
class RoundedPixmapLabel;
}

namespace lib::audio{
class IAudioControl;
}

namespace lib::video {
class CameraSource;
class VideoFrame;
}  // namespace lib::video

namespace module::meet {

/**
 * 选项配置控件
 */
class MeetingOptionWidget : public lib::ui::OMediaConfigWidget {
    Q_OBJECT
public:
    explicit MeetingOptionWidget(QWidget* parent = nullptr);
    void retranslateUi() override;
// signals:
//     // 状态改变事件
//     void stateChanged();
};

}  // namespace module::meet
