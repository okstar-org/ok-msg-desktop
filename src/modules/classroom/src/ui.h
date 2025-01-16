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

#include <string>

namespace module::classroom {

enum class PageMenu {
    welcome = 0,
#ifdef OK_MODULE_PAINTER
    classing,
#endif
    chat,
    record,
    calendar,
    email,
    personal,
    setting
};

enum class VIDEO_MODE { PLAYER, CAMERA };

enum class VIDEO_SIZE {
    SMALL,
    MIDDLE,
    BIG,
};

enum class VIDEO_FOR {
    STUDENT,
    TEACHER,
};

typedef struct VideoWidgetConfT {
    VIDEO_MODE mode;
    VIDEO_SIZE size;
    VIDEO_FOR _for;

} VideoWidgetConfig;

constexpr VideoWidgetConfig TeacherConf = {.size = VIDEO_SIZE::MIDDLE, ._for = VIDEO_FOR::TEACHER};

constexpr VideoWidgetConfig StudentConf = {.size = VIDEO_SIZE::SMALL, ._for = VIDEO_FOR::STUDENT};

constexpr VideoWidgetConfig SelfConf = {.size = VIDEO_SIZE::MIDDLE, ._for = VIDEO_FOR::STUDENT};

}  // namespace module::classroom
